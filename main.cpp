#include <csignal>
#include "Util/DataSender.hpp"
#include "EngineLoader/TrtEngineLoader.hpp"
#include "CameraManager/CameraManager.hpp"

int interruptCount = 0;

void signalHandler(int signal)
{
	std::string warning = std::format("Received signal {}", signal);
	std::cerr << warning << std::endl;
	LOGGER(Logger::WARNING, warning);
	interruptCount++;
	if (interruptCount >= MAX_INTERRUPT_COUNT)
	{
		exit(-1);
	}
}

int mainBody()
{
	std::ios::sync_with_stdio(false);
	std::cout.tie(nullptr);

	auto dataSender = DataSender(0);

	CameraManager cameraManager;
	cameraManager.initRsCamera();

#if defined(WITH_CUDA)
	TrtEngineLoader engineLoader = TrtEngineLoader("yolov8s-dynamic-best.engine", cameraManager.cameraCount_, 0.5, 0.4);
#elif defined(WITH_OPENVINO)
	OvEngineLoader engineLoader = OvEngineLoader("yolov8s-dynamic-best.xml", "yolov8s-dynamic-best.bin",
												 "CPU", cameraManager.cameraCount_, 0.5, 0.4);
#endif

	while (!interruptCount)
	{
		cameraManager.checkCameraStatus();
		cameraManager.detect();
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

		if (cv::waitKey(1) == 27)
		{
			break;
		}
	}

	cv::destroyAllWindows();

	std::cout << "Exiting. Please wait a minute..." << std::endl;
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
		std::cerr << "[Error] " << e.what() << std::endl;
		LOGGER(Logger::ERROR, e.what());
	}
	LOGGER(Logger::INFO, "Program exiting");
	return ret;
}
