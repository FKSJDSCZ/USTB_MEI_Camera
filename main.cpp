#include "DataSender.hpp"
#include "BackDataProcessor.hpp"
#include "FrontDataProcessor.hpp"
#include "TrtEngineLoader.hpp"
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
	TrtEngineLoader trtEngineLoader = TrtEngineLoader("best-7cls-fp32.engine", 0.4, 0.4, 0.4);
	RsCameraGroup rsCameraGroup;
//	WideFieldCameraGroup wideFieldCameraGroup;

	rsCameraGroup.detectRsCamera();
	rsCameraGroup.groupInit();
//	wideFieldCameraGroup.detectWideFieldCamera();
//	wideFieldCameraGroup.groupInit();

	while (true)
	{
		rsCameraGroup.groupGetImg();
		rsCameraGroup.groupInfer(trtEngineLoader, backDataProcessor);

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
