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
	ov::InferRequest inferRequest_;
	int inputHeight_;
	int inputWidth_;
	int offsetX_;
	int offsetY_;
	int classNum_;
	float imgRatio_;
	int outputMaxNum_;
	float minObjectness_;
	float minConfidence_;
	float maxIou_;

	void loadEngine(std::string &modelPath_, std::string &device_);

	void setOutputSize();

	void imgProcess(Mat inputImg) override;

	void infer() override;

	void detectDataProcess(std::vector<Ball> &detectedBalls, std::vector<int> &pickedBallsIndex, int cameraId) override;

public:
	OvEngineLoader(std::string modelPath, std::string device, float minObjectness, float minConfidence, float maxIou);

	void detect(cv::Mat inputImg, std::vector<Ball> &detectedBalls, std::vector<int> &pickedBallsIndex, int cameraId) override;
};

#endif