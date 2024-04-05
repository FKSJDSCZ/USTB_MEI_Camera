#include "Util/Functions.hpp"

float Functions::calcIou(Ball &ball1, Ball &ball2)
{
	Rect_<float> interRect = ball1 & ball2;
	float interArea = interRect.area();
	float unionArea = ball1.area() + ball2.area() - interArea;
	return interArea / unionArea;
}

float Functions::calcDistance3f(Point3f cameraPosition1, Point3f cameraPosition2)
{
	return std::sqrt(pow(cameraPosition1.x - cameraPosition2.x, 2) + pow(cameraPosition1.y - cameraPosition2.y, 2)
	                 + pow(cameraPosition1.z - cameraPosition2.z, 2));
}

float Functions::calcDistance2f(Point2f pixelPosition1, Point2f pixelPosition2)
{
	return std::sqrt(pow(pixelPosition1.x - pixelPosition2.x, 2) + pow(pixelPosition1.y - pixelPosition2.y, 2));
}

double Functions::calcGradient(Ball &origin, Ball &target)
{
	int deltaX = static_cast<int>(target.centerX_ - origin.centerX_);
	int deltaY = static_cast<int>(target.centerY_ - origin.centerY_);
	double tangent = static_cast<double>(deltaY) / deltaX;
	return tangent;
}