#include "WideFieldCameraLoader.hpp"

WideFieldCameraLoader::WideFieldCameraLoader() = default;

void WideFieldCameraLoader::init(int devIndex)
{
	cap_ = VideoCapture(200 + devIndex);
	cap_.set(CAP_PROP_FRAME_WIDTH, 1920);
	cap_.set(CAP_PROP_FRAME_HEIGHT, 1080);
	cap_.set(CAP_PROP_FPS, 30);
}

void WideFieldCameraLoader::getImg()
{
	cap_.read(colorImg_);
}