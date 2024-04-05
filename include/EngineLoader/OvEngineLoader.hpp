#pragma once

#if defined(WITH_OPENVINO)

#include "openvino/openvino.hpp"
#include "opencv2/opencv.hpp"
#include "Ball.hpp"
#include "Functions.hpp"
#include "Constants.hpp"

class OvEngineLoader
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

public:
	OvEngineLoader(std::string modelPath, std::string device, float minObjectness, float minConfidence, float maxIou);

	void imgProcess(Mat inputImg);

	void infer();

	void detectDataProcess(std::vector<Ball> &detectedBalls_, std::vector<int> &pickedBallsIndex_, int cameraId);
};

#endif