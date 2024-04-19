#pragma once

#include "opencv2/opencv.hpp"
#include "Entity/Ball.hpp"

class IEngineLoader
{
protected:
	virtual void imgProcess(Mat inputImg) = 0;

	virtual void infer() = 0;

	virtual void detectDataProcess(std::vector<Ball> &pickedBalls, int cameraId) = 0;

public:
	virtual void detect(Mat inputImg, std::vector<Ball> &pickedBalls, int cameraId) = 0;

	virtual ~IEngineLoader() = default;
};