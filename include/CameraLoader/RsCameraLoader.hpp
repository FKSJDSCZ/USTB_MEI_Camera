#pragma once

#include "librealsense2/rs.hpp"
#include "Entity/Parameters.hpp"

using namespace cv;

class RsCameraLoader
{
private:
	int imgWidth_;
	int imgHeight_;
	int framerate_;
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
	float pitchAngleDegree_;//俯角相反数
	float yawAngleDegree_;
	bool pipeStarted_ = false;
	/*
	 * ⌈    1,      0,       0      ⌉
	 * |    0,  cos(θ), -sin(θ) |
	 * ⌊    0,  sin(θ), cos(θ)  ⌋
	 */
	Mat_<float> pitchRotateMatrix_ = Mat_<float>(3, 3);
	/*
	 * ⌈        0,  cos(θ),   -sin(θ)   ⌉
	 * |        0,          1,          0    |
	 * ⌊-sin(θ),          0,    cos(θ)  ⌋
	 */
	Mat_<float> yawRotateMatrix_ = Mat_<float>(3, 3);
	rs2::config config_;
	rs2::align alignToColor_ = rs2::align(RS2_STREAM_COLOR);
	rs2::pipeline pipe_;
	rs2::frameset frameSet_;

public:
	Mat colorImg_;
	Parameters parameters_;

	RsCameraLoader();

	RsCameraLoader(int imgWidth, int imgHeight, int framerate, float pitchAngleDegree, float yawAngleDegree, Parameters parameters);

	~RsCameraLoader();

	//相机初始化
	void init(std::string &serialNumber);

	//获取彩色图
	void getImg();

	//获取相机坐标系坐标
	void getCameraPosition(float centerX, float centerY, Point3f &cameraPosition);
};