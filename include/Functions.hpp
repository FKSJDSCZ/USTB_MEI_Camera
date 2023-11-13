#pragma once

#include "Ball.hpp"

class Functions
{
public:
	static float calcIou(Ball &ball1, Ball &ball2);

	static float calcDistance(Point3f cameraPosition1, Point3f cameraPosition2);
};