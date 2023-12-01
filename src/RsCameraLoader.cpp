#include "RsCameraLoader.hpp"

RsCameraLoader::RsCameraLoader() = default;

RsCameraLoader::RsCameraLoader(int imgWidth, int imgHeight, int framerate, float pitchAngleDegree, float yawAngleDegree, Parameters parameters) :
		imgWidth_(imgWidth), imgHeight_(imgHeight), framerate_(framerate), pitchAngleDegree_(pitchAngleDegree), yawAngleDegree_(yawAngleDegree),
		parameters_(parameters)
{}

RsCameraLoader::~RsCameraLoader()
{
	if (pipeStarted_)
	{
		pipe_.stop();
	}
}

//相机初始化
void RsCameraLoader::init(std::string &serialNumber)
{
	config_.enable_device(serialNumber);
	config_.enable_stream(RS2_STREAM_COLOR, imgWidth_, imgHeight_, RS2_FORMAT_BGR8, framerate_);
	config_.enable_stream(RS2_STREAM_DEPTH, imgWidth_, imgHeight_, RS2_FORMAT_Z16, framerate_);
	pipe_.start(config_);
	pipeStarted_ = true;

	pitchRotateMatrix_ =
			(Mat_<float>(3, 3) << 1, 0, 0, 0, std::cos(pitchAngleDegree_ * CV_PI / 180), -std::sin(pitchAngleDegree_ * CV_PI / 180), 0, std::sin(
					pitchAngleDegree_ * CV_PI / 180), std::cos(pitchAngleDegree_ * CV_PI / 180));
	yawRotateMatrix_ = (Mat_<float>(3, 3) << std::cos(yawAngleDegree_ * CV_PI / 180), 0, std::sin(yawAngleDegree_ * CV_PI / 180), 0, 1, 0, -std::sin(
			yawAngleDegree_ * CV_PI / 180), 0, std::cos(yawAngleDegree_ * CV_PI / 180));
}

//获取彩色图
void RsCameraLoader::getImg()
{
	frameSet_ = pipe_.wait_for_frames();
	frameSet_ = alignToColor_.process(frameSet_);
	rs2::video_frame colorFrame = frameSet_.get_color_frame();
	colorImg_ = Mat(Size(imgWidth_, imgHeight_), CV_8UC3, (void *) colorFrame.get_data(), Mat::AUTO_STEP);
}

//获取相机坐标系坐标
void RsCameraLoader::getCameraPosition(float centerX, float centerY, Point3f &cameraPosition)
{
	rs2::depth_frame depthFrame = frameSet_.get_depth_frame();
	rs2::video_stream_profile depthProfile = depthFrame.get_profile().as<rs2::video_stream_profile>();
	rs2_intrinsics internReference_ = depthProfile.get_intrinsics();

	//邻近采样防止深度黑洞
	float position[3];
	for (auto &i: pixelOffset_)
	{
		float point[2] = {centerX + i[0], centerY + i[1]};
		if (point[0] >= 0 && point[0] < colorImg_.cols && point[1] >= 0 && point[1] < colorImg_.rows)
		{
			float depthValue = depthFrame.get_distance(point[0], point[1]);
			rs2_deproject_pixel_to_point(position, &internReference_, point, depthValue);
			if (position[0] || position[1] || position[2])
			{
				break;
			}
		}
	}

	Mat positionMatrix = (Mat_<float>(3, 1) << position[0], position[1], position[2]);
	positionMatrix = yawRotateMatrix_ * pitchRotateMatrix_ * positionMatrix;
	cameraPosition = {positionMatrix.at<float>(0), positionMatrix.at<float>(1), positionMatrix.at<float>(2)};
}