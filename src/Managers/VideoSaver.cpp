#include "Managers/VideoSaver.hpp"

void VideoSaver::start(CameraManager &cameraManager)
{
	for (auto &rsCamera: cameraManager.rsCameras_)
	{
		videoWriters_.push_back(
				cv::VideoWriter(
						"../videos/RS" + rsCamera->serialNumber_ + ".mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
						rsCamera->framerate_, {rsCamera->imgWidth_, rsCamera->imgHeight_}
				)
		);
	}
}

void VideoSaver::show(std::vector<CameraImage> &cameraImages)
{
	for (CameraImage &cameraImage: cameraImages)
	{
		cv::imshow(std::to_string(cameraImage.cameraId_), cameraImage.colorImage_);
	}
}

void VideoSaver::write(std::vector<CameraImage> &cameraImages)
{
	for (CameraImage &cameraImage: cameraImages)
	{
		videoWriters_.at(cameraImage.cameraId_).write(cameraImage.colorImage_);
	}
}

void VideoSaver::finish()
{
	for (cv::VideoWriter &videoWriter: videoWriters_)
	{
		videoWriter.release();
	}
}
