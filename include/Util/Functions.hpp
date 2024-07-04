#pragma once

#include "opencv2/opencv.hpp"
#include "define.hpp"

class Functions
{
public:
	static float calcIou(cv::Rect2f rect1, cv::Rect2f rect2);

	static float calcDistance3f(cv::Point3f cameraPosition1, cv::Point3f cameraPosition2 = cv::Point3f(0, 0, 0));

	static float calcDistanceXZ(cv::Point3f position1, cv::Point3f position2 = cv::Point3f(0, 0, 0));
};