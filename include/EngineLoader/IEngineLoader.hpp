#pragma once

#include "opencv2/opencv.hpp"
#include "Entity/Ball.hpp"
#include "Util/Functions.hpp"
#include "Constants.hpp"
#include "Util/Logger.hpp"

class IEngineLoader
{
public:
	virtual void imgProcess(Mat inputImg, int imageId) = 0;

	virtual void infer() = 0;

	virtual void detectDataProcess() = 0;

	virtual void getBallsByCameraId(int cameraId, std::vector<Ball> &container) = 0;

	virtual ~IEngineLoader() = default;
};