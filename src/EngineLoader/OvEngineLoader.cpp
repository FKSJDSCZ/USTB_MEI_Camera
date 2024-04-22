#if defined(WITH_OPENVINO)

#include "EngineLoader/OvEngineLoader.hpp"

OvEngineLoader::OvEngineLoader(std::string modelPath, std::string device = "CPU", float minConfidence = 0.5, float maxIou = 0.4) :
		minConfidence_(minConfidence), maxIou_(maxIou)
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
		//以YOLOv5为例，输出1*25200*12
		//其中1为batch_size，25200为先验框数量，12为：4（先验框参数，即centerX,centerY,width,height）+1（objectness，即boxConf）+7（classNum，即clsConf）
		std::cout << "[Info] Output shape[" << i << "]: size = " << outputShape.at(i) << std::endl;
	}
	classNum_ = outputShape.at(1) - 4;
	outputMaxNum_ = outputShape.at(2);
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

void OvEngineLoader::detectDataProcess(std::vector<Ball> &pickedBalls, int cameraId)
{
	detectedBalls_.clear();
	auto *ptr = inferRequest_.get_output_tensor().data<float>();

	for (int anchor = 0; anchor < outputMaxNum_; ++anchor)
	{
		// boxData = [centerX, centerY, width, height, clsConf0, clsConf1, ...]
		float boxData[classNum_ + 4];
		for (int j = 0; j < classNum_ + 4; ++j)
		{
			boxData[j] = ptr[anchor + j * outputMaxNum_];
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

void OvEngineLoader::detect(Mat inputImg, std::vector<Ball> &pickedBalls, int cameraId)
{
	imgProcess(inputImg);
	infer();
	detectDataProcess(pickedBalls, cameraId);
}

#endif