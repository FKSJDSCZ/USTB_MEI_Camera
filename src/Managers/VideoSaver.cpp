#include "Managers/VideoSaver.hpp"

void VideoSaver::start(CameraManager &cameraManager)
{
	for (auto &camera: cameraManager.cameras_)
	{
		if (camera->cameraType() & FRONT_WF_CAMERA)
		{
			std::shared_ptr<WideFieldCameraLoader> WfCamera = std::static_pointer_cast<WideFieldCameraLoader>(camera);
			videoWriters_.push_back(
					cv::VideoWriter(
							"../videos/video" + std::to_string(WfCamera->devIndex_) + ".mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
							WfCamera->framerate_, {WfCamera->imgWidth_, WfCamera->imgHeight_}
					)
			);
		}
		else
		{
			std::shared_ptr<RsCameraLoader> RsCamera = std::static_pointer_cast<RsCameraLoader>(camera);
			videoWriters_.push_back(
					cv::VideoWriter(
							"../videos/RS_" + RsCamera->serialNumber_ + ".mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
							RsCamera->framerate_, {RsCamera->imgWidth_, RsCamera->imgHeight_}
					)
			);
		}
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
		if (cameraImage.cameraType_ & FRONT_CAMERA)
		{
			videoWriters_.at(cameraImage.cameraId_).write(cameraImage.colorImage_);
		}
	}
}

void VideoSaver::finish()
{
	for (cv::VideoWriter &videoWriter: videoWriters_)
	{
		videoWriter.release();
	}
}
