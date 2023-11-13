#pragma once

#include "opencv2/opencv.hpp"

using namespace cv;

class WideFieldCameraLoader
{
private:
	VideoCapture cap_;

public:
	Mat colorImg_;

	WideFieldCameraLoader();

	void init(int devIndex);

	void getImg();
};