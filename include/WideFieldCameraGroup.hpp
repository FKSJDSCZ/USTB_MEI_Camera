#pragma once

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include "WideFieldCameraLoader.hpp"
#include "TrtEngineLoader.hpp"
#include "FrontDataProcessor.hpp"

class WideFieldCameraGroup
{
private:
	std::string cardInfo_[2] = {"USB 2.0 Camera: Camera001", "USB 2.0 Camera: Camera002"};
	bool enabled_[2] = {false, false};
	int devIndex_[2];

public:
	WideFieldCameraLoader wideFieldCameraArray_[2];

	void detectWideFieldCamera();

	void groupInit();

	void groupGetImg();

	void groupInfer(TrtEngineLoader &trtEngineLoader, FrontDataProcessor &frontDataProcessor);

	void groupDrawBoxes(FrontDataProcessor &frontDataProcessor);
};