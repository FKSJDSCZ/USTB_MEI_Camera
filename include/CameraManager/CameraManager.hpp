#pragma once

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <grp.h>
#include "CameraLoader/RsCameraLoader.hpp"
#include "CameraLoader/WideFieldCameraLoader.hpp"
#include "Processor/BackDataProcessor.hpp"
#include "Processor/FrontDataProcessor.hpp"
#include "EngineLoader/IEngineLoader.hpp"

class CameraManager
{
private:
	std::unordered_map<std::string, Parameters> paramsMap_ = {
			{"318122303126",
					Parameters(-210, -430, -50, -25, 0, 1.15)},
			{"135122251159",
					Parameters(230, -600, -80, -35, 0, 1.13)}
	};
	const std::string frontCameraSerialNumber_ = "308222301027";

public:
	int cameraCount_ = 0;
	std::vector<RsCameraLoader> backCameras_;
	std::vector<RsCameraLoader> frontCameras_;
	BackDataProcessor backDataProcessor_;
	FrontDataProcessor frontDataProcessor_;

	CameraManager();

	void initRsCamera();

	void detect(IEngineLoader &engineLoader);

	void outputData(DataSender &dataSender);

	void drawBoxes();

	void showImages();

	void saveVideos();

	void resetProcessors();
};
