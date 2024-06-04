#include "CameraManager/CameraManager.hpp"

void CameraManager::reconnect(std::shared_ptr<RsCameraLoader> &rsCamera)
{
	int attemptCount = 0;
	std::string info;

	info = std::format("Thread started reconnecting realsense camera {}", rsCamera->serialNumber_);
	std::cout << "[Info] " << info << std::endl;
	LOGGER(Logger::INFO, info);

	std::this_thread::sleep_for(std::chrono::seconds(3));

	while (true)
	{
		rs2::context context;
		bool isAttached = false;
		rs2::device_list deviceList = context.query_devices();
		for (auto &&camera: deviceList)
		{
			if (camera.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) == rsCamera->serialNumber_)
			{
				isAttached = true;
				break;
			}
		}

		attemptCount++;
		if (isAttached)
		{
			info = std::format("Attempt {}: Realsense camera {} attached", attemptCount, rsCamera->serialNumber_);
			std::cout << "[Info] " << info << std::endl;
			LOGGER(Logger::INFO, info);
			break;
		}
		else
		{
			info = std::format("Attempt {}: Realsense camera {} not attached", attemptCount, rsCamera->serialNumber_);
			std::cout << "[Warning] " << info << std::endl;
			LOGGER(Logger::WARNING, info);

			if (attemptCount == MAX_RECONNECT_ATTEMPTS_COUNT)
			{
				return;
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}

	rsCamera->resetPipe();
	info = std::format("Realsense camera {} reconnected", rsCamera->serialNumber_);
	std::cout << "[Info] " << info << std::endl;
	LOGGER(Logger::INFO, info);
}

void CameraManager::getImageFromCameras(std::vector<std::shared_ptr<RsCameraLoader>> &rsCameras)
{
	int status;
	std::string info;
	for (auto rsCamera = rsCameras.begin(); rsCamera != rsCameras.end();)
	{
		status = (**rsCamera).getImage();
		if (status == SUCCESS)
		{
//			engineLoader.setInput((**rsCamera).colorImg_, (**rsCamera).cameraId_);
			rsCamera++;
		}
		else
		{
			rsCameras.erase(rsCamera);
			disConnectedCameras_.push_back(*rsCamera);
			if (status == EMPTY_FRAME)
			{
				info = std::format("Empty frame from Realsense camera {}", (**rsCamera).serialNumber_);
				std::cout << "[Warning] " << info << std::endl;
				LOGGER(Logger::WARNING, info);
			}
			else    //TIME_OUT
			{
				info = std::format("Realsense camera {} time out. Reconnect", (**rsCamera).serialNumber_);
				std::cout << "[Warning] " << info << std::endl;
				LOGGER(Logger::WARNING, info);

				{
					std::lock_guard<std::mutex> lock((**rsCamera).mutex_);
					(**rsCamera).isConnected_ = false;
				}
				std::thread reconnectThread(&CameraManager::reconnect, std::ref(*rsCamera));
				reconnectThread.detach();
			}
		}
	}
}

CameraManager::CameraManager() = default;

void CameraManager::initRsCamera()
{
	rs2::context context;
	rs2::device_list deviceList = context.query_devices();

	backCameras_.reserve(2);
	frontCameras_.reserve(1);
	disConnectedCameras_.reserve(3);

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
			frontCameras_.push_back(
					std::make_shared<RsCameraLoader>(
							cameraCount_, RsCameraLoader::FRONT_CAMERA, 640, 480, 30, Parameters(), serialNumber
					)
			);
			frontCameras_.back()->init();
			frontCameras_.back()->startPipe();
		}
		else if (it != paramsMap_.end())
		{
			backCameras_.push_back(
					std::make_shared<RsCameraLoader>(
							cameraCount_, RsCameraLoader::BACK_CAMERA, 640, 480, 30, it->second, serialNumber
					)
			);
			backCameras_.back()->init();
			backCameras_.back()->startPipe();
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

void CameraManager::checkCameraStatus()
{
	for (auto rsCamera = disConnectedCameras_.begin(); rsCamera != disConnectedCameras_.end();)
	{
		bool isConnected;
		{
			std::lock_guard<std::mutex> lock((**rsCamera).mutex_);
			isConnected = (**rsCamera).isConnected_;
		}
		if (isConnected)
		{
			disConnectedCameras_.erase(rsCamera);
			((**rsCamera).cameraType_ == RsCameraLoader::FRONT_CAMERA ? frontCameras_ : backCameras_).push_back(*rsCamera);
		}
		else
		{
			rsCamera++;
		}
	}
}

void CameraManager::detect()
{
	getImageFromCameras(frontCameras_);
	getImageFromCameras(backCameras_);
//	engineLoader.preProcess();
//	engineLoader.infer();
//	engineLoader.postProcess();
//	for (auto &rsCamera: frontCameras_)
//	{
//		engineLoader.getBallsByCameraId(rsCamera->cameraId_, frontDataProcessor_.pickedBalls_);
//	}
//	for (auto &rsCamera: backCameras_)
//	{
//		engineLoader.getBallsByCameraId(rsCamera->cameraId_, backDataProcessor_.pickedBalls_);
//	}
//	if (!frontCameras_.empty())
//	{
//		frontDataProcessor_.dataProcess();
//	}
//	if (!backCameras_.empty())
//	{
//		backDataProcessor_.dataProcess(backCameras_);
//	}
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
	for (auto &rsCamera: frontCameras_)
	{
		imshow("Realsense " + std::to_string(rsCamera->cameraId_), rsCamera->colorImg_);
	}
	for (auto &rsCamera: backCameras_)
	{
		imshow("Realsense " + std::to_string(rsCamera->cameraId_), rsCamera->colorImg_);
	}
}

void CameraManager::saveVideos()
{
	for (auto &rsCamera: frontCameras_)
	{
		rsCamera->saveImage();
	}
	for (auto &rsCamera: backCameras_)
	{
		rsCamera->saveImage();
	}
}

void CameraManager::resetProcessors()
{
	backDataProcessor_.resetProcessor();
	frontDataProcessor_.resetProcessor();
}
