#pragma once

#include "opencv2/opencv.hpp"

using namespace cv;

class Parameters
{
public:
	float offsetToEncodingDiskX_;
	float offsetToEncodingDiskY_;
	float offsetToEncodingDiskZ_;
	Point3f zeroPointToEncodingDisk_;

	Parameters();

	Parameters(float offsetToEncodingDiskX, float offsetToEncodingDiskY, float offsetToEncodingDiskZ);

	void parametersInit();
};