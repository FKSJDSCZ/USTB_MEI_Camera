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
	//以本项目所用模型为例，输入batchSize*3*640*640（NCHW）
	//其中3为通道数，两个640依次为矩阵的高和宽
	auto inputDims = cudaEngine_->getTensorShape("images");
	if (inputDims.d[0] != -1)
	{
		batchSize_ = inputDims.d[0];
	}
	inputLayerHeight_ = inputDims.d[2];
	inputLayerWidth_ = inputDims.d[3];
	inputSize_ = inputDims.d[1] * inputLayerHeight_ * inputLayerWidth_;
	inputTensorSize_ = batchSize_ * inputSize_;

	//以本项目所用模型（YOLO8-p2）为例，输出batchSize*11*34000（NHW）
	//其中11为：centerX, centerY, width, height, clsConf0, clsConf1, ...，25200为先验框数量
	//加上TensorRT EfficientNMS Plugin后，输出分为四个部分
	auto detectedBoxesDims = cudaEngine_->getTensorShape("det_boxes");
	maxOutputNumber_ = detectedBoxesDims.d[1];
}

TrtEngineLoader::TrtEngineLoader(std::string enginePath, int batchSize) : batchSize_(batchSize)
{
	initLibNvInferPlugins(&trtLogger, "");
	loadEngine(enginePath);
	setInOutputSize();
}

void TrtEngineLoader::setLetterboxParameters(std::vector<std::shared_ptr<ICameraLoader>> &cameras)
{
	for (const std::shared_ptr<ICameraLoader> &camera: cameras)
	{
		float imageScale = std::min((inputLayerWidth_ * 1.) / camera->imageWidth(), (inputLayerHeight_ * 1.) / camera->imageHeight());
		int resizedWidth = camera->imageWidth() * imageScale;
		int resizedHeight = camera->imageHeight() * imageScale;
		int offsetX = (inputLayerWidth_ - resizedWidth) / 2;
		int offsetY = (inputLayerHeight_ - resizedHeight) / 2;
		letterboxParameters_.push_back(
				{
						camera->imageWidth(),
						camera->imageHeight(),
						resizedWidth,
						resizedHeight,
						offsetX,
						offsetY,
						imageScale
				}
		);
	}
}

void TrtEngineLoader::init()
{
	cudaStreamCreate(&meiCudaStream_);

	//allocate output buffers
	cudaMalloc(&numDetBuffer_, batchSize_ * 1 * sizeof(int));
	cudaMalloc(&detBoxesBuffer_, batchSize_ * maxOutputNumber_ * 4 * sizeof(float));
	cudaMalloc(&detScoresBuffer_, batchSize_ * maxOutputNumber_ * sizeof(float));
	cudaMalloc(&detClassesBuffer_, batchSize_ * maxOutputNumber_ * sizeof(int));
	cpuNumDetBuffer_ = new int[batchSize_ * 1];
	cpuDetBoxesBuffer_ = new float[batchSize_ * maxOutputNumber_ * 4];
	cpuDetScoresBuffer_ = new float[batchSize_ * maxOutputNumber_];
	cpuDetClassesBuffer_ = new int[batchSize_ * maxOutputNumber_];

	//prepare for preprocess
	for (int i = 0; i < batchSize_; ++i)
	{
		imageTensors_.push_back(
				nvcv::Tensor(1, {letterboxParameters_.at(i).imageWidth_, letterboxParameters_.at(i).imageHeight_}, nvcv::FMT_BGR8)
		);
		resizedImageTensors_.push_back(
				nvcv::Tensor(1, {letterboxParameters_.at(i).resizedWidth_, letterboxParameters_.at(i).resizedHeight_}, nvcv::FMT_BGR8)
		);
		rgbImageTensors_.push_back(
				nvcv::Tensor(1, {letterboxParameters_.at(i).resizedWidth_, letterboxParameters_.at(i).resizedHeight_}, nvcv::FMT_RGB8)
		);
		borderImageTensors_.push_back(
				nvcv::Tensor(1, {inputLayerWidth_, inputLayerHeight_}, nvcv::FMT_RGB8)
		);
		normalizedImageTensors_.push_back(
				nvcv::Tensor(1, {inputLayerWidth_, inputLayerHeight_}, nvcv::FMT_RGBf32)
		);
		reformattedImageTensors_.push_back(
				nvcv::Tensor(1, {inputLayerWidth_, inputLayerHeight_}, nvcv::FMT_RGBf32p)
		);
	}
	inputTensorBatch_ = nvcv::TensorBatch(batchSize_);
	inputTensor_ = nvcv::Tensor(batchSize_, {inputLayerWidth_, inputLayerHeight_}, nvcv::FMT_RGBf32p);

	//set IO tensor address
	executionContext_->setInputShape("images", nvinfer1::Dims4{batchSize_, 3, inputLayerWidth_, inputLayerHeight_});
	executionContext_->setTensorAddress("images", inputTensor_.exportData<nvcv::TensorDataStridedCuda>()->basePtr());
	executionContext_->setTensorAddress("num_dets", numDetBuffer_);
	executionContext_->setTensorAddress("det_boxes", detBoxesBuffer_);
	executionContext_->setTensorAddress("det_scores", detScoresBuffer_);
	executionContext_->setTensorAddress("det_classes", detClassesBuffer_);

	detectedBalls_.insert(detectedBalls_.begin(), batchSize_, {});
}

void TrtEngineLoader::setInput(cv::Mat &BGRImage, int imageId)
{
	auto tensorData = imageTensors_.at(imageId).exportData<nvcv::TensorDataStridedCuda>();
	cudaMemcpyAsync(tensorData->basePtr(), BGRImage.data, tensorData->stride(0), cudaMemcpyHostToDevice, meiCudaStream_);
}

void TrtEngineLoader::setInput(std::vector<CameraImage> &cameraImages)
{
	for (CameraImage &cameraImage: cameraImages)
	{
		setInput(cameraImage.colorImage_, cameraImage.cameraId_);
	}
}

void TrtEngineLoader::preProcess()
{
	inputTensorBatch_.clear();
	for (int i = 0; i < batchSize_; ++i)
	{
		//resize
		resize_(meiCudaStream_, imageTensors_.at(i), resizedImageTensors_.at(i), NVCV_INTERP_LINEAR);
		//cvtColor(BGR -> RGB)
		cvtColor_(meiCudaStream_, resizedImageTensors_.at(i), rgbImageTensors_.at(i), NVCV_COLOR_BGR2RGB);
		//copyMakeBorder
		copyMakeBorder_(meiCudaStream_, rgbImageTensors_.at(i), borderImageTensors_.at(i),
		                letterboxParameters_.at(i).offsetY_, letterboxParameters_.at(i).offsetX_, NVCV_BORDER_CONSTANT, {114, 114, 114, 0});
		//normalize
		convertTo_(meiCudaStream_, borderImageTensors_.at(i), normalizedImageTensors_.at(i), 1.0 / 255.0, 0);
		//reformat(HWC -> CHW)
		reformat_(meiCudaStream_, normalizedImageTensors_.at(i), reformattedImageTensors_.at(i));

		inputTensorBatch_.pushBack(reformattedImageTensors_.at(i));
	}
	//stack
	stack_(meiCudaStream_, inputTensorBatch_, inputTensor_);
	cudaStreamSynchronize(meiCudaStream_);
}

void TrtEngineLoader::infer()
{
	executionContext_->enqueueV3(meiCudaStream_);
	cudaStreamSynchronize(meiCudaStream_);
}

void TrtEngineLoader::postProcess()
{
	for (int i = 0; i < batchSize_; ++i)
	{
		detectedBalls_.at(i).clear();
	}

	cudaMemcpy(cpuNumDetBuffer_, numDetBuffer_, batchSize_ * 1 * sizeof(int), cudaMemcpyDeviceToHost);
	cudaMemcpy(cpuDetBoxesBuffer_, detBoxesBuffer_, batchSize_ * maxOutputNumber_ * 4 * sizeof(float), cudaMemcpyDeviceToHost);
	cudaMemcpy(cpuDetScoresBuffer_, detScoresBuffer_, batchSize_ * maxOutputNumber_ * sizeof(float), cudaMemcpyDeviceToHost);
	cudaMemcpy(cpuDetClassesBuffer_, detClassesBuffer_, batchSize_ * maxOutputNumber_ * sizeof(int), cudaMemcpyDeviceToHost);

	for (int batchSize = 0; batchSize < batchSize_; ++batchSize)
	{
		for (int index = 0; index < cpuNumDetBuffer_[batchSize]; ++index)
		{
			float topLeftX = cpuDetBoxesBuffer_[(batchSize * maxOutputNumber_ + index) * 4 + 0];
			float topLeftY = cpuDetBoxesBuffer_[(batchSize * maxOutputNumber_ + index) * 4 + 1];
			float bottomRightX = cpuDetBoxesBuffer_[(batchSize * maxOutputNumber_ + index) * 4 + 2];
			float bottomRightY = cpuDetBoxesBuffer_[(batchSize * maxOutputNumber_ + index) * 4 + 3];
			Ball ball;
			ball.addGraphPosition(
					((topLeftX + bottomRightX) / 2 - letterboxParameters_.at(batchSize).offsetX_) / (letterboxParameters_.at(batchSize).imageScale_),
					((topLeftY + bottomRightY) / 2 - letterboxParameters_.at(batchSize).offsetY_) / (letterboxParameters_.at(batchSize).imageScale_),
					(bottomRightX - topLeftX) / letterboxParameters_.at(batchSize).imageScale_,
					(bottomRightY - topLeftY) / letterboxParameters_.at(batchSize).imageScale_,
					cpuDetScoresBuffer_[batchSize * maxOutputNumber_ + index],
					cpuDetClassesBuffer_[batchSize * maxOutputNumber_ + index] / 2,
					batchSize,
					cpuDetClassesBuffer_[batchSize * maxOutputNumber_ + index] % 2
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
	cudaFree(numDetBuffer_);
	cudaFree(detBoxesBuffer_);
	cudaFree(detScoresBuffer_);
	cudaFree(detClassesBuffer_);
	delete[] cpuNumDetBuffer_;
	delete[] cpuDetBoxesBuffer_;
	delete[] cpuDetScoresBuffer_;
	delete[] cpuDetClassesBuffer_;
}

#endif