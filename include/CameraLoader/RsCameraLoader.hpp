#pragma once

#include <atomic>
#include <mutex>
#include "librealsense2/rs.hpp"
#include "Entity/Parameters.hpp"


class RsCameraLoader
{
private:
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
	                             {-5, 5}};//深度采样的像素坐标偏移量
	/*
	 * ⌈    1,      0,       0      ⌉
	 * |    0,  cos(θ), -sin(θ) |
	 * ⌊    0,  sin(θ), cos(θ)  ⌋
	 */
	cv::Mat_<float> pitchRotateMatrix_ = cv::Mat_<float>(3, 3);
	/*
	 * ⌈ cos(θ),         0,     sin(θ)  ⌉
	 * |        0,          1,          0   |
	 * ⌊-sin(θ),          0,    cos(θ)  ⌋
	 */
	cv::Mat_<float> yawRotateMatrix_ = cv::Mat_<float>(3, 3);

	rs2::pipeline *pipe_;
	rs2::config *config_;
	rs2::align alignToColor_ = rs2::align(RS2_STREAM_COLOR);
	rs2::frameset frameSet_;
	cv::VideoWriter videoWriter_;

public:
	enum CameraType
	{
		FRONT_CAMERA = 0,
		BACK_CAMERA = 1
	};

	int cameraId_;
	int cameraType_;
	std::string serialNumber_;
	int imgWidth_;
	int imgHeight_;
	int framerate_;
	cv::Mat colorImg_;
	Parameters parameters_;
	std::atomic<bool> isConnected_{true};
	std::mutex mutex_;

	RsCameraLoader(int cameraId, int cameraType, int imgWidth, int imgHeight, int framerate, Parameters parameters, std::string serialNumber);

	void init();

	void startPipe();

	void resetPipe();

	int getImage();

	cv::Point3f getCameraPosition(const cv::Point2f &graphCenter);

	void saveImage();

	void stopPipe();

	~RsCameraLoader();
};