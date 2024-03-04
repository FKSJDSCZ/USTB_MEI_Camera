#include "WideFieldCameraLoader.hpp"

WideFieldCameraLoader::WideFieldCameraLoader() = default;

void WideFieldCameraLoader::init(int devIndex)
{
	cap_ = VideoCapture(200 + devIndex);
}

void WideFieldCameraLoader::getImg()
{
	cap_.read(colorImg_);
}