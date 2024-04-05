#include "WideFieldCameraGroup.hpp"

void WideFieldCameraGroup::detectWideFieldCamera()
{
	int index = 0;
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

		std::string info = std::string(reinterpret_cast<char *>(cap.card));
		for (int i = 0; i < 2; ++i)
		{
			if (info == cardInfo_[i])
			{
				enabled_[i] = true;
				devIndex_[i] = index;
				break;
			}
		}
		index += 2;
	}

	if (enabled_[0] ^ enabled_[1])
	{
		std::cerr << "[Warning] Detected 1 wide field camera successfully. Please connect more cameras" << std::endl;
	}
	else if (enabled_[0])
	{
		std::cout << "[Info] Detected 2 wide field cameras successfully" << std::endl;
	}
	else
	{
		throw std::runtime_error("[Error] No wide field camera detected");
	}
}

void WideFieldCameraGroup::groupInit()
{
	for (int i = 0; i < 2; ++i)
	{
		if (enabled_[i])
		{
			wideFieldCameraArray_[i] = WideFieldCameraLoader();
			wideFieldCameraArray_[i].init(devIndex_[i]);

			std::cout << "[Info] Wide field camera " << i << " connected. Card info: " << cardInfo_[i] << std::endl;
		}
	}
}

void WideFieldCameraGroup::groupGetImg()
{
	for (int i = 0; i < 2; ++i)
	{
		if (enabled_[i])
		{
			wideFieldCameraArray_[i].getImg();
		}
	}
}

#if defined(WITH_CUDA)
void WideFieldCameraGroup::groupInfer(TrtEngineLoader &trtEngineLoader, FrontDataProcessor &frontDataProcessor)
{
	for (int i = 0; i < 2; ++i)
	{
		if (enabled_[i])
		{
			trtEngineLoader.imgProcess(wideFieldCameraArray_[i].colorImg_);
			trtEngineLoader.infer();
			trtEngineLoader.detectDataProcess(frontDataProcessor.detectedBalls_, frontDataProcessor.pickedBallsIndex_, i);
		}
	}
}
#elif defined(WITH_OPENVINO)

void WideFieldCameraGroup::groupInfer(OvEngineLoader &ovEngineLoader, FrontDataProcessor &frontDataProcessor)
{
	for (int i = 0; i < 2; ++i)
	{
		if (enabled_[i])
		{
			ovEngineLoader.imgProcess(wideFieldCameraArray_[i].colorImg_);
			ovEngineLoader.infer();
			ovEngineLoader.detectDataProcess(frontDataProcessor.detectedBalls_, frontDataProcessor.pickedBallsIndex_, i);
		}
	}
}

#endif

void WideFieldCameraGroup::groupDrawBoxes(FrontDataProcessor &frontDataProcessor)
{
	frontDataProcessor.drawBoxes(wideFieldCameraArray_);
	for (int i = 0; i < 2; ++i)
	{
		if (enabled_[i])
		{
			imshow("Wide field camera " + std::to_string(i), wideFieldCameraArray_[i].colorImg_);
		}
	}
}