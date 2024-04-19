#include "Processor/BackDataProcessor.hpp"

BackDataProcessor::InlinedBalls::InlinedBalls(Ball &ball)
{
	balls_.push_back(ball);
}

int BackDataProcessor::InlinedBalls::appendBall(Ball &targetBall)
{
	if (balls_.size() < 4)
	{
		//斜率判定
		double gradient = Functions::calcGradient(balls_.back(), targetBall);
		if (balls_.size() == 1)
		{
			if (std::abs(gradient) < 1)
			{
				gradient_ = gradient;
				balls_.push_back(targetBall);
				return SUCCESS;
			}
			else
			{
				return FAILURE;
			}
		}
		else
		{
			if (std::abs(gradient) < 1 && std::abs(gradient - gradient_) < 0.3)
			{
				balls_.push_back(targetBall);
				return SUCCESS;
			}
			else
			{
				return FAILURE;
			}
		}
	}
	else
	{
		return FAILURE;
	}
}

void BackDataProcessor::InlinedBalls::calcBallsCenter()
{
	if (balls_.size() == 1)
	{
		ballsCenterX_ = balls_.front().centerX_;
		ballsCenterY_ = balls_.front().centerY_;
	}
	else
	{
		float ballsSumX = 0;
		float ballsSumY = 0;
		for (Ball &tempBall: balls_)
		{
			ballsSumX += tempBall.centerX_;
			ballsSumY += tempBall.centerY_;
		}
		ballsCenterX_ = ballsSumX / balls_.size();
		ballsCenterY_ = ballsSumY / balls_.size();
	}
}

bool BackDataProcessor::InlinedBalls::checkDistance()
{
	if (balls_.size() >= 2)
	{
		sortLinedBalls();

		auto nextBallIt = balls_.begin();
		bool isContinue = false;
		while (true)
		{
			auto ballIt = nextBallIt++;
			if (nextBallIt == balls_.end())
			{
				break;
			}
			Point2f p1 = Point2f(ballIt->centerX_, ballIt->centerY_);
			Point2f p2 = Point2f(nextBallIt->centerX_, nextBallIt->centerY_);
			if (Functions::calcDistance2f(p1, p2) <= 2.3f * ballIt->width / 2)
			{
				isContinue = true;
			}
			else
			{
				if (isContinue)
				{
					while (nextBallIt != balls_.end())
					{
						balls_.erase(nextBallIt);
					}
					break;
				}
				else
				{
					balls_.erase(ballIt);
					nextBallIt--;
				}
			}
		}
		return !balls_.empty();
	}
	return true;
}

void BackDataProcessor::InlinedBalls::positionRevise(RsCameraLoader *rsCameraArray)
{
	std::vector<int> standardBallIndex;
	std::vector<int> wrongBallIndex;
	for (int i = 0; i < balls_.size(); ++i)
	{
		Ball &tempBall = balls_.at(i);
		if (rsCameraArray[tempBall.cameraId_].parameters_.offsetPoint == tempBall.cameraPosition_)
		{
			wrongBallIndex.push_back(i);
		}
		else
		{
			standardBallIndex.push_back(i);
		}
	}
	if (standardBallIndex.size() > 1 && standardBallIndex.size() < 4)
	{
		float differenceX = (balls_.back().cameraPosition_.x - balls_.front().cameraPosition_.x) / (balls_.size() - 1);
		float differenceZ = (balls_.back().cameraPosition_.z - balls_.front().cameraPosition_.z) / (balls_.size() - 1);

		for (int wrongIndex: wrongBallIndex)
		{
			auto it = std::upper_bound(standardBallIndex.begin(), standardBallIndex.end(), wrongIndex);
			if (it == standardBallIndex.end())
			{
				int length_ = wrongIndex - standardBallIndex.back();
				Point3f &standardBallPosition = balls_.at(standardBallIndex.back()).cameraPosition_;
				balls_.at(wrongIndex).cameraPosition_ = Point3f(standardBallPosition.x + differenceX * length_, standardBallPosition.y,
				                                                standardBallPosition.z + differenceZ * length_);
			}
			else
			{
				int length_ = *(it) - wrongIndex;
				Point3f &standardBallPosition = balls_.at(*(it)).cameraPosition_;
				balls_.at(wrongIndex).cameraPosition_ = Point3f(standardBallPosition.x - differenceX * length_, standardBallPosition.y,
				                                                standardBallPosition.z - differenceZ * length_);
			}
		}
	}
}

void BackDataProcessor::InlinedBalls::sortLinedBalls()
{
	std::sort(balls_.begin(), balls_.end(), [](Ball &ball1, Ball &ball2) -> bool {
		return ball1.centerX_ < ball2.centerX_;
	});
}

void BackDataProcessor::backDataProcess(RsCameraLoader *rsCameraArray)
{
	//参数计算
	for (Ball &tempBall: pickedBalls)
	{
		rsCameraArray[tempBall.cameraId_].getCameraPosition(tempBall.centerX_, tempBall.centerY_, tempBall.cameraPosition_);
		tempBall.toMillimeter();//转毫米
		tempBall.offsetToEncodingDisk(rsCameraArray[tempBall.cameraId_].parameters_);//偏移到码盘
		tempBall.distance_ = Functions::calcDistance3f(tempBall.cameraPosition_, Point3f(0, 0, 0));
	}

	//删除框内球和框，并拷贝0号摄像头的球作为候选球
	for (auto ballIt = pickedBalls.begin(); ballIt != pickedBalls.end();)
	{
		if (ballIt->isInBasket_ || ballIt->labelNum_ == 3)
		{
			pickedBalls.erase(ballIt);
		}
		else if (ballIt->cameraId_ == 0)
		{
			candidateBalls_.push_back(*(ballIt));
			ballIt++;
		}
		else
		{
			ballIt++;
		}
	}

	//自下向上扫描
	std::sort(candidateBalls_.begin(), candidateBalls_.end(), [](Ball &ball1, Ball &ball2) -> bool {
		return ball1.centerY_ > ball2.centerY_;
	});
	for (Ball &tempBall: candidateBalls_)
	{
		bool isAppended = false;
		for (InlinedBalls &inlinedBalls: inlinedBallsGroup_)
		{
			if (inlinedBalls.appendBall(tempBall) == SUCCESS)
			{
				isAppended = true;
				break;
			}
		}
		if (!isAppended)
		{
			inlinedBallsGroup_.emplace_back(tempBall);
		}
	}

	//计算每排球的几何中心
	for (InlinedBalls &inlinedBalls: inlinedBallsGroup_)
	{
		inlinedBalls.calcBallsCenter();
	}

	//自下向上排序
	std::sort(inlinedBallsGroup_.begin(), inlinedBallsGroup_.end(), [](InlinedBalls &line1, InlinedBalls &line2) -> bool {
		return line1.ballsCenterY_ > line2.ballsCenterY_;
	});

	//整球识别结果处理
	for (auto it = inlinedBallsGroup_.begin(); it != inlinedBallsGroup_.end();)
	{
		if (it->checkDistance())
		{
			it->positionRevise(rsCameraArray);
			it++;
		}
	}

	//去除坐标无效的球。注：这一步不能放在positionRevise()之前
	for (auto ballIt = pickedBalls.begin(); ballIt != pickedBalls.end();)
	{
		if (rsCameraArray[ballIt->cameraId_].parameters_.offsetPoint == ballIt->cameraPosition_)
		{
			pickedBalls.erase(ballIt);
		}
		else
		{
			ballIt++;
		}
	}

	if (pickedBalls.empty())
	{
		detectMode_ = NO_BALL;
	}
	else if (inlinedBallsGroup_.empty() || inlinedBallsGroup_.front().balls_.size() < 4)
	{
		detectMode_ = SINGLE_BALL;

		std::sort(pickedBalls.begin(), pickedBalls.end(), [this](Ball &ball1, Ball &ball2) -> bool {
			if (ball1.labelNum_ == ball2.labelNum_)
			{
				return ball1.distance_ < ball2.distance_;
			}
			return ballPriority_[ball1.labelNum_] < ballPriority_[ball2.labelNum_];
		});

		if (!inlinedBallsGroup_.empty())
		{
			for (auto ballIt = pickedBalls.begin(); ballIt != pickedBalls.end(); ++ballIt)
			{
				if (ballIt->labelNum_ == 0 || ballIt->labelNum_ == 1)
				{
					pickedBalls.insert(pickedBalls.begin(), *ballIt);
					pickedBalls.erase(ballIt);
					break;
				}
			}
		}
	}
	else
	{
		detectMode_ = MULTIPLE_BALLS;
	}
}

//数据输出
void BackDataProcessor::outputPosition(DataSender &dataSender)
{
	int data[17] = {0};
	data[0] = detectMode_;
	if (detectMode_ == SINGLE_BALL)
	{
		data[1] = pickedBalls.front().cameraPosition_.x;
		data[2] = pickedBalls.front().cameraPosition_.y;
		data[3] = pickedBalls.front().cameraPosition_.z;
		data[4] = newLabelNum_[pickedBalls.front().labelNum_];
	}
	else if (detectMode_ == MULTIPLE_BALLS)
	{
		InlinedBalls &inlinedBalls = inlinedBallsGroup_.at(0);
		int size = std::min(4, static_cast<int>(inlinedBalls.balls_.size()));
		for (int i = 0; i < size; ++i)
		{
			Ball &tempBall = inlinedBalls.balls_.at(i);
			data[4 * i + 1] = tempBall.cameraPosition_.x;
			data[4 * i + 2] = tempBall.cameraPosition_.y;
			data[4 * i + 3] = tempBall.cameraPosition_.z;
			data[4 * i + 4] = newLabelNum_[tempBall.labelNum_];
		}
	}
	dataSender.writeToBuffer(0, 17, data);
	std::cout << "[Info] Selected " << inlinedBallsGroup_.size() << " lines of objects. Detect mode: " << data[0] << std::endl;
}

//画图
void BackDataProcessor::drawBoxes(RsCameraLoader *rsCameraArray)
{
	for (Ball &tempBall: pickedBalls)
	{
		Mat &img = rsCameraArray[tempBall.cameraId_].colorImg_;

		rectangle(img, tempBall, RED, 2);
		putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G") + " x: "
		                                                                                       + std::to_string(tempBall.cameraPosition_.x)
				                                                                                       .substr(0, 6) + " y: "
		                                                                                       + std::to_string(tempBall.cameraPosition_.y)
				                                                                                       .substr(0, 6) + " z: "
		                                                                                       + std::to_string(tempBall.cameraPosition_.z)
				                                                                                       .substr(0, 6), Point(tempBall.x, tempBall.y),
		        FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
	}

	for (int row = 0; row < inlinedBallsGroup_.size(); ++row)
	{
		for (int col = 0; col < inlinedBallsGroup_.at(row).balls_.size(); ++col)
		{
			Ball &tempBall = inlinedBallsGroup_.at(row).balls_.at(col);
			Mat &img = rsCameraArray[tempBall.cameraId_].colorImg_;

			rectangle(img, tempBall, RED, 2);
			putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G") + " R" + std::to_string(row) + " C"
			             + std::to_string(col) + " x: " + std::to_string(tempBall.cameraPosition_.x).substr(0, 6) + " y: "
			                                                                                                      + std::to_string(
					                                                                                                      tempBall.cameraPosition_.y)
					                                                                                                      .substr(0, 6) + " z: "
			                                                                                                      + std::to_string(
					                                                                                                      tempBall.cameraPosition_.z)
					                                                                                                      .substr(0, 6),
			        Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
		}
	}

	if (!inlinedBallsGroup_.empty())
	{
		InlinedBalls &inlinedBalls = inlinedBallsGroup_.at(0);
		for (int col = 0; col < inlinedBalls.balls_.size(); ++col)
		{
			Ball &tempBall = inlinedBalls.balls_.at(col);
			Mat &img = rsCameraArray[tempBall.cameraId_].colorImg_;

			rectangle(img, tempBall, GREEN, 2);
			putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G") + " R0 C" + std::to_string(col)
			             + " x: " + std::to_string(tempBall.cameraPosition_.x).substr(0, 6)
			             + " y: " + std::to_string(tempBall.cameraPosition_.y).substr(0, 6)
			             + " z: " + std::to_string(tempBall.cameraPosition_.z).substr(0, 6), Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6,
			        GREEN, 2);
		}
	}
}

void BackDataProcessor::resetProcessor()
{
	pickedBalls.clear();
	candidateBalls_.clear();
	inlinedBallsGroup_.clear();
}