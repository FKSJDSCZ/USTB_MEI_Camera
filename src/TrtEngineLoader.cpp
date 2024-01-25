#if defined(WITH_CUDA)

#include "TrtEngineLoader.hpp"

TrtEngineLoader::TrtEngineLoader(std::string enginePath, float minObjectness = 0.4, float minConfidence = 0.5, float maxIou = 0.4) :
		minObjectness_(minObjectness), minConfidence_(minConfidence), maxIou_(maxIou)
{
	inputWidth_ = 640;
	inputHeight_ = 640;
	outputSize_ = 1;

	loadEngine(enginePath);
	initBuffers();
}

// 读取模型，反序列化成engine
void TrtEngineLoader::loadEngine(std::string &enginePath_)
{
	char *modelStream{nullptr};

	std::ifstream inputFileStream(enginePath_.c_str(), std::ios::binary);
	size_t engineSize{0};
	if (inputFileStream.good())
	{
		inputFileStream.seekg(0, std::ifstream::end);
		engineSize = inputFileStream.tellg();
		inputFileStream.seekg(0, std::ifstream::beg);
		modelStream = new char[engineSize];
		assert(modelStream);
		inputFileStream.read(modelStream, engineSize);
		inputFileStream.close();
	}
	meiRuntime_ = nvinfer1::createInferRuntime(meiLogger);
	assert(meiRuntime_ != nullptr);
	meiCudaEngine_ = meiRuntime_->deserializeCudaEngine(modelStream, engineSize);//反序列化
	assert(meiCudaEngine_ != nullptr);
	meiExecutionContext_ = meiCudaEngine_->createExecutionContext();
	assert(meiExecutionContext_ != nullptr);

	delete[] modelStream;
}

//获取网络输出层结构
void TrtEngineLoader::setOutputSize()
{
	auto out_dims = meiCudaEngine_->getBindingDimensions(1);
	for (int j = 0; j < out_dims.nbDims; j++)
	{
		//以YOLOv5为例，输出1*25200*85
		//其中1为batch_size，25200为先验框数量，85为：4（先验框参数，即centerX,centerY,width,height）+1（objectness，即boxConf）+80（classNum，即clsConf）
		std::cout << "[Info] Output dim" << j << ": size = " << out_dims.d[j] << std::endl;
		outputSize_ *= out_dims.d[j];
	}
	batchSize_ = out_dims.d[0];
	outputMaxNum_ = out_dims.d[1];
	classNum_ = out_dims.d[2] - 5;
}

//分配相关内存
void TrtEngineLoader::initBuffers()
{
	setOutputSize();
	inputSize_ = batchSize_ * inputHeight_ * inputWidth_ * 3;
	inputBlob_ = new float[inputSize_];
	//输入输出层的名字根据网络结构确定
	const int inputIndex = meiCudaEngine_->getBindingIndex("images");
	const int outputIndex = meiCudaEngine_->getBindingIndex("output0");
	assert(inputIndex == 0);
	assert(outputIndex == 1);
	cudaMalloc((void **) &gpuBuffers_[inputIndex], inputSize_ * sizeof(float));//分配显存（接收主机内存输入）
	cudaMalloc((void **) &gpuBuffers_[outputIndex], outputSize_ * sizeof(float));//分配显存（向主机内存输出）
	cudaStreamCreate(&meiCudaStream_);
	cpuOutputBuffer_ = new float[outputSize_]();//主机内存接收来自GPU的推理结果
}

//图像预处理
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

// 推理
void TrtEngineLoader::infer()
{
	cudaMemcpyAsync(gpuBuffers_[0], inputBlob_, inputSize_ * sizeof(float), cudaMemcpyHostToDevice, meiCudaStream_);//输入数据传入显存
	auto start = std::chrono::system_clock::now();//计时器
	meiExecutionContext_->enqueueV2((void **) gpuBuffers_, meiCudaStream_, nullptr);//异步推理
	cudaMemcpyAsync((void *) cpuOutputBuffer_, gpuBuffers_[1], outputSize_ * sizeof(float), cudaMemcpyDeviceToHost, meiCudaStream_);//推理数据传出显存
	cudaStreamSynchronize(meiCudaStream_);//流同步
	auto end = std::chrono::system_clock::now();

	std::cout << "[Info] Inference time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
}

//后处理推理数据
void TrtEngineLoader::detectDataProcess(std::vector<Ball> &detectedBalls_, std::vector<int> &pickedBallsIndex_, int cameraId)
{
	int detectedBallCount_ = detectedBalls_.size();
	float *ptr = cpuOutputBuffer_;
	for (int i = 0; i < outputMaxNum_; ++i)
	{
		float objectness = ptr[4];//网格单元中存在物体的概率（置信度）
		if (objectness >= minObjectness_)
		{
			int label = std::max_element(ptr + 5, ptr + 5 + classNum_) - (ptr + 5);
			bool isInBasket = false;
			float centerX = (ptr[0] - offsetX_) / imgRatio_;//减去填充像素
			float centerY = (ptr[1] - offsetY_) / imgRatio_;
			float confidence = ptr[label + 5] * objectness;//该物体属于某个标签类别的概率（置信度）

			//判断是否在球框内（7cls）
			if (classNum_ == 7)
			{
				if (label % 2)
				{
					label--;
					isInBasket = true;
				}
				label /= 2;
			}

			if (confidence >= minConfidence_)
			{
				Ball ball = Ball(centerX, centerY, label, confidence, cameraId, isInBasket);
				ball.width = ptr[2] / imgRatio_;
				ball.height = ptr[3] / imgRatio_;
				ball.x = ball.centerX_ - ball.width * 0.5;
				ball.y = ball.centerY_ - ball.height * 0.5;
				detectedBalls_.push_back(ball);
			}
		}
		ptr += 5 + classNum_;
	}
//	std::cout << "[Info] Found " << detectedBalls_.size() << " objects" << std::endl;

	//NMS 防止出现大框套小框
	for (; detectedBallCount_ < detectedBalls_.size(); ++detectedBallCount_)
	{
		bool pick = true;
		for (int index: pickedBallsIndex_)
		{
			if (Functions::calcIou(detectedBalls_.at(detectedBallCount_), detectedBalls_.at(index)) > maxIou_)//两框重叠程度太高就抛弃一个
			{
				pick = false;
			}
		}
		if (pick)
		{
			pickedBallsIndex_.push_back(detectedBallCount_);
		}
	}
	std::cout << "[Info] Picked " << pickedBallsIndex_.size() << " objects" << std::endl;
}

TrtEngineLoader::~TrtEngineLoader()
{
	cudaStreamDestroy(meiCudaStream_);
	delete[] inputBlob_;
	delete[] cpuOutputBuffer_;
	cudaFree(gpuBuffers_[0]);
	cudaFree(gpuBuffers_[1]);
	delete meiExecutionContext_;
	delete meiCudaEngine_;
	delete meiRuntime_;
}

#endif