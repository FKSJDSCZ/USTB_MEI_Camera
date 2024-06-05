#pragma once

#include <thread>
#include <queue>
#include <atomic>
#include <mutex>
#include "Entity/Parameters.hpp"


class RsCameraLoader
{
private:
	int getFrameFromHardware(FrameData &frameData);

	int reconnect();

	float pixelOffset_[17][2] = {{0,  0},
	                             {0,  3},
	                             {0,  -3},
	                             {3,  0},
	                             {-3, 0},
	                             {3,  3},
	                             {-3, -3},
	                             {3,  -3},
	                             {-3, 3},
	                             {0,  5},
	                             {0,  -5},
	                             {5,  0},
	                             {-5, 0},
	                             {5,  5},
	                             {-5, -5},
	                             {5,  -5},
	                             {-5, 5}};
	cv::Mat_<float> pitchRotateMatrix_ = cv::Mat_<float>(3, 3);
	cv::Mat_<float> yawRotateMatrix_ = cv::Mat_<float>(3, 3);

	rs2::pipeline pipe_;
	rs2::config config_;
	rs2::align alignToColor_ = rs2::align(RS2_STREAM_COLOR);
	rs2::frameset currentFrameSet_;

	std::mutex queueMutex_;
	std::queue<FrameData> frameQueue_;

public:
	int cameraId_;
	int cameraType_;
	int imgWidth_;
	int imgHeight_;
	int framerate_;
	bool isRunning_ = true;
	Parameters parameters_;
	std::string serialNumber_;

	RsCameraLoader(int cameraId, int cameraType, int imgWidth, int imgHeight, int framerate, Parameters parameters, std::string serialNumber);

	void init();

	int startPipe();

	void updateFrame();

	int getCurrentFrame(long currentTimeStamp, cv::Mat &colorImage);

	cv::Point3f getCameraPosition(const cv::Point2f &graphCenter);

	void stopPipe();
};