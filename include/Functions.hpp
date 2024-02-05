#pragma once

#include "Ball.hpp"

class Functions
{
public:
	static float calcIou(Ball &ball1, Ball &ball2);

	static float calcDistance3f(Point3f cameraPosition1, Point3f cameraPosition2);

	static float calcDistance2f(Point2f pixelPosition1, Point2f pixelPosition2);

	static double calcGradient(Ball &origin, Ball &target);
};