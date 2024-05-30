#pragma once

#include "Util/Logger.hpp"

#define LOGGER(infoType, message) Logger::getInstance().writeMsg(infoType, message)

#define GREEN cv::Scalar(0, 255, 0)
#define WHITE cv::Scalar(255, 255, 255)
#define RED cv::Scalar(0, 0, 255)
#define ROBOT_WIDTH 550.0f
#define ROBOT_WIDTH_LIMIT 275.0f
#define RADIUS 95.0f
