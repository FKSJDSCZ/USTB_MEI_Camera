#pragma once

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <grp.h>
#include "CameraLoader/RsCameraLoader.hpp"
#include "CameraLoader/WideFieldCameraLoader.hpp"
#include "Processor/BackDataProcessor.hpp"
#include "Processor/FrontDataProcessor.hpp"
#include "EngineLoader/IEngineLoader.hpp"
#include "Util/Logger.hpp"

class CameraManager
{
private:
#if defined(IS_R2_GEN2_VER1)
	std::unordered_map<std::string, Parameters> paramsMap_ = {
			{"308222301027",
					Parameters(240, -605, 440, -50, -26, 1.1)},
			{"318122303126",
					Parameters(230, -490, 335, -15, -30, 1.1)}
	};
#else
	std::unordered_map<std::string, Parameters> paramsMap_ = {
			{"135122251159",
					Parameters(-210, -430, -50, -25, 0, 1.15)},
			{"318122301624",
					Parameters(230, -600, -80, -35, 0, 1.13)}
	};
#endif

public:
	int cameraCount_ = 0;
	std::vector<RsCameraLoader> rsCameras_;
	std::vector<WideFieldCameraLoader> wideFieldCameras_;
	BackDataProcessor backDataProcessor_;
	FrontDataProcessor frontDataProcessor_;

	CameraManager();

	void initRsCamera();

	void initWFCamera();

	void detect(IEngineLoader &engineLoader);

	void outputData(DataSender &dataSender);

	void drawBoxes();

	void showImages();

	void saveVideos();

	void resetProcessors();
};
