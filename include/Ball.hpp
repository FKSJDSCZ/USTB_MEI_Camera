#pragma once

#include "Parameters.hpp"

using namespace cv;

class Ball :
		public Rect_<float>
{
public:
	float centerX_, centerY_, confidence_, distance_;
	int labelNum_, cameraId_;
	Point3f cameraPosition_;

	Ball(float centerX, float centerY, int labelNum, float confidence, int cameraId);

	void toMillimeter();

	void offsetToEncodingDisk(Parameters &parameters);
};