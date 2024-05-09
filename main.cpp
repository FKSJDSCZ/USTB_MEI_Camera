#include <csignal>
#include "Util/DataSender.hpp"
#include "Processor/BackDataProcessor.hpp"
#include "Processor/FrontDataProcessor.hpp"
#include "EngineLoader/IEngineLoader.hpp"
#include "EngineLoader/TrtEngineLoader.hpp"
#include "EngineLoader/OvEngineLoader.hpp"
#include "CameraManager/CameraManager.hpp"

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

	DataSender dataSender = DataSender(0);

	CameraManager cameraManager;
	cameraManager.initRsCamera();
//	cameraManager.initWFCamera();

#if defined(WITH_CUDA)
	TrtEngineLoader engineLoader = TrtEngineLoader("yolov8s-dynamic-best.engine",
	                                               cameraManager.cameraCount_, 0.5, 0.4);
#elif defined(WITH_OPENVINO)
	OvEngineLoader engineLoader = OvEngineLoader("yolov8s-dynamic-best.xml", "yolov8s-dynamic-best.bin",
												 "CPU", cameraManager.cameraCount_, 0.5, 0.4);
#endif

	while (!interruptFlag)
	{
		cameraManager.detect(engineLoader);
		cameraManager.outputData(dataSender);
#if defined(WITH_SERIAL)
		dataSender.sendData();
#endif
		cameraManager.drawBoxes();
#if defined(GRAPHIC_DEBUG)
		cameraManager.showImages();
#endif
		cameraManager.saveVideos();
		cameraManager.resetProcessors();

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
