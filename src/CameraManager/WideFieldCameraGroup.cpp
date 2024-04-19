#include "CameraManager/WideFieldCameraGroup.hpp"

void WideFieldCameraGroup::detectWideFieldCamera()
{
	int index = 0;
	std::string info;
	v4l2_capability cap{};
	struct stat statInfo{};
	struct group *group_;
	while (index <= 20)
	{
		std::string cameraFilePath = "/dev/video" + std::to_string(index);

		if (stat(cameraFilePath.c_str(), &statInfo) == -1)
		{
			index += 2;
			continue;
		}
		group_ = getgrgid(statInfo.st_gid);
		if (std::string(group_->gr_name) != "video")
		{
			index += 2;
			continue;
		}

		int fd = open(cameraFilePath.c_str(), O_RDONLY);
		ioctl(fd, VIDIOC_QUERYCAP, &cap);
		close(fd);

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
	engineLoader.detect(wideFieldCamera_.colorImg_, frontDataProcessor.pickedBalls_, 0);

	frontDataProcessor.frontDataProcess(wideFieldCamera_.colorImg_.cols, wideFieldCamera_.colorImg_.rows);
}

void WideFieldCameraGroup::groupDrawBoxes(FrontDataProcessor &frontDataProcessor)
{
	frontDataProcessor.drawBoxes(wideFieldCamera_);
	imshow("Wide field camera 0", wideFieldCamera_.colorImg_);
}
