#include "CameraManager/CameraManager.hpp"

CameraManager::CameraManager() = default;

void CameraManager::initRsCamera()
{
	rs2::context context;
	rs2::device_list deviceList = context.query_devices();

	if (!deviceList.size())
	{
		throw std::runtime_error("No Realsense camera detected");
	}
	else if (deviceList.size() == 1)
	{
		std::cerr << "[Warning] Detected 1 Realsense camera successfully. Please connect more cameras" << std::endl;
	}
	else
	{
		std::cout << "[Info] Detected 2 Realsense cameras successfully" << std::endl;
	}

	for (auto &&rsCamera: deviceList)
	{
		rsCamera.hardware_reset();
		std::string serialNumber = rsCamera.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);

		int cameraId;
		auto it = paramsMap_.find(serialNumber);
		if (it == paramsMap_.end())
		{
			std::cerr << "[Warning] Detected unregistered realsense camera " << serialNumber << std::endl;
			Logger::getInstance().writeMsg(Logger::WARNING, std::format("Detected unregistered realsense camera {}", serialNumber));
			continue;
		}
		else
		{
			cameraId = cameraCount_++;
		}

		rsCameras_.emplace_back(cameraId, 640, 480, 30, it->second);
		rsCameras_.at(cameraId).init(serialNumber);

//		std::vector<rs2::sensor> sensors = rsCamera.query_sensors();
//		rs2::color_sensor colorSensor = rs2::color_sensor(sensors[1]);
//		colorSensor.set_option(RS2_OPTION_HUE, 10);

		std::cout << "[Info] Realsense camera " << cameraId << " connected. Serial number: " << serialNumber << std::endl;
		Logger::getInstance().writeMsg(Logger::INFO, std::format("Realsense camera {}({}) connected", serialNumber, cameraId));
	}
}

void CameraManager::initWFCamera()
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
		break;
	}

	if (info.empty())
	{
		throw std::runtime_error("No wide field camera detected");
	}
	else
	{
		wideFieldCameras_.emplace_back(cameraCount_++);
		wideFieldCameras_.front().init(index);

		std::cout << "[Info] Wide field camera " << info << " connected" << std::endl;
		Logger::getInstance().writeMsg(Logger::INFO, std::format("Wide field camera {} connected", info));
	}
}

void CameraManager::detect(IEngineLoader &engineLoader)
{
	for (RsCameraLoader &rsCamera: rsCameras_)
	{
		rsCamera.getImage();
		if (rsCamera.colorImg_.empty())
		{
			Logger::getInstance().writeMsg(Logger::WARNING,
			                               std::format("Ignored empty image from Realsense camera {}", rsCamera.cameraId_));
		}
		else
		{
			engineLoader.setInput(rsCamera.colorImg_, rsCamera.cameraId_);
		}
	}
	for (WideFieldCameraLoader &wideFieldCamera: wideFieldCameras_)
	{
		wideFieldCamera.getImage();
		if (wideFieldCamera.colorImg_.empty())
		{
			Logger::getInstance().writeMsg(Logger::WARNING,
			                               std::format("Ignored empty image from wide field camera {}", wideFieldCamera.cameraId_));
		}
		else
		{
			engineLoader.setInput(wideFieldCamera.colorImg_, wideFieldCamera.cameraId_);
		}
	}
	engineLoader.preProcess();
	engineLoader.infer();
	engineLoader.postProcess();
	for (RsCameraLoader &rsCamera: rsCameras_)
	{
		engineLoader.getBallsByCameraId(rsCamera.cameraId_, backDataProcessor_.pickedBalls_);
	}
	for (WideFieldCameraLoader &wideFieldCamera: wideFieldCameras_)
	{
		engineLoader.getBallsByCameraId(wideFieldCamera.cameraId_, frontDataProcessor_.pickedBalls_);
	}
	if (!rsCameras_.empty())
	{
		backDataProcessor_.dataProcess(rsCameras_);
	}
	if (!wideFieldCameras_.empty())
	{
		frontDataProcessor_.dataProcess();
	}
}

void CameraManager::outputData(DataSender &dataSender)
{
	if (!rsCameras_.empty())
	{
		backDataProcessor_.outputData(dataSender);
	}
	if (!wideFieldCameras_.empty())
	{
		frontDataProcessor_.outputData(dataSender);
	}
}

void CameraManager::drawBoxes()
{
	if (!rsCameras_.empty())
	{
		backDataProcessor_.drawBoxes(rsCameras_);
	}
	if (!wideFieldCameras_.empty())
	{
		frontDataProcessor_.drawBoxes(wideFieldCameras_);
	}
}

void CameraManager::showImages()
{
	for (RsCameraLoader &rsCamera: rsCameras_)
	{
		imshow("Realsense " + std::to_string(rsCamera.cameraId_), rsCamera.colorImg_);
	}
	for (WideFieldCameraLoader &wideFieldCamera: wideFieldCameras_)
	{
		imshow("Wide field " + std::to_string(wideFieldCamera.cameraId_), wideFieldCamera.colorImg_);
	}
}

void CameraManager::saveVideos()
{
	for (RsCameraLoader &rsCamera: rsCameras_)
	{
		rsCamera.saveImage();
	}
	for (WideFieldCameraLoader &wideFieldCamera: wideFieldCameras_)
	{
		wideFieldCamera.saveImage();
	}
}

void CameraManager::resetProcessors()
{
	backDataProcessor_.resetProcessor();
	frontDataProcessor_.resetProcessor();
}
