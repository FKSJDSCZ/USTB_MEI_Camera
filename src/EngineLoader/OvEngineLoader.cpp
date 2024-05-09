#if defined(WITH_OPENVINO)

#include "EngineLoader/OvEngineLoader.hpp"

void OvEngineLoader::loadEngine(std::string &modelPath, std::string &binPath, std::string &device)
{
	ov::Core core;
	auto model = core.read_model(modelPath, binPath);
	model->reshape({{batchSize_, 3, inputHeight_, inputWidth_}});
	ov::CompiledModel compiledModel = core.compile_model(model, device);
	inferRequest_ = compiledModel.create_infer_request();

	Logger::getInstance().writeMsg(Logger::INFO, std::format("Load engine {} successfully", modelPath));
}

void OvEngineLoader::setInOutputSize()
{
	ov::Shape outputShape = inferRequest_.get_output_tensor().get_shape();
	for (int i = 0; i < outputShape.size(); i++)
	{
		std::cout << "[Info] Output shape[" << i << "]: size = " << outputShape.at(i) << std::endl;
	}
	classNum_ = outputShape.at(1) - 4;
	outputMaxNum_ = outputShape.at(2);
	outputSize_ = outputShape.at(1) * outputMaxNum_;
}

OvEngineLoader::OvEngineLoader(std::string modelPath, std::string binPath, std::string device, int batchSize, float minConfidence, float maxIou) :
		batchSize_(batchSize), minConfidence_(minConfidence), maxIou_(maxIou)
{
	inputWidth_ = 640;
	inputHeight_ = 640;
	inputSize_ = 3 * inputHeight_ * inputWidth_;

	loadEngine(modelPath, binPath, device);
	setInOutputSize();

	detectedBalls_.insert(detectedBalls_.begin(), batchSize_, {});
	pickedBallsIndex_.insert(pickedBallsIndex_.begin(), batchSize_, {});
}

void OvEngineLoader::imgProcess(Mat inputImg, int imageId)
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
	ov::Tensor inputTensor = inferRequest_.get_input_tensor();
	auto *inputBlob = inputTensor.data<float>();
	int channels = inputImg.channels();
	for (int c = 0; c < channels; ++c)
	{
		for (int h = 0; h < inputHeight_; ++h)
		{
			for (int w = 0; w < inputWidth_; ++w)
			{
				inputBlob[imageId * inputSize_ + c * inputWidth_ * inputHeight_ + h * inputWidth_ + w] = inputImg.at<Vec3b>(h, w)[c] / 255.0f;
			}
		}
	}
}

void OvEngineLoader::infer()
{
	auto start = std::chrono::system_clock::now();
	inferRequest_.start_async();
	inferRequest_.wait();
	auto end = std::chrono::system_clock::now();

	std::cout << "[Info] Inference time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
}

void OvEngineLoader::detectDataProcess()
{
	for (int i = 0; i < batchSize_; ++i)
	{
		detectedBalls_.at(i).clear();
		pickedBallsIndex_.at(i).clear();
	}

	auto *ptr = inferRequest_.get_output_tensor().data<float>();
	for (int batchSize = 0; batchSize < batchSize_; ++batchSize)
	{
		for (int anchor = 0; anchor < outputMaxNum_; ++anchor)
		{
			// boxData = [centerX, centerY, width, height, clsConf0, clsConf1, ...]
			float boxData[classNum_ + 4];
			for (int row = 0; row < classNum_ + 4; ++row)
			{
				boxData[row] = ptr[batchSize * outputSize_ + row * outputMaxNum_ + anchor];
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

void OvEngineLoader::getBallsByCameraId(int cameraId, std::vector<Ball> &container)
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

#endif