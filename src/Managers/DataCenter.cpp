#include "Managers/DataCenter.hpp"

void DataCenter::getBallData(IEngineLoader &engineLoader)
{
	for (CameraImage &cameraImage: cameraImages_)
	{
		engineLoader.getBallsByCameraId(
				cameraImage.cameraId_,
				cameraImage.cameraType_ & FRONT_CAMERA ? frontBalls_ : backBalls_
		);
	}
}

void DataCenter::processFrontData()
{
	std::sort(frontBalls_.begin(), frontBalls_.end(), [](Ball &ball1, Ball &ball2) -> bool {
		return ball1.graphCenter().x < ball2.graphCenter().x;
	});

	//选出框，删除紫球
	for (auto ballIt = frontBalls_.begin(); ballIt != frontBalls_.end();)
	{
		if (ballIt->labelNum_ == BASKET)
		{
			frontBaskets_.emplace_back(*(ballIt));
			frontBalls_.erase(ballIt);
		}
		else if (ballIt->labelNum_ == PURPLE_BALL)
		{
			frontBalls_.erase(ballIt);
		}
		else
		{
			ballIt++;
		}
	}

	//没有框，退出
	if (frontBaskets_.empty())
	{
		return;
	}

	//筛选框内球
	auto ballIt = frontBalls_.begin();
	for (Basket &basket: frontBaskets_)
	{
		//横向与纵向筛选
		cv::Rect2f filterRect = cv::Rect2f(basket.graphRect().x, basket.graphRect().y - basket.graphRect().height * 0.5f,
		                                   basket.graphRect().width, basket.graphRect().height * 1.5f);
		for (; ballIt != frontBalls_.end(); ++ballIt)
		{
			if (filterRect.contains(ballIt->graphCenter()))
			{
				basket.containedBalls_.push_back(*(ballIt));
			}
			else if (ballIt->graphCenter().x >= filterRect.x + filterRect.width)
			{
				break;
			}
		}
		//高度升序（y降序）排序
		std::sort(basket.containedBalls_.begin(), basket.containedBalls_.end(), [](Ball &ball1, Ball &ball2) -> bool {
			return ball1.graphCenter().y > ball2.graphCenter().y;
		});
	}
}

void DataCenter::processBackData(std::vector<std::shared_ptr<ICameraLoader>> &cameras)
{
	for (Ball &tempBall: backBalls_)
	{
		tempBall.setCameraPosition(cameras);
		tempBall.toMillimeter();
		tempBall.offsetToEncodingDisk(cameras);
		tempBall.calcDistance();
	}

	//删除框内球、框和坐标无效的球
	for (auto ballIt = backBalls_.begin(); ballIt != backBalls_.end();)
	{
		if (ballIt->isInBasket_ || ballIt->labelNum_ == BASKET || !ballIt->isValid_)
		{
			backBalls_.erase(ballIt);
		}
		else
		{
			ballIt++;
		}
	}

	//按距离排序
	std::sort(backBalls_.begin(), backBalls_.end(), [this](Ball &ball1, Ball &ball2) -> bool {
		if (ball1.labelNum_ == ball2.labelNum_)
		{
			return ball1.distance_ < ball2.distance_;
		}
		return ballPriority_[ball1.labelNum_] < ballPriority_[ball2.labelNum_];
	});

	//排序后去重
	if (!backBalls_.empty())
	{
		auto lastIt = backBalls_.begin();
		auto firstIt = lastIt++;
		while (lastIt != backBalls_.end())
		{
			if (Functions::calcDistanceXZ(firstIt->cameraPosition(), lastIt->cameraPosition()) < RADIUS)
			{
				firstIt->merge(*lastIt);
				backBalls_.erase(lastIt);
			}
			else
			{
				firstIt++;
				lastIt++;
			}
		}
	}

	//判断前进路线上是否有球
	if (!backBalls_.empty())
	{
		cv::Point3f firstBallPosition = backBalls_.front().cameraPosition();
		float leftLimit = std::min(-ROBOT_WIDTH_LIMIT, firstBallPosition.x) - 2 * RADIUS;
		float rightLimit = std::max(ROBOT_WIDTH_LIMIT, firstBallPosition.x) + 2 * RADIUS;
		float frontLimit = firstBallPosition.z - RADIUS;
		for (Ball &tempBall: backBalls_)
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

void DataCenter::setSenderBuffer(DataSender &dataSender)
{
	int backData[8] = {0};
	if (!backBalls_.empty())
	{
		cv::Point3i cameraPosition = backBalls_.front().cameraPosition();

		backData[0] = cameraPosition.x;
		backData[1] = cameraPosition.y;
		backData[2] = cameraPosition.z;
		backData[3] = backLabel_[backBalls_.front().labelNum_];
	}
	if (backBalls_.size() >= 2)
	{
		Ball &tempBall = backBalls_.at(1);
		if (tempBall.labelNum_ == RED_BALL || tempBall.labelNum_ == BLUE_BALL)
		{
			backData[4] = tempBall.cameraPosition().x;
			backData[5] = tempBall.cameraPosition().y;
			backData[6] = tempBall.cameraPosition().z;
			backData[7] = backLabel_[tempBall.labelNum_];
		}
	}

	int frontData[15];
	std::fill(frontData, frontData + 15, 3);
	if (frontBaskets_.size() == 5)
	{
		for (int i = 0; i < 5; ++i)
		{
			int j = 0;
			int ballCount = std::min(3, static_cast<int>(frontBaskets_.at(i).containedBalls_.size()));
			for (; j < ballCount; ++j)
			{
				frontData[i + j * 5] = frontLabel_[frontBaskets_.at(i).containedBalls_.at(j).labelNum_];
			}
			for (; j < 3; ++j)
			{
				frontData[i + j * 5] = 0;
			}
		}
	}

	dataSender.writeToBuffer(1, 1, (int *) &haveBallInFront_);
	dataSender.writeToBuffer(2, 8, backData);
	dataSender.writeToBuffer(10, 15, frontData);
}

void DataCenter::drawFrontImage()
{
	for (CameraImage &cameraImage: cameraImages_)
	{
		if (cameraImage.cameraType_ & FRONT_CAMERA)
		{
			cv::Mat &img = cameraImage.colorImage_;

			for (Ball &tempBall: frontBalls_)
			{
				rectangle(img, tempBall.graphRect(), RED, 2);
				putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G"),
				        cv::Point(tempBall.graphRect().x, tempBall.graphRect().y),
				        cv::FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
			}

			for (Basket &basket: frontBaskets_)
			{
				rectangle(img, basket.graphRect(), GREEN, 2);
				putText(img, std::to_string(basket.labelNum_), cv::Point(basket.graphRect().x, basket.graphRect().y),
				        cv::FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
				for (Ball &tempBall: basket.containedBalls_)
				{
					rectangle(img, tempBall.graphRect(), GREEN, 2);
					putText(img, std::to_string(tempBall.labelNum_), cv::Point(tempBall.graphRect().x, tempBall.graphRect().y),
					        cv::FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
				}
			}

			break;
		}
	}
}

void DataCenter::drawBackImage()
{
	cv::Mat *images[3] = {nullptr};
	for (CameraImage &cameraImage: cameraImages_)
	{
		images[cameraImage.cameraId_] = &cameraImage.colorImage_;
	}

	for (int i = 0; i < backBalls_.size(); ++i)
	{
		Ball &tempBall = backBalls_.at(i);
		for (const BallPosition &ballPosition: tempBall.ballPositions_)
		{
			if (images[ballPosition.cameraId_])
			{
				cv::Mat &img = *images[ballPosition.cameraId_];
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
	}
	if (!backBalls_.empty())
	{
		for (const BallPosition &ballPosition: backBalls_.front().ballPositions_)
		{
			if (images[ballPosition.cameraId_])
			{
				cv::Mat &img = *images[ballPosition.cameraId_];
				rectangle(img, ballPosition.graphRect_, GREEN, 2);
			}
		}
	}
	if (backBalls_.size() >= 2)
	{
		for (const BallPosition &ballPosition: backBalls_.at(1).ballPositions_)
		{
			if (images[ballPosition.cameraId_])
			{
				cv::Mat &img = *images[ballPosition.cameraId_];
				rectangle(img, ballPosition.graphRect_, WHITE, 2);
			}
		}
	}
}

void DataCenter::clearAll()
{
	cameraImages_.clear();
	frontBalls_.clear();
	frontBaskets_.clear();
	backBalls_.clear();
	haveBallInFront_ = false;
}
