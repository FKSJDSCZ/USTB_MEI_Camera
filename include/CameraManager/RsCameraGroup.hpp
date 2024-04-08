#pragma once

#include "CameraLoader/RsCameraLoader.hpp"
#include "EngineLoader/IEngineLoader.hpp"
#include "Processor/BackDataProcessor.hpp"

class RsCameraGroup
{
private:
	rs2::context context_;
	rs2::device_list deviceList_;
	std::string serialNumbers_[2] = {"318122303126", "308222301027"};
	Parameters parameters_[2] = {Parameters(200, -615, 410), Parameters(210, -490, 355)};
	float pitchAngleDegrees_[2] = {-50.0, -15.0};
	float yawAngleDegrees_[2] = {-20.0, -30.0};
	bool enabled_[2] = {false, false};

public:
	RsCameraLoader rsCamerasArray_[2];

	void detectRsCamera();

	void groupInit();

	void groupDetect(IEngineLoader &engineLoader, BackDataProcessor &backDataProcessor);

	void groupDataProcess(BackDataProcessor &backDataProcessor);

	void groupDrawBoxes(BackDataProcessor &backDataProcessor);
};