#if defined(WITH_CUDA)

#include "EngineLoader/TrtEngineLoader.hpp"

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

	Logger::getInstance().writeMsg(Logger::INFO, std::format("Load engine {} successfully", enginePath));
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
	inputHeight_ = inputDims.d[2];
	inputWidth_ = inputDims.d[3];
	inputSize_ = inputDims.d[1] * inputHeight_ * inputWidth_;
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

	cudaMalloc(&gpuBuffers_[0], inputTensorSize_ * sizeof(float));//分配显存（接收主机内存输入）
	cudaMalloc(&gpuBuffers_[1], outputTensorSize_ * sizeof(float));//分配显存（向主机内存输出）

	executionContext_->setInputShape("images", nvinfer1::Dims4{batchSize_, 3, inputWidth_, inputHeight_});
	executionContext_->setTensorAddress("images", gpuBuffers_[0]);
	executionContext_->setTensorAddress("output0", gpuBuffers_[1]);

	cudaStreamCreate(&meiCudaStream_);
	inputBlob_ = new float[inputTensorSize_];
	cpuOutputBuffer_ = new float[outputTensorSize_];//主机内存接收来自GPU的推理结果

	detectedBalls_.insert(detectedBalls_.begin(), batchSize_, {});
	pickedBallsIndex_.insert(pickedBallsIndex_.begin(), batchSize_, {});
}

TrtEngineLoader::TrtEngineLoader(std::string enginePath, int batchSize, float minConfidence, float maxIou) :
		batchSize_(batchSize), minConfidence_(minConfidence), maxIou_(maxIou)
{
	loadEngine(enginePath);
	initBuffers();
}

void TrtEngineLoader::imgProcess(Mat inputImg, int imageId)
{
	if (imageId >= batchSize_)
	{
		throw std::runtime_error(std::format("ImageId {} exceeded batch size limit {}", imageId, batchSize_));
	}

	//letterbox
	imgRatio_ = std::min((inputWidth_ * 1.) / inputImg.cols, (inputHeight_ * 1.) / inputImg.rows);
	int borderWidth = inputImg.cols * imgRatio_;
	int borderHeight = inputImg.rows * imgRatio_;
	offsetX_ = (inputWidth_ - borderWidth) / 2;
	offsetY_ = (inputHeight_ - borderHeight) / 2;
	resize(inputImg, inputImg, Size(borderWidth, borderHeight));
	copyMakeBorder(inputImg, inputImg, offsetY_, offsetY_, offsetX_, offsetX_, BORDER_CONSTANT, GRAY);
	cvtColor(inputImg, inputImg, COLOR_BGR2RGB);

	//HWC转CHW与归一化
	int channels = inputImg.channels();
	for (int c = 0; c < channels; ++c)
	{
		for (int h = 0; h < inputHeight_; ++h)
		{
			for (int w = 0; w < inputWidth_; ++w)
			{
				inputBlob_[imageId * inputSize_ + c * inputWidth_ * inputHeight_ + h * inputWidth_ + w] = inputImg.at<Vec3b>(h, w)[c] / 255.0f;
			}
		}
	}
}

void TrtEngineLoader::infer()
{
	cudaMemcpyAsync(gpuBuffers_[0], inputBlob_, inputTensorSize_ * sizeof(float), cudaMemcpyHostToDevice, meiCudaStream_);//输入数据传入显存
	auto start = std::chrono::system_clock::now();
	executionContext_->enqueueV3(meiCudaStream_);//异步推理
	cudaStreamSynchronize(meiCudaStream_);
	auto end = std::chrono::system_clock::now();
	cudaMemcpyAsync(cpuOutputBuffer_, gpuBuffers_[1], outputTensorSize_ * sizeof(float), cudaMemcpyDeviceToHost, meiCudaStream_);//推理数据传出显存

//	std::cout << "[Info] Inference time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us" << std::endl;
}

void TrtEngineLoader::detectDataProcess()
{
	for (int i = 0; i < batchSize_; ++i)
	{
		detectedBalls_.at(i).clear();
		pickedBallsIndex_.at(i).clear();
	}

	for (int batchSize = 0; batchSize < batchSize_; ++batchSize)
	{
		for (int anchor = 0; anchor < outputMaxNum_; ++anchor)
		{
			// boxData = [centerX, centerY, width, height, clsConf0, clsConf1, ...]
			float boxData[classNum_ + 4];
			for (int row = 0; row < classNum_ + 4; ++row)
			{
				boxData[row] = cpuOutputBuffer_[batchSize * outputSize_ + row * outputMaxNum_ + anchor];
			}

			float *maxClassConf = std::max_element(boxData + 4, boxData + classNum_ + 4);
			if (*maxClassConf < minConfidence_)
			{
				continue;
			}

			int labelNum = maxClassConf - (boxData + 4);
			bool isInBasket = false;
			if (labelNum % 2)
			{
				labelNum--;
				isInBasket = true;
			}
			labelNum /= 2;

			Ball ball;
			ball.addGraphPosition((boxData[0] - offsetX_) / imgRatio_,
			                      (boxData[1] - offsetY_) / imgRatio_,
			                      boxData[2] / imgRatio_,
			                      boxData[3] / imgRatio_,
			                      *maxClassConf,
			                      labelNum,
			                      batchSize,
			                      isInBasket);
			detectedBalls_.at(batchSize).push_back(ball);
		}

		//NMS
		sort(detectedBalls_.at(batchSize).begin(), detectedBalls_.at(batchSize).end(), [](Ball &ball1, Ball &ball2) -> bool {
			return ball1.confidence_ > ball2.confidence_;
		});

		for (int newIndex = 0; newIndex < detectedBalls_.at(batchSize).size(); ++newIndex)
		{
			bool pick = true;
			for (int pickedIndex: pickedBallsIndex_.at(batchSize))
			{
				if (Functions::calcIou(detectedBalls_.at(batchSize).at(pickedIndex).graphRect(),
				                       detectedBalls_.at(batchSize).at(newIndex).graphRect()) > maxIou_)
				{
					pick = false;
					break;
				}
			}
			if (pick)
			{
				pickedBallsIndex_.at(batchSize).push_back(newIndex);
			}
		}
	}
//	std::cout << "[Info] Picked " << pickedBallsIndex.size() << " objects" << std::endl;
}

void TrtEngineLoader::getBallsByCameraId(int cameraId, std::vector<Ball> &container)
{
	if (cameraId >= batchSize_)
	{
		throw std::runtime_error(std::format("cameraId {} exceeded batch size limit {}", cameraId, batchSize_));
	}

	for (const int index: pickedBallsIndex_.at(cameraId))
	{
		Ball &tempBall = detectedBalls_.at(cameraId).at(index);
		if (tempBall.cameraId() == cameraId)
		{
			container.push_back(tempBall);
		}
	}
}

TrtEngineLoader::~TrtEngineLoader()
{
	cudaStreamDestroy(meiCudaStream_);
	delete[] inputBlob_;
	delete[] cpuOutputBuffer_;
	cudaFree(gpuBuffers_[0]);
	cudaFree(gpuBuffers_[1]);
}

#endif