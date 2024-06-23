#pragma once

#include "Loaders/ICameraLoader.hpp"


class WideFieldCameraLoader :
		public ICameraLoader
{
private:
	int getFrameFromHardware(CvFrameData &frameData);

	int reconnect();

	cv::VideoCapture cap_;
	std::mutex queueMutex_;
	std::queue<CvFrameData> frameQueue_;

public:
	int cameraId_;
	int cameraType_;
	int imgWidth_ = 640;
	int imgHeight_ = 480;
	int framerate_ = 30;
	bool isRunning_ = true;
	int devIndex_;

	explicit WideFieldCameraLoader(int cameraId, int cameraType, int devIndex);

	int cameraId() override;

	int cameraType() override;

	void init() override;

	int start() override;

	void updateFrame() override;

	int getCurrentFrame(long currentTimeStamp, cv::Mat &colorImage) override;

	void stop() override;
};
