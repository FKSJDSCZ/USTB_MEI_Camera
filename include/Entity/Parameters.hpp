#pragma once

#include "opencv2/opencv.hpp"

class Parameters
{
public:
	float XOffsetToDisk_;
	float YOffsetToDisk_;
	float ZOffsetToDisk_;
	float pitchAngle_;
	float yawAngle_;
	float changeRate_;

	Parameters(float XOffsetToDisk, float YOffsetToDisk, float ZOffsetToDisk, float pitchAngle, float yawAngle, float changeRate);
};