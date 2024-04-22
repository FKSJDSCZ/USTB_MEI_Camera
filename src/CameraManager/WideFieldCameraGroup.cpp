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
		throw std::runtime_error("No wide field camera detected");
	}
	else
	{
		std::cout << "[Info] Wide field camera " << info << " connected" << std::endl;
		Logger::getInstance().writeMsg(Logger::INFO, std::format("Camera {} connected", info));
	}
}

void WideFieldCameraGroup::groupInit()
{
	wideFieldCamera_ = WideFieldCameraLoader();
	wideFieldCamera_.init(devIndex_);
}

void WideFieldCameraGroup::groupDetect(IEngineLoader &engineLoader)
{
	wideFieldCamera_.getImage();
	if (wideFieldCamera_.colorImg_.empty())
	{
		Logger::getInstance().writeMsg(Logger::WARNING, "Ignored empty image from wide field camera");
		return;
	}
	engineLoader.detect(wideFieldCamera_.colorImg_, frontDataProcessor_.pickedBalls_, 0);
	frontDataProcessor_.dataProcess(wideFieldCamera_.colorImg_.cols, wideFieldCamera_.colorImg_.rows);
}

void WideFieldCameraGroup::groupDrawBoxes()
{
	frontDataProcessor_.drawBoxes(wideFieldCamera_);
}

void WideFieldCameraGroup::groupShowImages() const
{
	imshow("Wide field camera 0", wideFieldCamera_.colorImg_);
}

void WideFieldCameraGroup::groupSaveVideos()
{
	wideFieldCamera_.saveImage();
}
