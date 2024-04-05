#include "Functions.hpp"

float Functions::calcIou(Ball &ball1, Ball &ball2)
{
	Rect_<float> interRect = ball1 & ball2;
	float interArea = interRect.area();
	float unionArea = ball1.area() + ball2.area() - interArea;
	return interArea / unionArea;
}

float Functions::calcDistance(Point3f cameraPosition1, Point3f cameraPosition2)
{
	return std::sqrt(pow(cameraPosition1.x - cameraPosition2.x, 2) + pow(cameraPosition1.y - cameraPosition2.y, 2)
	                 + pow(cameraPosition1.z - cameraPosition2.z, 2));
}