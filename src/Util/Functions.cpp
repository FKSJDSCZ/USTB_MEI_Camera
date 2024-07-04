#include "Util/Functions.hpp"

float Functions::calcIou(cv::Rect2f rect1, cv::Rect2f rect2)
{
	float internArea = (rect1 & rect2).area();
	float unionArea = rect1.area() + rect2.area() - internArea;
	return internArea / unionArea;
}

float Functions::calcDistance3f(cv::Point3f cameraPosition1, cv::Point3f cameraPosition2)
{
	return std::sqrt(pow(cameraPosition1.x - cameraPosition2.x, 2)
	                 + pow(cameraPosition1.y - cameraPosition2.y, 2)
	                 + pow(cameraPosition1.z - cameraPosition2.z, 2));
}

float Functions::calcDistanceXZ(cv::Point3f position1, cv::Point3f position2)
{
	return std::sqrt(pow(position1.x - position2.x, 2) + pow(position1.z - position2.z, 2));
}