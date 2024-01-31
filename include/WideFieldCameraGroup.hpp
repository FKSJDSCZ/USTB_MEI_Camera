#pragma once

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <grp.h>
#include "WideFieldCameraLoader.hpp"

#if defined(WITH_CUDA)
#include "TrtEngineLoader.hpp"
#elif defined(WITH_OPENVINO)

#include "OvEngineLoader.hpp"

#endif

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

#if defined(WITH_CUDA)
	void groupInfer(TrtEngineLoader &trtEngineLoader, FrontDataProcessor &frontDataProcessor);
#elif defined(WITH_OPENVINO)

	void groupInfer(OvEngineLoader &ovEngineLoader, FrontDataProcessor &frontDataProcessor);

#endif

	void groupDrawBoxes(FrontDataProcessor &frontDataProcessor);
};