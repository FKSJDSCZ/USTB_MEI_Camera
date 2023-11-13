#pragma once

#include "librealsense2/rs.hpp"
#include "Parameters.hpp"

using namespace cv;

class RsCameraLoader
{
private:
	int imgWidth_;
	int imgHeight_;
	int framerate_;
	float pitchAngleDegree_;
	bool pipeStarted_ = false;
	Mat_<float> rotateMatrix_ = Mat_<float>(3, 3);
	rs2::context context_;
	rs2::device_list deviceList_;
	rs2::config config_;
	rs2::align alignToColor_ = rs2::align(RS2_STREAM_COLOR);
	rs2::pipeline pipe_;
	rs2::frameset frameSet_;

public:
	Mat colorImg_;
	Parameters parameters_;

	RsCameraLoader();

	RsCameraLoader(int imgWidth, int imgHeight, int framerate, float rotateDegree, Parameters parameters);

	~RsCameraLoader();

	//相机初始化
	void init(std::string &serialNumber);

	//获取彩色图
	void getImg();

	//获取相机坐标系坐标
	void getCameraPosition(float centerX, float centerY, Point3f &cameraPosition);
};