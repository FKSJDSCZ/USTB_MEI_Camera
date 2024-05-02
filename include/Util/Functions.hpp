#pragma once

#include "opencv2/opencv.hpp"

using namespace cv;

class Functions
{
public:
	static float calcIou(Rect2f rect1, Rect2f rect2);

	static float calcDistance3f(Point3f cameraPosition1, Point3f cameraPosition2 = Point3f(0, 0, 0));

	static float calcDistance2f(Point2f pixelPosition1, Point2f pixelPosition2 = Point2f(0, 0));
};