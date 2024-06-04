#pragma once

#include <thread>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <grp.h>
#include "Loaders/RsCameraLoader.hpp"

class CameraManager
{
private:
	std::unordered_map<std::string, Parameters> paramsMap_ = {
			{"318122303126",
					Parameters(-210, -430, -30, -25, 0, 1.15)},
			{"135122251159",
					Parameters(240, -590, -50, -35, 0, 1.13)}
	};
	const std::string frontCameraSerialNumber_ = "308222301027";

public:
	int cameraCount_ = 0;
	std::vector<std::shared_ptr<RsCameraLoader>> rsCameras_;

	void initRsCamera();

	void startUpdateThread();

	void getCameraImage(std::vector<CameraImage> &cameraImages);

	void stopUpdateThread();
};
