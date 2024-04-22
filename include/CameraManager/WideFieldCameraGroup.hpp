#pragma once

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <grp.h>
#include "CameraLoader/WideFieldCameraLoader.hpp"
#include "EngineLoader/IEngineLoader.hpp"
#include "Processor/FrontDataProcessor.hpp"
#include "Util/Logger.hpp"

class WideFieldCameraGroup
{
private:
	int devIndex_ = 0;

public:
	WideFieldCameraLoader wideFieldCamera_;
	FrontDataProcessor frontDataProcessor_;

	void detectWideFieldCamera();

	void groupInit();

	void groupDetect(IEngineLoader &engineLoader);

	void groupDrawBoxes();

	void groupShowImages() const;

	void groupSaveVideos();
};