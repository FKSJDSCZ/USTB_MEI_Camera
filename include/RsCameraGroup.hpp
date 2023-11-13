#pragma once

#include "RsCameraLoader.hpp"
#include "TrtEngineLoader.hpp"
#include "BackDataProcessor.hpp"

class RsCameraGroup
{
private:
	rs2::context context_;
	rs2::device_list deviceList_;
	std::string serialNumbers_[2] = {"135122251159", "318122301624"};
	Parameters parameters_[2] = {Parameters(373, -510, 103), Parameters(353, -712, -20)};
	float pitchAngleDegrees_[2] = {-57.0, 0};
	bool enabled_[2] = {false, false};

public:
	RsCameraLoader rsCamerasArray_[2];

	void detectRsCamera();

	void groupInit();

	void groupGetImg();

	void groupInfer(TrtEngineLoader &trtEngineLoader, BackDataProcessor &backDataProcessor);

	void groupDataProcess(BackDataProcessor &backDataProcessor);

	void groupDrawBoxes(BackDataProcessor &backDataProcessor);
};