#include "DataSender.hpp"
#include "BackDataProcessor.hpp"
#include "FrontDataProcessor.hpp"

#if defined(WITH_CUDA)

#include "TrtEngineLoader.hpp"

#elif defined(WITH_OPENVINO)

#include "OvEngineLoader.hpp"

#endif

#include "RsCameraGroup.hpp"
#include "WideFieldCameraGroup.hpp"

int main()
{
	//standard output for debug only
	std::ios::sync_with_stdio(false);
	std::cout.tie(nullptr);

	DataSender dataSender = DataSender(0);
	BackDataProcessor backDataProcessor;
//	FrontDataProcessor frontDataProcessor;

#if defined(WITH_CUDA)
	TrtEngineLoader trtEngineLoader = TrtEngineLoader("best-7cls-fp32.engine", 0.4, 0.6, 0.4);
#elif defined(WITH_OPENVINO)
	OvEngineLoader ovEngineLoader = OvEngineLoader("best-7cls.xml", "CPU", 0.4, 0.6, 0.4);
#endif
	RsCameraGroup rsCameraGroup;
//	WideFieldCameraGroup wideFieldCameraGroup;

	rsCameraGroup.detectRsCamera();
	rsCameraGroup.groupInit();
//	wideFieldCameraGroup.detectWideFieldCamera();
//	wideFieldCameraGroup.groupInit();

	while (true)
	{
		//后场识别
		rsCameraGroup.groupGetImg();
#if defined(WITH_CUDA)
		rsCameraGroup.groupInfer(trtEngineLoader, backDataProcessor);
#elif defined(WITH_OPENVINO)
		rsCameraGroup.groupInfer(ovEngineLoader, backDataProcessor);
#endif
		rsCameraGroup.groupDataProcess(backDataProcessor);
		backDataProcessor.outputPosition(dataSender);
#if defined(GRAPHIC_DEBUG)
		rsCameraGroup.groupDrawBoxes(backDataProcessor);
#endif
		backDataProcessor.resetProcessor();

		//前场识别
//		wideFieldCameraGroup.groupGetImg();
//#if defined(WITH_CUDA)
//		wideFieldCameraGroup.groupInfer(trtEngineLoader, frontDataProcessor);
//#elif defined(WITH_OPENVINO)
//		wideFieldCameraGroup.groupInfer(ovEngineLoader, frontDataProcessor);
//#endif
//		frontDataProcessor.frontDataProcess();
//		frontDataProcessor.outputPosition(dataSender);
//#if defined(GRAPHIC_DEBUG)
//		wideFieldCameraGroup.groupDrawBoxes(frontDataProcessor);
//#endif
//		frontDataProcessor.resetProcessor();

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
