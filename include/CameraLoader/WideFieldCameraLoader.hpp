#pragma once

#include "opencv2/opencv.hpp"

class WideFieldCameraLoader
{
private:
	cv::VideoCapture cap_;
	cv::VideoWriter videoWriter_;
	int framerate_ = 30;
	int imgWidth_ = 640;
	int imgHeight_ = 480;

public:
	cv::Mat colorImg_;
	int cameraId_;

	explicit WideFieldCameraLoader(int cameraId);

	void init(int devIndex);

	void getImage();

	void saveImage();

	~WideFieldCameraLoader();
};