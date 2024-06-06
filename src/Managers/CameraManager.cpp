#include "Managers/CameraManager.hpp"

void CameraManager::initRsCamera()
{
	rs2::context context;
	rs2::device_list deviceList = context.query_devices();

	rsCameras_.reserve(3);

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
		std::string serialNumber = rsCamera.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);

		auto it = paramsMap_.find(serialNumber);
		if (serialNumber == frontCameraSerialNumber_)
		{
			rsCameras_.push_back(
					std::make_shared<RsCameraLoader>(
							cameraCount_, FRONT_CAMERA, 640, 480, 30, Parameters(), serialNumber
					)
			);
			rsCameras_.back()->init();
			rsCameras_.back()->startPipe();
		}
		else if (it != paramsMap_.end())
		{
			rsCameras_.push_back(
					std::make_shared<RsCameraLoader>(
							cameraCount_, BACK_CAMERA, 640, 480, 30, it->second, serialNumber
					)
			);
			rsCameras_.back()->init();
			rsCameras_.back()->startPipe();
		}
		else
		{
			LOGGER(Logger::WARNING, std::format("Detected unregistered realsense camera {}", serialNumber), true);
			continue;
		}

//		std::vector<rs2::sensor> sensors = rsCamera.query_sensors();
//		rs2::color_sensor colorSensor = rs2::color_sensor(sensors[1]);
//		colorSensor.set_option(RS2_OPTION_HUE, 10);

		LOGGER(Logger::INFO, std::format("Realsense camera {}({}) connected", serialNumber, cameraCount_), true);
		cameraCount_++;
	}
}

void CameraManager::startUpdateThread()
{
	for (auto &rsCamera: rsCameras_)
	{
		std::thread thread(&RsCameraLoader::updateFrame, rsCamera);
		thread.detach();
	}
}

void CameraManager::getCameraImage(std::vector<CameraImage> &cameraImages)
{
	int status;
	std::string info;
	cv::Mat colorImage;

	long currentTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	for (auto &rsCamera: rsCameras_)
	{
		status = rsCamera->getCurrentFrame(currentTimeStamp, colorImage);
		if (status == SUCCESS)
		{
			cameraImages.emplace_back(rsCamera->cameraId_, rsCamera->cameraType_, colorImage);
		}
	}
}

void CameraManager::stopUpdateThread()
{
	for (auto &rsCamera: rsCameras_)
	{
		rsCamera->stopPipe();
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));
}
