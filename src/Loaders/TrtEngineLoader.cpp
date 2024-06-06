#if defined(WITH_CUDA)

#include "Loaders/TrtEngineLoader.hpp"

void TrtEngineLoader::loadEngine(std::string &enginePath)
{
	std::vector<unsigned char> modelData;

	std::ifstream inputFileStream(enginePath.c_str(), std::ios::binary);
	std::streamsize engineSize;
	if (inputFileStream.good())
	{
		inputFileStream.seekg(0, std::ifstream::end);
		engineSize = inputFileStream.tellg();
		modelData.resize(engineSize);
		inputFileStream.seekg(0, std::ifstream::beg);
		inputFileStream.read(reinterpret_cast<char *>(modelData.data()), engineSize);
		inputFileStream.close();
	}
	runtime_ = std::unique_ptr<nvinfer1::IRuntime>(nvinfer1::createInferRuntime(trtLogger));
	cudaEngine_ = std::unique_ptr<nvinfer1::ICudaEngine>(runtime_->deserializeCudaEngine(modelData.data(), engineSize));
	executionContext_ = std::unique_ptr<nvinfer1::IExecutionContext>(cudaEngine_->createExecutionContext());

	LOGGER(Logger::INFO, std::format("Load engine {} successfully", enginePath), true);
}

void TrtEngineLoader::setInOutputSize()
{
	//以本项目所用模型为例，输入(-)1*3*640*640（NCHW）
	//其中1为batch，3为通道数，两个640依次为矩阵的高和宽
	auto inputDims = cudaEngine_->getTensorShape("images");
	if (inputDims.d[0] != -1)
	{
		batchSize_ = inputDims.d[0];
	}
	inputLayerHeight_ = inputDims.d[2];
	inputLayerWidth_ = inputDims.d[3];
	inputSize_ = inputDims.d[1] * inputLayerHeight_ * inputLayerWidth_;
	inputTensorSize_ = batchSize_ * inputSize_;

	//以本项目所用模型为例，输出(-)1*11*34000（NHW）
	//其中1为batch，11为：centerX, centerY, width, height, clsConf0, clsConf1, ...，25200为先验框数量
	auto outputDims = cudaEngine_->getTensorShape("output0");
	classNum_ = outputDims.d[1] - 4;
	outputMaxNum_ = outputDims.d[2];
	outputSize_ = outputDims.d[1] * outputMaxNum_;
	outputTensorSize_ = batchSize_ * outputSize_;
}

void TrtEngineLoader::initBuffers()
{
	setInOutputSize();

	//letterbox
	imgRatio_ = std::min((inputLayerWidth_ * 1.) / inputImageWidth_, (inputLayerHeight_ * 1.) / inputImageHeight_);
	int borderWidth = inputImageWidth_ * imgRatio_;
	int borderHeight = inputImageHeight_ * imgRatio_;
	offsetX_ = (inputLayerWidth_ - borderWidth) / 2;
	offsetY_ = (inputLayerHeight_ - borderHeight) / 2;

	cudaStreamCreate(&meiCudaStream_);
	cudaMalloc(&gpuOutputBuffer_, outputTensorSize_ * sizeof(float));

	//imageBatch
	imageBatch_ = nvcv::TensorBatch(batchSize_);
	for (int i = 0; i < batchSize_; ++i)
	{
		imageBatch_.pushBack(nvcv::Tensor(1, {inputImageWidth_, inputImageHeight_}, nvcv::FMT_BGR8));
	}
	//imageTensor
	imageTensor_ = nvcv::Tensor(batchSize_, {inputImageWidth_, inputImageHeight_}, nvcv::FMT_BGR8);
	//inputTensor
	inputTensor_ = nvcv::Tensor(batchSize_, {inputLayerWidth_, inputLayerHeight_}, nvcv::FMT_RGBf32p);

	//tensors in preprocess
	resizedImageTensor_ = nvcv::Tensor(batchSize_, {borderWidth, borderHeight}, nvcv::FMT_BGR8);
	rgbImageTensor_ = nvcv::Tensor(batchSize_, {borderWidth, borderHeight}, nvcv::FMT_RGB8);
	borderImageTensor_ = nvcv::Tensor(batchSize_, {inputLayerWidth_, inputLayerHeight_}, nvcv::FMT_RGB8);
	normalizedImageTensor_ = nvcv::Tensor(batchSize_, {inputLayerWidth_, inputLayerHeight_}, nvcv::FMT_RGBf32);

	executionContext_->setInputShape("images", nvinfer1::Dims4{batchSize_, 3, inputLayerWidth_, inputLayerHeight_});
	executionContext_->setTensorAddress("images", inputTensor_.exportData<nvcv::TensorDataStridedCuda>()->basePtr());
	executionContext_->setTensorAddress("output0", gpuOutputBuffer_);

	detectedBalls_.insert(detectedBalls_.begin(), batchSize_, {});
}

TrtEngineLoader::TrtEngineLoader(std::string enginePath, int batchSize, float minConfidence, float maxIou) :
		batchSize_(batchSize), minConfidence_(minConfidence), maxIou_(maxIou)
{
	loadEngine(enginePath);
	initBuffers();
}

void TrtEngineLoader::setInput(cv::Mat &BGRImage, int imageId)
{
	setInput(BGRImage.data, imageId);
}

void TrtEngineLoader::setInput(uint8_t *rawInput, int imageId)
{
	if (imageId >= batchSize_)
	{
		throw std::runtime_error(std::format("ImageId {} exceeded batch size limit {}", imageId, batchSize_));
	}

	auto it = imageBatch_.begin();
	while (--imageId >= 0)
	{
		it++;
	}

	auto singleImageBuffer = it->exportData<nvcv::TensorDataStridedCuda>();
	cudaMemcpyAsync(singleImageBuffer->basePtr(), rawInput, singleImageBuffer->stride(0), cudaMemcpyHostToDevice, meiCudaStream_);
}

void TrtEngineLoader::preProcess()
{
	stack_(meiCudaStream_, imageBatch_, imageTensor_);
	//resize
	resize_(meiCudaStream_, imageTensor_, resizedImageTensor_, NVCV_INTERP_LINEAR);
	//cvtColor(BGR -> RGB)
	cvtColor_(meiCudaStream_, resizedImageTensor_, rgbImageTensor_, NVCV_COLOR_BGR2RGB);
	//copyMakeBorder
	copyMakeBorder_(meiCudaStream_, rgbImageTensor_, borderImageTensor_,
	                offsetY_, offsetX_, NVCV_BORDER_CONSTANT, {114, 114, 114, 0});
	//normalize
	convertTo_(meiCudaStream_, borderImageTensor_, normalizedImageTensor_, 1.0 / 255.0, 0);
	//reformat(NHWC -> NCHW)
	reformat_(meiCudaStream_, normalizedImageTensor_, inputTensor_);
}

void TrtEngineLoader::infer()
{
	executionContext_->enqueueV3(meiCudaStream_);
	cudaStreamSynchronize(meiCudaStream_);//wait for asynchronous inference
}

void TrtEngineLoader::postProcess()
{
	for (int i = 0; i < batchSize_; ++i)
	{
		detectedBalls_.at(i).clear();
	}

	torch::Tensor rawData;
	torch::Tensor ballData;
	torch::Tensor classConfData;
	torch::Tensor maxConfData;
	torch::Tensor mask;
	torch::Tensor classIndexData;
	torch::Tensor filteredIndex;
	torch::Tensor basketTag;

	rawData = torch::from_blob(gpuOutputBuffer_, {(classNum_ + 4) * batchSize_, outputMaxNum_}, torch::dtype(torch::kFloat).device(torch::kCUDA));
	for (int batchSize = 0; batchSize < batchSize_; ++batchSize)
	{
		//NMS
		ballData = rawData.slice(0, (classNum_ + 4) * batchSize, (classNum_ + 4) * batchSize + 4);
		ballData = ballData.transpose(0, 1);
		classConfData = rawData.slice(0, (classNum_ + 4) * batchSize + 4, (classNum_ + 4) * (batchSize + 1));

		std::tuple<torch::Tensor, torch::Tensor> maxInfo = classConfData.max(0);
		maxConfData = std::get<0>(maxInfo);
		classIndexData = std::get<1>(maxInfo);

		mask = maxConfData >= minConfidence_;
		ballData = ballData.index({mask});
		maxConfData = maxConfData.index({mask});
		classIndexData = classIndexData.index({mask});

		ballData.slice(1, 2, 3) += ballData.slice(1, 0, 1);
		ballData.slice(1, 3, 4) += ballData.slice(1, 1, 2);

		filteredIndex = vision::ops::nms(ballData, maxConfData, 0.4);

		ballData = ballData.index({filteredIndex});
		maxConfData = maxConfData.index({filteredIndex});
		classIndexData = classIndexData.index({filteredIndex});

		ballData.slice(1, 2, 3) = (ballData.slice(1, 2, 3) - ballData.slice(1, 0, 1)) / imgRatio_;
		ballData.slice(1, 3, 4) = (ballData.slice(1, 3, 4) - ballData.slice(1, 1, 2)) / imgRatio_;
		ballData.slice(1, 0, 1) = (ballData.slice(1, 0, 1) - offsetX_) / imgRatio_;
		ballData.slice(1, 1, 2) = (ballData.slice(1, 1, 2) - offsetY_) / imgRatio_;
		basketTag = classIndexData % 2 == 1;
		classIndexData = (classIndexData / 2).to(torch::kLong);

		ballData = ballData.to(torch::kCPU);
		maxConfData = maxConfData.to(torch::kCPU);
		classIndexData = classIndexData.to(torch::kCPU);
		basketTag = basketTag.to(torch::kCPU);

		auto ballDataAccess = ballData.accessor<float, 2>();
		auto maxConfDataAccess = maxConfData.accessor<float, 1>();
		auto classIndexDataAccess = classIndexData.accessor<long, 1>();
		auto basketTagAccess = basketTag.accessor<bool, 1>();

		for (int ballCount = 0; ballCount < ballData.size(0); ++ballCount)
		{
			Ball ball;
			ball.addGraphPosition(
					ballDataAccess[ballCount][0], ballDataAccess[ballCount][1], ballDataAccess[ballCount][2], ballDataAccess[ballCount][3],
					maxConfDataAccess[ballCount], classIndexDataAccess[ballCount], batchSize, basketTagAccess[ballCount]
			);
			detectedBalls_.at(batchSize).push_back(ball);
		}
	}
}

void TrtEngineLoader::getBallsByCameraId(int cameraId, std::vector<Ball> &container)
{
	if (cameraId >= batchSize_)
	{
		throw std::runtime_error(std::format("cameraId {} exceeded batch size limit {}", cameraId, batchSize_));
	}

	for (const Ball &tempBall: detectedBalls_.at(cameraId))
	{
		container.push_back(tempBall);
	}
}

TrtEngineLoader::~TrtEngineLoader()
{
	cudaStreamDestroy(meiCudaStream_);
	cudaFree(gpuOutputBuffer_);
}

#endif