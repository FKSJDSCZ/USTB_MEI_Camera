#include "CameraManager/CameraManager.hpp"

#include <ranges>

CameraManager::CameraManager() = default;

void CameraManager::initRsCamera()
{
	rs2::context context;
	rs2::device_list deviceList = context.query_devices();

	backCameras_.reserve(2);
	frontCameras_.reserve(1);

	if (!deviceList.size())
	{
		throw std::runtime_error("No Realsense camera detected");
	}
	else if (deviceList.size() < 3)
	{
		std::cerr << "[Warning] Detected " << deviceList.size() << " Realsense camera successfully. Please connect more cameras" << std::endl;
	}
	else
	{
		std::cout << "[Info] Detected 3 Realsense cameras successfully" << std::endl;
	}

	for (auto &&rsCamera: deviceList)
	{
//		rsCamera.hardware_reset();
		std::string serialNumber = rsCamera.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);

		auto it = paramsMap_.find(serialNumber);
		if (serialNumber == frontCameraSerialNumber_)
		{
			frontCameras_.emplace_back(cameraCount_, 640, 480, 30, it->second, serialNumber);
			frontCameras_.back().init();
		}
		else if (it != paramsMap_.end())
		{
			backCameras_.emplace_back(cameraCount_, 640, 480, 30, it->second, serialNumber);
			backCameras_.back().init();
		}
		else
		{
			std::cerr << "[Warning] Detected unregistered realsense camera " << serialNumber << std::endl;
			LOGGER(Logger::WARNING, std::format("Detected unregistered realsense camera {}", serialNumber));
			continue;
		}

//		std::vector<rs2::sensor> sensors = rsCamera.query_sensors();
//		rs2::color_sensor colorSensor = rs2::color_sensor(sensors[1]);
//		colorSensor.set_option(RS2_OPTION_HUE, 10);

		std::cout << "[Info] Realsense camera " << serialNumber << "(" << cameraCount_ << ") connected" << std::endl;
		LOGGER(Logger::INFO, std::format("Realsense camera {}({}) connected", serialNumber, cameraCount_));
		cameraCount_++;
	}
}

void CameraManager::detect(IEngineLoader &engineLoader)
{
	for (RsCameraLoader &rsCamera: frontCameras_)
	{
		rsCamera.getImage();
		if (rsCamera.colorImg_.empty())
		{
			LOGGER(Logger::WARNING,
			       std::format("Ignored empty image from Realsense camera {}", rsCamera.cameraId_));
		}
		else
		{
			engineLoader.setInput(rsCamera.colorImg_, rsCamera.cameraId_);
		}
	}
	for (RsCameraLoader &rsCamera: backCameras_)
	{
		rsCamera.getImage();
		if (rsCamera.colorImg_.empty())
		{
			LOGGER(Logger::WARNING,
			       std::format("Ignored empty image from Realsense camera {}", rsCamera.cameraId_));
		}
		else
		{
			engineLoader.setInput(rsCamera.colorImg_, rsCamera.cameraId_);
		}
	}
	engineLoader.preProcess();
	engineLoader.infer();
	engineLoader.postProcess();
	for (RsCameraLoader &rsCamera: frontCameras_)
	{
		engineLoader.getBallsByCameraId(rsCamera.cameraId_, frontDataProcessor_.pickedBalls_);
	}
	for (RsCameraLoader &rsCamera: backCameras_)
	{
		engineLoader.getBallsByCameraId(rsCamera.cameraId_, backDataProcessor_.pickedBalls_);
	}
	if (!frontCameras_.empty())
	{
		frontDataProcessor_.dataProcess();
	}
	if (!backCameras_.empty())
	{
		backDataProcessor_.dataProcess(backCameras_);
	}
}

void CameraManager::outputData(DataSender &dataSender)
{
	if (!frontCameras_.empty())
	{
		frontDataProcessor_.outputData(dataSender);
	}
	if (!backCameras_.empty())
	{
		backDataProcessor_.outputData(dataSender);
	}
}

void CameraManager::drawBoxes()
{
	if (!frontCameras_.empty())
	{
		frontDataProcessor_.drawBoxes(frontCameras_);
	}
	if (!backCameras_.empty())
	{
		backDataProcessor_.drawBoxes(backCameras_);
	}
}

void CameraManager::showImages()
{
	for (RsCameraLoader &rsCamera: frontCameras_)
	{
		imshow("Realsense " + std::to_string(rsCamera.cameraId_), rsCamera.colorImg_);
	}
	for (RsCameraLoader &rsCamera: backCameras_)
	{
		imshow("Realsense " + std::to_string(rsCamera.cameraId_), rsCamera.colorImg_);
	}
}

void CameraManager::saveVideos()
{
	for (RsCameraLoader &rsCamera: frontCameras_)
	{
		rsCamera.saveImage();
	}
	for (RsCameraLoader &rsCamera: backCameras_)
	{
		rsCamera.saveImage();
	}
}

void CameraManager::resetProcessors()
{
	backDataProcessor_.resetProcessor();
	frontDataProcessor_.resetProcessor();
}
