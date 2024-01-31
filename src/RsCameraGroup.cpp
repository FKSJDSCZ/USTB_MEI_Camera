#include "RsCameraGroup.hpp"

void RsCameraGroup::detectRsCamera()
{
	deviceList_ = context_.query_devices();
	if (!deviceList_.size())
	{
		throw std::runtime_error("[Error] No Realsense camera detected");
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
		rsCamera.hardware_reset();
		std::string serialNumber = rsCamera.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
		int cameraId = 0;
		if (serialNumbers_[cameraId] != serialNumber)
		{
			cameraId = 1;
		}
		rsCamerasArray_[cameraId] = RsCameraLoader(640, 480, 30, pitchAngleDegrees_[cameraId], yawAngleDegrees_[cameraId], parameters_[cameraId]);
		rsCamerasArray_[cameraId].init(serialNumber);
		enabled_[cameraId] = true;

		std::cout << "[Info] Realsense camera " << cameraId << " connected. Serial number: " << serialNumber << std::endl;
	}
}

void RsCameraGroup::groupGetImg()
{
	for (int i = 0; i < 2; ++i)
	{
		if (enabled_[i])
		{
			rsCamerasArray_[i].getImg();
		}
	}
}

#if defined(WITH_CUDA)
void RsCameraGroup::groupInfer(TrtEngineLoader &trtEngineLoader, BackDataProcessor &backDataProcessor)
{
	for (int i = 0; i < 2; ++i)
	{
		if (enabled_[i])
		{
			trtEngineLoader.imgProcess(rsCamerasArray_[i].colorImg_);
			trtEngineLoader.infer();
			trtEngineLoader.detectDataProcess(backDataProcessor.detectedBalls_, backDataProcessor.pickedBallsIndex_, i);
		}
	}
}
#elif defined(WITH_OPENVINO)

void RsCameraGroup::groupInfer(OvEngineLoader &ovEngineLoader, BackDataProcessor &backDataProcessor)
{
	for (int i = 0; i < 2; ++i)
	{
		if (enabled_[i])
		{
			ovEngineLoader.imgProcess(rsCamerasArray_[i].colorImg_);
			ovEngineLoader.infer();
			ovEngineLoader.detectDataProcess(backDataProcessor.detectedBalls_, backDataProcessor.pickedBallsIndex_, i);
		}
	}
}

#endif

void RsCameraGroup::groupDataProcess(BackDataProcessor &backDataProcessor)
{
	backDataProcessor.backDataProcess(rsCamerasArray_);
}

void RsCameraGroup::groupDrawBoxes(BackDataProcessor &backDataProcessor)
{
	backDataProcessor.drawBoxes(rsCamerasArray_);
	for (int i = 0; i < 2; ++i)
	{
		if (enabled_[i])
		{
			imshow("Realsense " + std::to_string(i), rsCamerasArray_[i].colorImg_);
		}
	}
}