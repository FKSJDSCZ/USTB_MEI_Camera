#pragma once

#include <thread>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <grp.h>
#include "Processor/BackDataProcessor.hpp"
#include "Processor/FrontDataProcessor.hpp"
#include "EngineLoader/IEngineLoader.hpp"

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

	static void reconnect(std::shared_ptr<RsCameraLoader> &rsCamera);

	void getImageFromCameras(std::vector<std::shared_ptr<RsCameraLoader>> &rsCameras);

public:
	int cameraCount_ = 0;
	std::vector<std::shared_ptr<RsCameraLoader>> backCameras_;
	std::vector<std::shared_ptr<RsCameraLoader>> frontCameras_;
	std::vector<std::shared_ptr<RsCameraLoader>> disConnectedCameras_;
	BackDataProcessor backDataProcessor_;
	FrontDataProcessor frontDataProcessor_;

	CameraManager();

	void initRsCamera();

	void checkCameraStatus();

	void detect();

	void outputData(DataSender &dataSender);

	void drawBoxes();

	void showImages();

	void saveVideos();

	void resetProcessors();
};
