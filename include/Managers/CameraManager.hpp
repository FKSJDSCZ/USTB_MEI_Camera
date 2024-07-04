#pragma once

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <grp.h>
#include <fcntl.h>
#include "Loaders/RsCameraLoader.hpp"
#include "Loaders/WideFieldCameraLoader.hpp"

class CameraManager
{
private:
	std::unordered_map<std::string, Parameters> paramsMap_ = {
			{
					"318122303126",
					Parameters(-150, -400, -35, -25, 0, 1.15)
			},
			{
					"318122301624",
					Parameters(190, -490, -55, -35, 0, 1.13)
			}
	};
	const std::string frontCameraSerialNumber_ = "308222301027";

public:
	int cameraCount_ = 0;
	std::vector<std::shared_ptr<ICameraLoader>> cameras_;

	void initRsCamera();

	void initWFCamera();

	void startUpdateThread();

	void getCameraImage(std::vector<CameraImage> &cameraImages);

	void stopUpdateThread();
};
