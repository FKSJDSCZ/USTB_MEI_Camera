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
	Parameters parameters_[2] = {Parameters(290, -405, 460), Parameters(260, -520, 365)};
	float pitchAngleDegrees_[2] = {-35.0, -15.0};
	float yawAngleDegrees_[2] = {-20.0, -30.0};
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