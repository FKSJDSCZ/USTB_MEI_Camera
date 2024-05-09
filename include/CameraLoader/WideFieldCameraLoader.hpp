#pragma once

#include "opencv2/opencv.hpp"

using namespace cv;

class WideFieldCameraLoader
{
private:
	VideoCapture cap_;
	VideoWriter videoWriter_;
	int framerate_ = 30;
	int imgWidth_ = 640;
	int imgHeight_ = 480;

public:
	Mat colorImg_;
	int cameraId_;

	explicit WideFieldCameraLoader(int cameraId);

	void init(int devIndex);

	void getImage();

	void saveImage();

	~WideFieldCameraLoader();
};