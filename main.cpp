#include <csignal>
#include "Loaders/TrtEngineLoader.hpp"
#include "Managers/DataCenter.hpp"
#include "Managers/CameraManager.hpp"
#include "Managers/VideoSaver.hpp"

std::atomic<int> interruptCount = 0;

void signalHandler(int signal)
{
	interruptCount++;
	LOGGER(Logger::WARNING, std::format("Received signal {}", signal), true);
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
//	cameraManager.initRsCamera();
	cameraManager.initWFCamera();

	DataCenter dataCenter;

	VideoSaver videoSaver;
	videoSaver.start(cameraManager.cameras_);

	TrtEngineLoader engineLoader = TrtEngineLoader("yolov8s-p2-dynamic-nms-0.5conf-best.engine", cameraManager.cameraCount_);
	engineLoader.setLetterboxParameters(cameraManager.cameras_);
	engineLoader.init();

	cameraManager.startUpdateThread();

	while (!interruptCount)
	{
		dataCenter.clearAll();

		cameraManager.getCameraImage(dataCenter.cameraImages_);
		engineLoader.setInput(dataCenter.cameraImages_);
		engineLoader.preProcess();
		engineLoader.infer();
		engineLoader.postProcess();
		dataCenter.getBallData(engineLoader);
		dataCenter.processFrontData();
		dataCenter.processBackData(cameraManager.cameras_);
		dataCenter.setSenderBuffer(dataSender);
#if defined(WITH_SERIAL)
		dataSender.sendData();
#endif
		dataCenter.drawFrontImage();
		dataCenter.drawBackImage();
#if defined(GRAPHIC_DEBUG)
		videoSaver.show(dataCenter.cameraImages_);
#endif
//		videoSaver.write(dataCenter.cameraImages_);

#if defined(GRAPHIC_DEBUG)
		if (cv::waitKey(1) == 27)
		{
			break;
		}
#endif
	}
	std::cout << "Exiting. Please wait a minute..." << std::endl;

#if defined(GRAPHIC_DEBUG)
	cv::destroyAllWindows();
#endif
	cameraManager.stopUpdateThread();
	videoSaver.finish();

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
		LOGGER(Logger::ERROR, e.what(), true);
	}
	LOGGER(Logger::INFO, "Program exiting", false);
	return ret;
}
