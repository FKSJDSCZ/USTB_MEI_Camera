#include "CameraLoader/WideFieldCameraLoader.hpp"

WideFieldCameraLoader::WideFieldCameraLoader(int cameraId) : cameraId_(cameraId)
{}

void WideFieldCameraLoader::init(int devIndex)
{
	cap_ = VideoCapture(200 + devIndex);
//	cap_.set(CAP_PROP_FRAME_WIDTH,1280);
//	cap_.set(CAP_PROP_FRAME_HEIGHT,720);

	videoWriter_.open("../videos/Camera" + std::to_string(devIndex) + ".mp4", VideoWriter::fourcc('m', 'p', '4', 'v'), framerate_,
	                  Size(imgWidth_, imgHeight_));
}

void WideFieldCameraLoader::getImage()
{
	cap_.read(colorImg_);
}

void WideFieldCameraLoader::saveImage()
{
	videoWriter_.write(colorImg_);
}

WideFieldCameraLoader::~WideFieldCameraLoader()
{
	cap_.release();
	videoWriter_.release();
}
