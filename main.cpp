#include "Util/DataSender.hpp"
#include "Processor/BackDataProcessor.hpp"
#include "Processor/FrontDataProcessor.hpp"
#include "EngineLoader/IEngineLoader.hpp"
#include "EngineLoader/TrtEngineLoader.hpp"
#include "EngineLoader/OvEngineLoader.hpp"
#include "CameraManager/RsCameraGroup.hpp"
#include "CameraManager/WideFieldCameraGroup.hpp"

int main()
{
	std::ios::sync_with_stdio(false);
	std::cout.tie(nullptr);

	DataSender dataSender = DataSender(0);
	BackDataProcessor backDataProcessor;
//	FrontDataProcessor frontDataProcessor;

#if defined(WITH_CUDA)
	TrtEngineLoader engineLoader = TrtEngineLoader("yolov8s-best.engine", 0.5, 0.4);
#elif defined(WITH_OPENVINO)
	OvEngineLoader engineLoader = OvEngineLoader("yolov8s-best.xml", "CPU", 0.5, 0.4);
#endif
	RsCameraGroup rsCameraGroup;
//	WideFieldCameraGroup wideFieldCameraGroup;

	rsCameraGroup.detectRsCamera();
	rsCameraGroup.groupInit();
//	wideFieldCameraGroup.detectWideFieldCamera();
//	wideFieldCameraGroup.groupInit();

	while (true)
	{
		//back
		rsCameraGroup.groupDetect(engineLoader, backDataProcessor);
		rsCameraGroup.groupDataProcess(backDataProcessor);
		backDataProcessor.outputPosition(dataSender);
#if defined(GRAPHIC_DEBUG)
		rsCameraGroup.groupDrawBoxes(backDataProcessor);
#endif
		backDataProcessor.resetProcessor();

		//front
//		wideFieldCameraGroup.groupDetect(engineLoader, frontDataProcessor);
//		frontDataProcessor.outputPosition(dataSender);
//#if defined(GRAPHIC_DEBUG)
//		wideFieldCameraGroup.groupDrawBoxes(frontDataProcessor);
//#endif
//		frontDataProcessor.resetProcessor();

		//serial
#if defined(WITH_SERIAL)
		dataSender.sendData();
#endif

		if (waitKey(1) == 27)
		{
			break;
		}
	}

	destroyAllWindows();
	return 0;
}
