#include "WideFieldCameraLoader.hpp"

WideFieldCameraLoader::WideFieldCameraLoader() = default;

void WideFieldCameraLoader::init(int devIndex)
{
	cap_ = VideoCapture(200 + devIndex);
//	cap_.set(CAP_PROP_FRAME_WIDTH,1080);
//	cap_.set(CAP_PROP_FRAME_HEIGHT,720);
}

void WideFieldCameraLoader::getImg()
{
	cap_.read(colorImg_);
//	colorImg_ = Mat(colorImg_, Rect(300, 300, 480, 220));
}