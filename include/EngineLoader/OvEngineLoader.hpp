#pragma once

#if defined(WITH_OPENVINO)

#include "openvino/openvino.hpp"
#include "opencv2/opencv.hpp"

#include "IEngineLoader.hpp"
#include "Entity/Ball.hpp"
#include "Util/Functions.hpp"
#include "Constants.hpp"

class OvEngineLoader :
		public IEngineLoader
{
private:
	std::vector<Ball> detectedBalls_;
	ov::InferRequest inferRequest_;
	int inputHeight_;
	int inputWidth_;
	int offsetX_;
	int offsetY_;
	int classNum_;
	float imgRatio_;
	int outputMaxNum_;
	float minConfidence_;
	float maxIou_;

	void loadEngine(std::string &modelPath_, std::string &device_);

	void setOutputSize();

	void imgProcess(Mat inputImg) override;

	void infer() override;

	void detectDataProcess(std::vector<Ball> &pickedBalls, int cameraId) override;

public:
	OvEngineLoader(std::string modelPath, std::string device, float minConfidence, float maxIou);

	void detect(Mat inputImg, std::vector<Ball> &pickedBalls, int cameraId) override;
};

#endif