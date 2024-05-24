#pragma once

#include "CameraLoader/RsCameraLoader.hpp"
#include "Entity/BallPosition.hpp"


class Ball
{
public:
	int labelNum_;
	bool isInBasket_;
	bool isValid_;
	float distance_;
	float confidence_ = 0;
	std::vector<BallPosition> ballPositions_;

	int cameraId();

	cv::Point3f cameraPosition();

	cv::Rect2f graphRect();

	cv::Point2f graphCenter();

	void merge(Ball &ball);

	void addGraphPosition(float centerX, float centerY, float width, float height, float confidence, int labelNum, int cameraId, bool isInBasket);

	void setCameraPosition(std::vector<RsCameraLoader> &rsCameras);

	void toMillimeter();

	void offsetToEncodingDisk(std::vector<RsCameraLoader> &rsCameras);

	void calcDistance();
};