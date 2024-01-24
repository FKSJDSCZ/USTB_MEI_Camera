#if defined(WITH_OPENVINO)

#include "OvEngineLoader.hpp"

OvEngineLoader::OvEngineLoader(std::string modelPath, std::string device = "CPU", float minObjectness = 0.4, float minConfidence = 0.5,
                               float maxIou = 0.4) : minObjectness_(minObjectness), minConfidence_(minConfidence), maxIou_(maxIou)
{
	inputWidth_ = 640;
	inputHeight_ = 640;

	loadEngine(modelPath, device);
}

void OvEngineLoader::loadEngine(std::string &modelPath_, std::string &device_)
{
	ov::Core core;
	ov::CompiledModel compiledModel = core.compile_model(modelPath_, device_);
	inferRequest_ = compiledModel.create_infer_request();
	setOutputSize();
}

void OvEngineLoader::setOutputSize()
{
	ov::Shape outputShape = inferRequest_.get_output_tensor().get_shape();
	for (int i = 0; i < outputShape.size(); i++)
	{
		//以YOLOv5为例，输出1*25200*85
		//其中1为batch_size，25200为先验框数量，85为：4（先验框参数，即centerX,centerY,width,height）+1（objectness，即boxConf）+80（classNum，即clsConf）
		std::cout << "[Info] Output shape[" << i << "]: size = " << outputShape.at(i) << std::endl;
	}
	outputMaxNum_ = outputShape.at(1);
	classNum_ = outputShape.at(2) - 5;
}

void OvEngineLoader::imgProcess(Mat inputImg)
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
	ov::Tensor inputTensor = inferRequest_.get_input_tensor();
	float *inputBlob = inputTensor.data<float>();
	for (int c = 0; c < channels; ++c)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = 0; w < width; ++w)
			{
				inputBlob[c * width * height + h * width + w] = inputImg.at<Vec3b>(h, w)[c] / 255.0f;
			}
		}
	}
}

void OvEngineLoader::infer()
{
	auto start = std::chrono::system_clock::now();//计时器
	inferRequest_.start_async();
	inferRequest_.wait();
	auto end = std::chrono::system_clock::now();

//	std::cout << "[Info] Inference time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
}

void OvEngineLoader::detectDataProcess(std::vector<Ball> &detectedBalls_, std::vector<int> &pickedBallsIndex_, int cameraId)
{
	int detectedBallCount_ = detectedBalls_.size();
	float *ptr = inferRequest_.get_output_tensor().data<float>();

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

#endif