#include "Processor/BackDataProcessor.hpp"

void BackDataProcessor::dataProcess(std::vector<std::shared_ptr<RsCameraLoader>> &rsCameras)
{
	for (Ball &tempBall: pickedBalls_)
	{
		tempBall.setCameraPosition(rsCameras);
		tempBall.toMillimeter();
		tempBall.offsetToEncodingDisk(rsCameras);
		tempBall.calcDistance();
	}

	//删除框内球、框和坐标无效的球
	for (auto ballIt = pickedBalls_.begin(); ballIt != pickedBalls_.end();)
	{
		if (ballIt->isInBasket_ || ballIt->labelNum_ == 3 || !ballIt->isValid_)
		{
			pickedBalls_.erase(ballIt);
		}
		else
		{
			ballIt++;
		}
	}

	//按距离排序
	std::sort(pickedBalls_.begin(), pickedBalls_.end(), [this](Ball &ball1, Ball &ball2) -> bool {
		if (ball1.labelNum_ == ball2.labelNum_)
		{
			return ball1.distance_ < ball2.distance_;
		}
		return ballPriority_[ball1.labelNum_] < ballPriority_[ball2.labelNum_];
	});

	//排序后去重
	if (!pickedBalls_.empty())
	{
		auto lastIt = pickedBalls_.begin();
		auto firstIt = lastIt++;
		while (lastIt != pickedBalls_.end())
		{
			if (Functions::calcDistance3f(firstIt->cameraPosition(), lastIt->cameraPosition()) < 1.5 * RADIUS)
			{
				firstIt->merge(*lastIt);
				pickedBalls_.erase(lastIt);
			}
			else
			{
				firstIt++;
				lastIt++;
			}
		}
	}

	//判断前进路线上是否有球
	if (!pickedBalls_.empty())
	{
		cv::Point3f firstBallPosition = pickedBalls_.front().cameraPosition();
		float leftLimit = std::min(-ROBOT_WIDTH_LIMIT, firstBallPosition.x) - 2 * RADIUS;
		float rightLimit = std::max(ROBOT_WIDTH_LIMIT, firstBallPosition.x) + 2 * RADIUS;
		float frontLimit = firstBallPosition.z - RADIUS;
		for (Ball &tempBall: pickedBalls_)
		{
			cv::Point3f cameraPosition = tempBall.cameraPosition();
			if (cameraPosition.x > leftLimit && cameraPosition.x < rightLimit && cameraPosition.z < frontLimit)
			{
				haveBallInFront_ = true;
				break;
			}
		}
	}
}

//数据输出
void BackDataProcessor::outputData(DataSender &dataSender)
{
	int data[8] = {0};
	if (!pickedBalls_.empty())
	{
		cv::Point3i cameraPosition = pickedBalls_.front().cameraPosition();

		data[0] = cameraPosition.x;
		data[1] = cameraPosition.y;
		data[2] = cameraPosition.z;
		data[3] = newLabelNum_[pickedBalls_.front().labelNum_];
	}
	if (pickedBalls_.size() >= 2)
	{
		Ball &tempBall = pickedBalls_.at(1);
		if (tempBall.labelNum_ == 0 || tempBall.labelNum_ == 1)
		{
			data[4] = tempBall.cameraPosition().x;
			data[5] = tempBall.cameraPosition().y;
			data[6] = tempBall.cameraPosition().z;
			data[7] = newLabelNum_[tempBall.labelNum_];
		}
	}
	dataSender.writeToBuffer(1, 1, (int *) &haveBallInFront_);
	dataSender.writeToBuffer(2, 8, data);
}

//画图
void BackDataProcessor::drawBoxes(std::vector<std::shared_ptr<RsCameraLoader>> &rsCameras)
{
	for (int i = 0; i < pickedBalls_.size(); ++i)
	{
		Ball &tempBall = pickedBalls_.at(i);
		for (const BallPosition &ballPosition: tempBall.ballPositions_)
		{
			cv::Mat &img = rsCameras.at(ballPosition.cameraId_)->colorImg_;

			rectangle(img, ballPosition.graphRect_, RED, 2);
			putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B " : " G ") + std::to_string(i),
			        cv::Point2i(ballPosition.graphRect_.x, ballPosition.graphRect_.y),
			        cv::FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
			putText(img, "x: " + std::to_string(ballPosition.cameraPosition_.x).substr(0, 6),
			        cv::Point2i(ballPosition.graphRect_.x, ballPosition.graphRect_.y + 12),
			        cv::FONT_HERSHEY_SIMPLEX, 0.4, GREEN, 1);
			putText(img, "y: " + std::to_string(ballPosition.cameraPosition_.y).substr(0, 6),
			        cv::Point2i(ballPosition.graphRect_.x, ballPosition.graphRect_.y + 24),
			        cv::FONT_HERSHEY_SIMPLEX, 0.4, GREEN, 1);
			putText(img, "z: " + std::to_string(ballPosition.cameraPosition_.z).substr(0, 6),
			        cv::Point2i(ballPosition.graphRect_.x, ballPosition.graphRect_.y + 36),
			        cv::FONT_HERSHEY_SIMPLEX, 0.4, GREEN, 1);
		}
	}
	if (!pickedBalls_.empty())
	{
		for (const BallPosition &ballPosition: pickedBalls_.front().ballPositions_)
		{
			cv::Mat &img = rsCameras.at(ballPosition.cameraId_)->colorImg_;
			rectangle(img, ballPosition.graphRect_, GREEN, 2);
		}
	}
	if (pickedBalls_.size() >= 2)
	{
		for (const BallPosition &ballPosition: pickedBalls_.at(1).ballPositions_)
		{
			cv::Mat &img = rsCameras.at(ballPosition.cameraId_)->colorImg_;
			rectangle(img, ballPosition.graphRect_, WHITE, 2);
		}
	}
}

void BackDataProcessor::resetProcessor()
{
	pickedBalls_.clear();
	haveBallInFront_ = false;
}