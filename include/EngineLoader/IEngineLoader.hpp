#pragma once

#include "opencv2/opencv.hpp"
#include "Entity/Ball.hpp"

class IEngineLoader
{
public:
	virtual void imgProcess(Mat inputImg) = 0;

	virtual void infer() = 0;

	virtual void detectDataProcess(std::vector<Ball> &detectedBalls, std::vector<int> &pickedBallsIndex, int cameraId) = 0;

	virtual void detect(Mat inputImg, std::vector<Ball> &detectedBalls, std::vector<int> &pickedBallsIndex, int cameraId) = 0;

	virtual ~IEngineLoader() = default;
};