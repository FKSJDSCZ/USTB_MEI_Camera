#include "CameraManager/WideFieldCameraGroup.hpp"

void WideFieldCameraGroup::detectWideFieldCamera()
{
	int index = 0;
	std::string info;
	v4l2_capability cap{};
	struct stat statInfo{};
	struct group *group_;
	while (true)
	{
		std::string cameraFilePath = "/dev/video" + std::to_string(index);

		if (stat(cameraFilePath.c_str(), &statInfo) == -1)
		{
			break;
		}
		group_ = getgrgid(statInfo.st_gid);
		if (std::string(group_->gr_name) != "video")
		{
			index += 2;
			continue;
		}

		int fd = open(cameraFilePath.c_str(), O_RDONLY);
		ioctl(fd, VIDIOC_QUERYCAP, &cap);

		info = std::string(reinterpret_cast<char *>(cap.card));
		devIndex_ = index;
		break;
	}

	if (info.empty())
	{
		throw std::runtime_error("[Error] No wide field camera detected");
	}
	else
	{
		std::cout << "[Info] Wide field camera " << info << " connected" << std::endl;
	}
}

void WideFieldCameraGroup::groupInit()
{
	wideFieldCamera_ = WideFieldCameraLoader();
	wideFieldCamera_.init(devIndex_);
}

void WideFieldCameraGroup::groupDetect(IEngineLoader &engineLoader, FrontDataProcessor &frontDataProcessor)
{
	wideFieldCamera_.getImg();
	engineLoader.detect(wideFieldCamera_.colorImg_, frontDataProcessor.detectedBalls_, frontDataProcessor.pickedBallsIndex_, 0);

	frontDataProcessor.frontDataProcess(wideFieldCamera_.colorImg_.cols, wideFieldCamera_.colorImg_.rows, false);
//	if (!frontDataProcessor.baskets_.empty())
//	{
//		frontDataProcessor.detectedBalls_.clear();
//		frontDataProcessor.pickedBallsIndex_.clear();
//		frontDataProcessor.baskets_.clear();
//
//		Mat roi = wideFieldCamera_.colorImg_(frontDataProcessor.basketRoi_);
//		engineLoader.detect(roi, frontDataProcessor.detectedBalls_, frontDataProcessor.pickedBallsIndex_, 0);
//		frontDataProcessor.frontDataProcess(wideFieldCamera_.colorImg_.cols, wideFieldCamera_.colorImg_.rows, false);
//	}
}

void WideFieldCameraGroup::groupDrawBoxes(FrontDataProcessor &frontDataProcessor)
{
	frontDataProcessor.drawBoxes(wideFieldCamera_);
	imshow("Wide field camera 0", wideFieldCamera_.colorImg_);
}
