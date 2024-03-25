#include "CameraLoader/WideFieldCameraLoader.hpp"

WideFieldCameraLoader::WideFieldCameraLoader() = default;

void WideFieldCameraLoader::init(int devIndex)
{
	cap_ = VideoCapture(200 + devIndex);
//	cap_.set(CAP_PROP_FRAME_WIDTH,1280);
//	cap_.set(CAP_PROP_FRAME_HEIGHT,720);
}

void WideFieldCameraLoader::getImg()
{
	cap_.read(colorImg_);
//	colorImg_=colorImg_(Rect(420,0,1080,1080));
}