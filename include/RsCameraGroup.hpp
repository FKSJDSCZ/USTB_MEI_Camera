#pragma once

#include "RsCameraLoader.hpp"

#if defined(WITH_CUDA)

#include "TrtEngineLoader.hpp"
#elif defined(WITH_OPENVINO)

#include "OvEngineLoader.hpp"

#endif

#include "BackDataProcessor.hpp"

class RsCameraGroup
{
private:
	rs2::context context_;
	rs2::device_list deviceList_;
	std::string serialNumbers_[2] = {"318122303126", "308222301027"};
	Parameters parameters_[2] = {Parameters(15, -370, 500), Parameters(495, -615, 25)};
	float pitchAngleDegrees_[2] = {-45.0, 0};
	float yawAngleDegrees_[2] = {120.0, 0};
	bool enabled_[2] = {false, false};

public:
	RsCameraLoader rsCamerasArray_[2];

	void detectRsCamera();

	void groupInit();

	void groupGetImg();

#if defined(WITH_CUDA)
	void groupInfer(TrtEngineLoader &trtEngineLoader, BackDataProcessor &backDataProcessor);
#elif defined(WITH_OPENVINO)

	void groupInfer(OvEngineLoader &ovEngineLoader, BackDataProcessor &backDataProcessor);

#endif

	void groupDataProcess(BackDataProcessor &backDataProcessor);

	void groupDrawBoxes(BackDataProcessor &backDataProcessor);
};