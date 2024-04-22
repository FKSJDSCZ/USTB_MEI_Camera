#include "CameraManager/RsCameraGroup.hpp"

void RsCameraGroup::detectRsCamera()
{
	deviceList_ = context_.query_devices();
	if (!deviceList_.size())
	{
		throw std::runtime_error("No Realsense camera detected");
	}
	else if (deviceList_.size() == 1)
	{
		std::cerr << "[Warning] Detected 1 Realsense camera successfully. Please connect more cameras" << std::endl;
	}
	else
	{
		std::cout << "[Info] Detected 2 Realsense cameras successfully" << std::endl;
	}
}

void RsCameraGroup::groupInit()
{
	for (auto &&rsCamera: deviceList_)
	{
//		rsCamera.hardware_reset();
		std::string serialNumber = rsCamera.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
		int cameraId = 0;
		if (serialNumbers_[cameraId] != serialNumber)
		{
			cameraId = 1;
		}
		rsCamerasArray_[cameraId] = RsCameraLoader(640, 480, 30, pitchAngleDegrees_[cameraId], yawAngleDegrees_[cameraId], parameters_[cameraId]);
		rsCamerasArray_[cameraId].init(serialNumber);
		enabled_[cameraId] = true;

//		std::vector<rs2::sensor> sensors = rsCamera.query_sensors();
//		rs2::color_sensor colorSensor = rs2::color_sensor(sensors[1]);
//		colorSensor.set_option(RS2_OPTION_HUE, 10);

		std::cout << "[Info] Realsense camera " << cameraId << " connected. Serial number: " << serialNumber << std::endl;
		Logger::getInstance().writeMsg(Logger::INFO, std::format("Realsense camera {}({}) connected", cameraId, serialNumber));
	}
}

void RsCameraGroup::groupDetect(IEngineLoader &engineLoader)
{
	for (int i = 0; i < 2; ++i)
	{
		if (enabled_[i])
		{
			rsCamerasArray_[i].getImage();
			if (rsCamerasArray_[i].colorImg_.empty())
			{
				Logger::getInstance().writeMsg(Logger::WARNING, std::format("Ignored empty image from Realsense camera {}", i));
				return;
			}
			engineLoader.detect(rsCamerasArray_[i].colorImg_, backDataProcessor_.pickedBalls_, i);
		}
	}
	backDataProcessor_.dataProcess(rsCamerasArray_);
}

void RsCameraGroup::groupDrawBoxes()
{
	backDataProcessor_.drawBoxes(rsCamerasArray_);

}

void RsCameraGroup::groupShowImages()
{
	for (int i = 0; i < 2; ++i)
	{
		if (enabled_[i])
		{
			imshow("Realsense " + std::to_string(i), rsCamerasArray_[i].colorImg_);
		}
	}
}

void RsCameraGroup::groupSaveVideos()
{
	for (int i = 0; i < 2; ++i)
	{
		if (enabled_[i])
		{
			rsCamerasArray_[i].saveImage();
		}
	}
}
