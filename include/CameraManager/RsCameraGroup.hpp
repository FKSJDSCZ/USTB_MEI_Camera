#pragma once

#include "CameraLoader/RsCameraLoader.hpp"
#include "EngineLoader/IEngineLoader.hpp"
#include "Processor/BackDataProcessor.hpp"
#include "Util/Logger.hpp"

class RsCameraGroup
{
private:
	rs2::context context_;
	rs2::device_list deviceList_;
	std::string serialNumbers_[2] = {"308222301027", "318122303126"};
	Parameters parameters_[2] = {Parameters(240, -605, 440, 1.1),
	                             Parameters(230, -490, 335, 1.1)};
	float pitchAngleDegrees_[2] = {-50.0, -15.0};
	float yawAngleDegrees_[2] = {-26.0, -30.0};
	bool enabled_[2] = {false, false};

public:
	RsCameraLoader rsCamerasArray_[2];
	BackDataProcessor backDataProcessor_;

	void detectRsCamera();

	void groupInit();

	void groupDetect(IEngineLoader &engineLoader);

	void groupDrawBoxes();

	void groupShowImages();

	void groupSaveVideos();
};