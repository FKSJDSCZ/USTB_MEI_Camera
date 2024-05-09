#include "CameraLoader/RsCameraLoader.hpp"

RsCameraLoader::RsCameraLoader(int cameraId, int imgWidth, int imgHeight, int framerate, Parameters parameters) :
		cameraId_(cameraId), imgWidth_(imgWidth), imgHeight_(imgHeight), framerate_(framerate), parameters_(parameters)
{}

void RsCameraLoader::init(std::string &serialNumber)
{
	config_.enable_device(serialNumber);
	config_.enable_stream(RS2_STREAM_COLOR, imgWidth_, imgHeight_, RS2_FORMAT_BGR8, framerate_);
	config_.enable_stream(RS2_STREAM_DEPTH, imgWidth_, imgHeight_, RS2_FORMAT_Z16, framerate_);
	pipe_.start(config_);

	pitchRotateMatrix_ = (Mat_<float>(3, 3) <<
	                                        1, 0, 0,
			0, std::cos(parameters_.pitchAngle_ * CV_PI / 180), -std::sin(parameters_.pitchAngle_ * CV_PI / 180),
			0, std::sin(parameters_.pitchAngle_ * CV_PI / 180), std::cos(parameters_.pitchAngle_ * CV_PI / 180));
	yawRotateMatrix_ = (Mat_<float>(3, 3) <<
	                                      std::cos(parameters_.yawAngle_ * CV_PI / 180), 0, std::sin(parameters_.yawAngle_ * CV_PI / 180),
			0, 1, 0,
			-std::sin(parameters_.yawAngle_ * CV_PI / 180), 0, std::cos(parameters_.yawAngle_ * CV_PI / 180));

	videoWriter_.open("../videos/RS" + serialNumber + ".mp4", VideoWriter::fourcc('m', 'p', '4', 'v'), framerate_, Size(imgWidth_, imgHeight_));
}

void RsCameraLoader::getImage()
{
	frameSet_ = pipe_.wait_for_frames();
	frameSet_ = alignToColor_.process(frameSet_);
	rs2::video_frame colorFrame = frameSet_.get_color_frame();
	colorImg_ = Mat(Size(imgWidth_, imgHeight_), CV_8UC3, (void *) colorFrame.get_data(), Mat::AUTO_STEP);
}

Point3f RsCameraLoader::getCameraPosition(const Point2f &graphCenter)
{
	rs2::depth_frame depthFrame = frameSet_.get_depth_frame();
	auto depthProfile = depthFrame.get_profile().as<rs2::video_stream_profile>();
	rs2_intrinsics internReference = depthProfile.get_intrinsics();

	//邻近采样防止深度黑洞
	float position[3];
	Rect2i imgRect = Rect2i(0, 0, colorImg_.cols, colorImg_.rows);
	for (auto &offset: pixelOffset_)
	{
		float point[2] = {graphCenter.x + offset[0], graphCenter.y + offset[1]};
		if (imgRect.contains(Point2i(point[0], point[1])))
		{
			float depthValue = depthFrame.get_distance(point[0], point[1]);
			rs2_deproject_pixel_to_point(position, &internReference, point, depthValue);
			if (position[0] || position[1] || position[2])
			{
				break;
			}
		}
	}

	Mat positionMatrix = (Mat_<float>(3, 1) << position[0], position[1], position[2]);
	positionMatrix = yawRotateMatrix_ * pitchRotateMatrix_ * positionMatrix;
	return {positionMatrix.at<float>(0), positionMatrix.at<float>(1), positionMatrix.at<float>(2)};
}

void RsCameraLoader::saveImage()
{
	videoWriter_.write(colorImg_);
}

RsCameraLoader::~RsCameraLoader()
{
	videoWriter_.release();
}
