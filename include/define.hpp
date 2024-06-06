#pragma once

#include "opencv2/opencv.hpp"
#include "librealsense2/rs.hpp"
#include "Util/Logger.hpp"

#define LOGGER(infoType, message, io) Logger::getInstance().writeMsg(infoType, message, io)

#define GREEN                                                        cv::Scalar(0, 255, 0)
#define WHITE                                                        cv::Scalar(255, 255, 255)
#define RED                                                            cv::Scalar(0, 0, 255)

#define ROBOT_WIDTH_LIMIT                                   275.0f
#define RADIUS                                                       95.0f

#define RS_FRAME_TIME_OUT                                   100
#define MAX_FRAME_QUEUE_SIZE                              5
#define MAX_RECONNECT_ATTEMPTS_COUNT              5
#define MAX_INTERRUPT_COUNT                                3

struct FrameData
{
	long timeStamp_{};
	rs2::frameset frameset_;
};

struct CameraImage
{
	int cameraId_;
	int cameraType_;
	cv::Mat colorImage_;
};

enum FlowControlFlag
{
	NO_FLOW_CONTROL = 0,
	HARDWARE_FLOW_CONTROL = 1,
	SOFTWARE_FLOW_CONTROL = 2,
};

enum ParityBitFlag
{
	NO_PARITY = 0,
	ODD_PARITY = 1,
	EVEN_PARITY = 2,
};

enum StopBitFlag
{
	ONE_STOP_BIT = 1,
	TWO_STOP_BIT = 2,
};

enum StatusCode
{
	FAILURE = -1,
	SUCCESS = 0,
	NO_FRAME = 1,
	TIME_OUT = 2,
};

enum CameraType
{
	FRONT_CAMERA = 0,
	BACK_CAMERA = 1,
};