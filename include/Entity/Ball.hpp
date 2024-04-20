#pragma once

#include "Parameters.hpp"

using namespace cv;

class Ball :
		public Rect_<float>
{
public:
	float centerX_, centerY_, confidence_, distance_;
	int id_, labelNum_, cameraId_;
	bool isInBasket_;
	Point3f cameraPosition_;

	Ball(int id, float centerX, float centerY, int labelNum, float confidence, int cameraId, bool isInBasket);

	void toMillimeter();

	void offsetToEncodingDisk(Parameters &parameters);
};