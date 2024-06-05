#include "Loaders/RsCameraLoader.hpp"

int RsCameraLoader::getFrameFromHardware(FrameData &frameData)
{
	if (pipe_.try_wait_for_frames(&frameData.frameset_, RS_FRAME_TIME_OUT))
	{
		frameData.timeStamp_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		frameData.frameset_ = alignToColor_.process(frameData.frameset_);
		return SUCCESS;
	}
	else
	{
		return TIME_OUT;
	}
}

int RsCameraLoader::reconnect() try
{
	int attemptCount = 0;
	std::string info;

	info = std::format("Started reconnecting realsense camera {}", serialNumber_);
	std::cout << "[Info] " << info << std::endl;
	LOGGER(Logger::INFO, info);

	while (true)
	{
		rs2::context context;
		bool isAttached = false;
		rs2::device_list deviceList = context.query_devices();
		for (auto &&camera: deviceList)
		{
			if (camera.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) == serialNumber_)
			{
				isAttached = true;
				break;
			}
		}
		attemptCount++;
		if (isAttached)
		{
			info = std::format("Attempt {}: Realsense camera {} attached", attemptCount, serialNumber_);
			std::cout << "[Info] " << info << std::endl;
			LOGGER(Logger::INFO, info);
			break;
		}
		else
		{
			info = std::format("Attempt {}: Realsense camera {} not attached", attemptCount, serialNumber_);
			std::cout << "[Warning] " << info << std::endl;
			LOGGER(Logger::WARNING, info);

			if (attemptCount == MAX_RECONNECT_ATTEMPTS_COUNT)
			{
				return FAILURE;
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	config_ = rs2::config();
	pipe_ = rs2::pipeline();
	init();
	startPipe();
	info = std::format("Realsense camera {} reconnected", serialNumber_);
	std::cout << "[Info] " << info << std::endl;
	LOGGER(Logger::INFO, info);
	return SUCCESS;
}
catch (std::exception &e)
{
	std::string info = std::format("Error reconnect realsense camera {}: {}", serialNumber_, e.what());
	std::cout << "[Error] " << info << std::endl;
	LOGGER(Logger::ERROR, info);
}

RsCameraLoader::RsCameraLoader(int cameraId, int cameraType, int imgWidth, int imgHeight, int framerate, Parameters parameters,
                               std::string serialNumber) :
		cameraId_(cameraId), cameraType_(cameraType), imgWidth_(imgWidth), imgHeight_(imgHeight), framerate_(framerate), parameters_(parameters),
		serialNumber_(std::move(serialNumber))
{
	pitchRotateMatrix_ = (cv::Mat_<float>(3, 3) <<
	                                            1, 0, 0,
			0, std::cos(parameters_.pitchAngle_ * CV_PI / 180), -std::sin(parameters_.pitchAngle_ * CV_PI / 180),
			0, std::sin(parameters_.pitchAngle_ * CV_PI / 180), std::cos(parameters_.pitchAngle_ * CV_PI / 180));

	yawRotateMatrix_ = (cv::Mat_<float>(3, 3) <<
	                                          std::cos(parameters_.yawAngle_ * CV_PI / 180), 0, std::sin(parameters_.yawAngle_ * CV_PI / 180),
			0, 1, 0,
			-std::sin(parameters_.yawAngle_ * CV_PI / 180), 0, std::cos(parameters_.yawAngle_ * CV_PI / 180));
}

void RsCameraLoader::init()
{
	config_.enable_device(serialNumber_);
	config_.enable_stream(RS2_STREAM_COLOR, imgWidth_, imgHeight_, RS2_FORMAT_BGR8, framerate_);
	config_.enable_stream(RS2_STREAM_DEPTH, imgWidth_, imgHeight_, RS2_FORMAT_Z16, framerate_);
}

int RsCameraLoader::startPipe()
{
	pipe_.start(config_);
	if (pipe_.try_wait_for_frames(&currentFrameSet_))
	{
		return SUCCESS;
	}
	else
	{
		return TIME_OUT;
	}
}

void RsCameraLoader::updateFrame()
{
	std::string info;
	FrameData frameData;

	while (isRunning_)
	{
		if (getFrameFromHardware(frameData) == SUCCESS)
		{
			std::lock_guard<std::mutex> lock(queueMutex_);
			frameQueue_.push(frameData);
			while (frameQueue_.size() > MAX_FRAME_QUEUE_SIZE)
			{
				frameQueue_.pop();
			}
		}
		else
		{
			info = std::format("Realsense camera {} disconnected", serialNumber_);
			std::cout << "[Warning] " << info << std::endl;
			LOGGER(Logger::WARNING, info);
			if (reconnect() == SUCCESS)
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}
	info = std::format("Sub-thread of realsense camera {} quited", serialNumber_);
	std::cout << "[Info] " << info << std::endl;
	LOGGER(Logger::INFO, info);
}

int RsCameraLoader::getCurrentFrame(long currentTimeStamp, cv::Mat &colorImage)
{
	std::lock_guard<std::mutex> lock(queueMutex_);
	if (frameQueue_.empty())
	{
		return NO_FRAME;
	}
	else
	{
		while (true)
		{
			FrameData frameData = frameQueue_.front();
			frameQueue_.pop();
			if (frameData.timeStamp_ >= currentTimeStamp || frameQueue_.empty())
			{
				currentFrameSet_ = frameData.frameset_;
				colorImage = cv::Mat({imgWidth_, imgHeight_}, CV_8UC3, (void *) frameData.frameset_.get_color_frame().get_data(), cv::Mat::AUTO_STEP);
				break;
			}
			else if (frameQueue_.front().timeStamp_ >= currentTimeStamp)
			{
				if (currentTimeStamp - frameData.timeStamp_ >= frameQueue_.front().timeStamp_ - currentTimeStamp)
				{
					currentFrameSet_ = frameQueue_.front().frameset_;
					colorImage = cv::Mat({imgWidth_, imgHeight_}, CV_8UC3,
					                     (void *) frameQueue_.front().frameset_.get_color_frame().get_data(), cv::Mat::AUTO_STEP);
				}
				else
				{
					currentFrameSet_ = frameData.frameset_;
					colorImage = cv::Mat({imgWidth_, imgHeight_}, CV_8UC3,
					                     (void *) frameData.frameset_.get_color_frame().get_data(), cv::Mat::AUTO_STEP);
				}
				break;
			}
		}
		return SUCCESS;
	}
}

cv::Point3f RsCameraLoader::getCameraPosition(const cv::Point2f &graphCenter)
{
	rs2::depth_frame depthFrame = currentFrameSet_.get_depth_frame();
	auto depthProfile = depthFrame.get_profile().as<rs2::video_stream_profile>();
	rs2_intrinsics internReference = depthProfile.get_intrinsics();

	//邻近采样防止深度黑洞
	float position[3];
	cv::Rect2i imgRect = cv::Rect2i(0, 0, imgWidth_, imgHeight_);
	for (auto &offset: pixelOffset_)
	{
		float point[2] = {graphCenter.x + offset[0], graphCenter.y + offset[1]};
		if (imgRect.contains(cv::Point2i(point[0], point[1])))
		{
			float depthValue = depthFrame.get_distance(point[0], point[1]);
			rs2_deproject_pixel_to_point(position, &internReference, point, depthValue);
			if (position[0] || position[1] || position[2])
			{
				break;
			}
		}
	}

	cv::Mat positionMatrix = (cv::Mat_<float>(3, 1) << position[0], position[1], position[2]);
	positionMatrix = yawRotateMatrix_ * pitchRotateMatrix_ * positionMatrix;
	return {positionMatrix.at<float>(0), positionMatrix.at<float>(1), positionMatrix.at<float>(2)};
}

void RsCameraLoader::stopPipe()
{
	isRunning_ = false;
//	pipe_.stop();
}
