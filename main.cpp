#include <csignal>
#include "Util/DataSender.hpp"
#include "Processor/BackDataProcessor.hpp"
#include "Processor/FrontDataProcessor.hpp"
#include "EngineLoader/IEngineLoader.hpp"
#include "EngineLoader/TrtEngineLoader.hpp"
#include "EngineLoader/OvEngineLoader.hpp"
#include "CameraManager/RsCameraGroup.hpp"
#include "CameraManager/WideFieldCameraGroup.hpp"

volatile bool interruptFlag = false;

void signalHandler(int signal)
{
	interruptFlag = true;
	std::string warning = std::format("Received signal {}", signal);
	std::cerr << warning << std::endl;
	Logger::getInstance().writeMsg(Logger::WARNING, warning);
}

int mainBody()
{
	std::ios::sync_with_stdio(false);
	std::cout.tie(nullptr);
	std::cout << getBuildInformation() << std::endl;

	DataSender dataSender = DataSender(0);

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

	while (!interruptFlag)
	{
		//back
		rsCameraGroup.groupDetect(engineLoader);
		rsCameraGroup.backDataProcessor_.outputPosition(dataSender);
		rsCameraGroup.groupDrawBoxes();
#if defined(GRAPHIC_DEBUG)
		rsCameraGroup.groupShowImages();
#endif
		rsCameraGroup.groupSaveVideos();
		rsCameraGroup.backDataProcessor_.resetProcessor();

		//front
//		wideFieldCameraGroup.groupDetect(engineLoader);
//		wideFieldCameraGroup.frontDataProcessor_.outputPosition(dataSender);
//		wideFieldCameraGroup.groupDrawBoxes();
//#if defined(GRAPHIC_DEBUG)
//		wideFieldCameraGroup.groupShowImages();
//#endif
//		wideFieldCameraGroup.groupSaveVideos();
//		wideFieldCameraGroup.frontDataProcessor_.resetProcessor();

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

int main()
{
	signal(SIGHUP, signalHandler);//1
	signal(SIGINT, signalHandler);//2
	signal(SIGQUIT, signalHandler);//3
	signal(SIGILL, signalHandler);
	signal(SIGTRAP, signalHandler);
	signal(SIGABRT, signalHandler);//6
	signal(SIGFPE, signalHandler);//8
	signal(SIGKILL, signalHandler);//9
	signal(SIGBUS, signalHandler);//10
	signal(SIGSEGV, signalHandler);//11
	signal(SIGSYS, signalHandler);
	signal(SIGPIPE, signalHandler);
	signal(SIGALRM, signalHandler);
	signal(SIGTERM, signalHandler);//15
	signal(SIGURG, signalHandler);
	signal(SIGSTOP, signalHandler);//17
	signal(SIGTSTP, signalHandler);
	signal(SIGCONT, signalHandler);
	signal(SIGCHLD, signalHandler);
	signal(SIGTTIN, signalHandler);
	signal(SIGTTOU, signalHandler);
	signal(SIGPOLL, signalHandler);
	signal(SIGXCPU, signalHandler);
	signal(SIGXFSZ, signalHandler);
	signal(SIGVTALRM, signalHandler);
	signal(SIGPROF, signalHandler);//27

	int ret;
	try
	{
		ret = mainBody();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		Logger::getInstance().writeMsg(Logger::ERROR, e.what());
	}
	Logger::getInstance().writeMsg(Logger::INFO, "Program exiting");
	return ret;
}
