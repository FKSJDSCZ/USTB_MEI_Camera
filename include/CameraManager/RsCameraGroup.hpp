#pragma once

#include "CameraLoader/RsCameraLoader.hpp"
#include "EngineLoader/IEngineLoader.hpp"
#include "Processor/BackDataProcessor.hpp"

class RsCameraGroup
{
private:
	rs2::context context_;
	rs2::device_list deviceList_;
	std::string serialNumbers_[2] = {"135122251159", "318122301624"};
	Parameters parameters_[2] = {Parameters(30, -760, 230), Parameters(260, -630, -415)};
	float pitchAngleDegrees_[2] = {-75.0, -20.0};
	float yawAngleDegrees_[2] = {0, 0};
	bool enabled_[2] = {false, false};

public:
	RsCameraLoader rsCamerasArray_[2];

	void detectRsCamera();

	void groupInit();

	void groupDetect(IEngineLoader &engineLoader, BackDataProcessor &backDataProcessor);

	void groupDataProcess(BackDataProcessor &backDataProcessor);

	void groupDrawBoxes(BackDataProcessor &backDataProcessor);
};