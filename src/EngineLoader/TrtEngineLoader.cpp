#if defined(WITH_CUDA)

#include "EngineLoader/TrtEngineLoader.hpp"

TrtEngineLoader::TrtEngineLoader(std::string enginePath, float minConfidence = 0.5, float maxIou = 0.4) :
		minConfidence_(minConfidence), maxIou_(maxIou)
{
	inputSize_ = 1;
	outputSize_ = 1;

	loadEngine(enginePath);
	initBuffers();
}

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
	auto inputDims = cudaEngine_->getTensorShape("images");
	for (int i = 0; i < inputDims.nbDims; ++i)
	{
		inputSize_ *= inputDims.d[i];
	}
	batchSize_ = inputDims.d[0];
	inputWidth_ = inputDims.d[2];
	inputHeight_ = inputDims.d[3];

	auto outputDims = cudaEngine_->getTensorShape("output0");
	for (int i = 0; i < outputDims.nbDims; i++)
	{
		//以本项目所用模型为例，输出1*11*34000
		//其中1为batch_size，11为：centerX, centerY, width, height, clsConf0, clsConf1, ...，25200为先验框数量
		std::cout << "[Info] Output dim" << i << ": size = " << outputDims.d[i] << std::endl;
		outputSize_ *= outputDims.d[i];
	}
	classNum_ = outputDims.d[1] - 4;
	outputMaxNum_ = outputDims.d[2];
}

void TrtEngineLoader::initBuffers()
{
	setInOutputSize();

	cudaMalloc(&gpuBuffers_[0], inputSize_ * sizeof(float));//分配显存（接收主机内存输入）
	cudaMalloc(&gpuBuffers_[1], outputSize_ * sizeof(float));//分配显存（向主机内存输出）

	executionContext_->setInputShape("images", nvinfer1::Dims4{batchSize_, 3, inputWidth_, inputHeight_});
	executionContext_->setTensorAddress("images", gpuBuffers_[0]);
	executionContext_->setTensorAddress("output0", gpuBuffers_[1]);

	cudaStreamCreate(&meiCudaStream_);
	inputBlob_ = new float[inputSize_];
	cpuOutputBuffer_ = new float[outputSize_];//主机内存接收来自GPU的推理结果
}

void TrtEngineLoader::imgProcess(Mat inputImg)
{
	//缩放与填充
	imgRatio_ = std::min((inputWidth_ * 1.) / inputImg.cols, (inputHeight_ * 1.) / inputImg.rows);
	int borderWidth = inputImg.cols * imgRatio_;
	int borderHeight = inputImg.rows * imgRatio_;
	offsetX_ = (inputWidth_ - borderWidth) / 2;
	offsetY_ = (inputHeight_ - borderHeight) / 2;
	resize(inputImg, inputImg, Size(borderWidth, borderHeight));
	//填充纯色像素，使原图变为网络输入大小
	copyMakeBorder(inputImg, inputImg, offsetY_, offsetY_, offsetX_, offsetX_, BORDER_CONSTANT, WHITE);
	cvtColor(inputImg, inputImg, COLOR_BGR2RGB);

	//转CHW与归一化
	int channels = inputImg.channels();
	int height = inputImg.rows;
	int width = inputImg.cols;
	for (int c = 0; c < channels; ++c)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = 0; w < width; ++w)
			{
				inputBlob_[c * width * height + h * width + w] = inputImg.at<Vec3b>(h, w)[c] / 255.0f;
			}
		}
	}
}

void TrtEngineLoader::infer()
{
	cudaMemcpyAsync(gpuBuffers_[0], inputBlob_, inputSize_ * sizeof(float), cudaMemcpyHostToDevice, meiCudaStream_);//输入数据传入显存
	auto start = std::chrono::system_clock::now();
	executionContext_->enqueueV3(meiCudaStream_);//异步推理
	cudaStreamSynchronize(meiCudaStream_);
	auto end = std::chrono::system_clock::now();
	cudaMemcpyAsync(cpuOutputBuffer_, gpuBuffers_[1], outputSize_ * sizeof(float), cudaMemcpyDeviceToHost, meiCudaStream_);//推理数据传出显存

//	std::cout << "[Info] Inference time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
}

void TrtEngineLoader::detectDataProcess(std::vector<Ball> &pickedBalls, int cameraId)
{
	detectedBalls_.clear();

	for (int anchor = 0; anchor < outputMaxNum_; ++anchor)
	{
		// boxData = [centerX, centerY, width, height, clsConf0, clsConf1, ...]
		float boxData[classNum_ + 4];
		for (int j = 0; j < classNum_ + 4; ++j)
		{
			boxData[j] = cpuOutputBuffer_[anchor + j * outputMaxNum_];
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

		Ball ball = Ball((boxData[0] - offsetX_) / imgRatio_,
		                 (boxData[1] - offsetY_) / imgRatio_,
		                 labelNum,
		                 *maxClassConf,
		                 cameraId,
		                 isInBasket);
		ball.width = boxData[2] / imgRatio_;
		ball.height = boxData[3] / imgRatio_;
		ball.x = ball.centerX_ - ball.width / 2;
		ball.y = ball.centerY_ - ball.height / 2;
		detectedBalls_.push_back(ball);
	}

	//NMS
	int pickedBallStart = pickedBalls.size();
	sort(detectedBalls_.begin(), detectedBalls_.end(), [](Ball &ball1, Ball &ball2) -> bool {
		return ball1.confidence_ > ball2.confidence_;
	});
	for (Ball &newBall: detectedBalls_)
	{
		bool pick = true;
		for (int index = pickedBallStart; index < pickedBalls.size(); ++index)
		{
			if (Functions::calcIou(pickedBalls.at(index), newBall) > maxIou_)
			{
				pick = false;
				break;
			}
		}
		if (pick)
		{
			pickedBalls.push_back(newBall);
		}
	}
	std::cout << "[Info] Picked " << pickedBalls.size() << " objects" << std::endl;
}

void TrtEngineLoader::detect(Mat inputImg, std::vector<Ball> &pickedBalls, int cameraId)
{
	imgProcess(inputImg);
	infer();
	detectDataProcess(pickedBalls, cameraId);
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