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
	float confidence_;
	std::vector<BallPosition> ballPositions_;

	Point3f cameraPosition();

	Rect2f graphRect();

	Point2f graphCenter();

	void merge(Ball &ball);

	void addGraphPosition(float centerX, float centerY, float width, float height, float confidence, int labelNum, int cameraId, bool isInBasket);

	void setCameraPosition(RsCameraLoader *rsCameraArray);

	void toMillimeter();

	void offsetToEncodingDisk(RsCameraLoader *rsCameraArray);

	void calcDistance();
};