#pragma once

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <grp.h>
#include "CameraLoader/WideFieldCameraLoader.hpp"
#include "EngineLoader/IEngineLoader.hpp"
#include "Processor/FrontDataProcessor.hpp"

class WideFieldCameraGroup
{
private:
	int devIndex_;

public:
	WideFieldCameraLoader wideFieldCamera_;

	void detectWideFieldCamera();

	void groupInit();

	void groupDetect(IEngineLoader &engineLoader, FrontDataProcessor &frontDataProcessor);

	void groupDrawBoxes(FrontDataProcessor &frontDataProcessor);
};