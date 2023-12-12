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
	FrontDataProcessor frontDataProcessor;

#if defined(WITH_CUDA)
	TrtEngineLoader trtEngineLoader = TrtEngineLoader("best-7cls-fp32.engine", 0.4, 0.4, 0.4);
#elif defined(WITH_OPENVINO)
	OvEngineLoader ovEngineLoader = OvEngineLoader("best-7cls.xml", "CPU", 0.4, 0.4, 0.4);
#endif

	RsCameraGroup rsCameraGroup;
//	WideFieldCameraGroup wideFieldCameraGroup;

	rsCameraGroup.detectRsCamera();
	rsCameraGroup.groupInit();
//	wideFieldCameraGroup.detectWideFieldCamera();
//	wideFieldCameraGroup.groupInit();

	while (true)
	{
		rsCameraGroup.groupGetImg();
#if defined(WITH_CUDA)
		rsCameraGroup.groupInfer(trtEngineLoader, backDataProcessor);
#elif defined(WITH_OPENVINO)
		rsCameraGroup.groupInfer(ovEngineLoader, backDataProcessor);
#endif

		//to be deprecated
		frontDataProcessor.detectedBalls_ = backDataProcessor.detectedBalls_;
		frontDataProcessor.pickedBallsIndex_ = backDataProcessor.pickedBallsIndex_;

		rsCameraGroup.groupDataProcess(backDataProcessor);
		backDataProcessor.outputPosition(dataSender);
		rsCameraGroup.groupDrawBoxes(backDataProcessor);
		backDataProcessor.clearBallVectors();

//		wideFieldCameraGroup.groupGetImg();
//		wideFieldCameraGroup.groupInfer(trtEngineLoader, frontDataProcessor);
		frontDataProcessor.frontDataProcess();
		frontDataProcessor.outputPosition(dataSender);
//		wideFieldCameraGroup.groupDrawBoxes(frontDataProcessor);
		frontDataProcessor.clearBallVectors();

//		dataSender.sendData();

		if (waitKey(1) == 27)
		{
			break;
		}
	}

	destroyAllWindows();
	return 0;
}
