#include "BackDataProcessor.hpp"

BackDataProcessor::InlinedBalls::InlinedBalls(int index)
{
	ballsIndex_.push_back(index);
}

int BackDataProcessor::InlinedBalls::appendBall(int index, BackDataProcessor &backDataProcessor)
{
	if (ballsIndex_.size() < 4)
	{
		//斜率判定
		Ball &targetBall = backDataProcessor.detectedBalls_.at(index);
		float minDistance = 1 << 12;
		int minIndex = 0;

		for (int i: ballsIndex_)
		{
			Ball &tempBall = backDataProcessor.detectedBalls_.at(i);
			float tempDistance =
					Functions::calcDistance2f(Point2f(targetBall.centerX_, targetBall.centerY_), Point2f(tempBall.centerX_, tempBall.centerY_));
			if (tempDistance < minDistance)
			{
				minIndex = i;
				minDistance = tempDistance;
			}
		}

		Ball &nearestBall = backDataProcessor.detectedBalls_.at(minIndex);
		int deltaX = static_cast<int>(targetBall.centerX_ - nearestBall.centerX_);
		int deltaY = static_cast<int>(targetBall.centerY_ - nearestBall.centerY_);
		double tangent;
		double degree;

		if (deltaX)
		{
			tangent = static_cast<double>(deltaY) / deltaX;
			degree = std::atan(tangent);
			if (!(degree > -CV_PI / 4 && degree < CV_PI / 4))
			{
				return FAILURE;
			}
		}
		else
		{
			return FAILURE;
		}

		//距离判断
		bool isBiggest = false;
		auto rightBallit = ballsIndex_.begin();
		while (rightBallit != ballsIndex_.end())
		{
			if (targetBall.centerX_ > backDataProcessor.detectedBalls_.at(*(rightBallit)).centerX_)
			{
				rightBallit++;
			}
			else
			{
				break;
			}
		}
		if (rightBallit == ballsIndex_.end())
		{
			isBiggest = true;
			rightBallit--;
		}
		Ball &rightBall = backDataProcessor.detectedBalls_.at(*(rightBallit));
		Point2f p2 = Point2f(targetBall.centerX_, targetBall.centerY_);
		Point2f p3 = Point2f(rightBall.centerX_, rightBall.centerY_);
		if (Functions::calcDistance2f(p2, p3) < 2.25f * nearestBall.width / 2)
		{
			if (isBiggest)
			{
				ballsIndex_.push_back(index);
			}
			else
			{
				ballsIndex_.insert(rightBallit, index);
			}
			return SUCCESS;
		}
		else
		{
			return FAILURE;
		}
	}
	else
	{
		return FAILURE;
	}
}

void BackDataProcessor::InlinedBalls::calcBallsCenter(BackDataProcessor &backDataProcessor)
{
	if (ballsIndex_.size() == 1)
	{
		ballsCenterX_ = backDataProcessor.detectedBalls_.at(0).centerX_;
		ballsCenterY_ = backDataProcessor.detectedBalls_.at(0).centerY_;
	}
	else
	{
		float ballsSumX = 0;
		float ballsSumY = 0;
		for (int index: ballsIndex_)
		{
			ballsSumX += backDataProcessor.detectedBalls_.at(index).centerX_;
			ballsSumY += backDataProcessor.detectedBalls_.at(index).centerY_;
		}
		ballsCenterX_ = ballsSumX / ballsIndex_.size();
		ballsCenterY_ = ballsSumY / ballsIndex_.size();
	}
}

void BackDataProcessor::InlinedBalls::sortLinedBalls(BackDataProcessor &backDataProcessor)
{
	std::sort(ballsIndex_.begin(), ballsIndex_.end(), [backDataProcessor](int index1, int index2) -> bool {
		return backDataProcessor.detectedBalls_.at(index1).centerX_ < backDataProcessor.detectedBalls_.at(index2).centerX_;
	});
}

void BackDataProcessor::backDataProcess(RsCameraLoader *rsCameraArray)
{
	for (int index: pickedBallsIndex_)
	{
		Ball &tempBall = detectedBalls_.at(index);
		rsCameraArray[tempBall.cameraId_].getCameraPosition(tempBall.centerX_, tempBall.centerY_, tempBall.cameraPosition_);
		tempBall.toMillimeter();//转毫米
		tempBall.offsetToEncodingDisk(rsCameraArray[tempBall.cameraId_].parameters_);//偏移到码盘
		tempBall.distance_ = Functions::calcDistance3f(tempBall.cameraPosition_, Point3f(0, 0, 0));
	}

	//删除框内球和框，并拷贝0号摄像头的球作为候选球
	for (auto it = pickedBallsIndex_.begin(); it != pickedBallsIndex_.end();)
	{
		Ball &tempBall = detectedBalls_.at(*(it));
		if (tempBall.isInBasket_ || tempBall.labelNum_ == 3)
		{
			pickedBallsIndex_.erase(it);
		}
		else if (tempBall.cameraId_ == 0)
		{
			candidateBalls_.push_back(*(it));
			it++;
		}
		else
		{
			it++;
		}
	}

	//准备发散球
	if (candidateBalls_.empty())
	{
		std::sort(pickedBallsIndex_.begin(), pickedBallsIndex_.end(), [this](int index1, int index2) -> bool {
			Ball &ball1 = detectedBalls_.at(index1);
			Ball &ball2 = detectedBalls_.at(index2);
			if (ball1.labelNum_ == ball2.labelNum_)
			{
				return ball1.distance_ < ball2.distance_;
			}
			return ballPriority_[ball1.labelNum_] < ballPriority_[ball2.labelNum_];
		});
	}

	//自下向上扫描
	std::sort(candidateBalls_.begin(), candidateBalls_.end(), [this](int index1, int index2) -> bool {
		return detectedBalls_.at(index1).centerY_ > detectedBalls_.at(index2).centerY_;
	});
	for (int index: candidateBalls_)
	{
		bool isAppended = false;
		for (InlinedBalls &inlinedBalls: inlinedBallsGroup_)
		{
			if (inlinedBalls.appendBall(index, *this) == SUCCESS)
			{
				isAppended = true;
				break;
			}
		}
		if (!isAppended)
		{
			inlinedBallsGroup_.emplace_back(index);
		}
	}

	//计算每排球的几何中心，每排球内部自左向右排序
	for (InlinedBalls &inlinedBalls: inlinedBallsGroup_)
	{
		inlinedBalls.calcBallsCenter(*this);
//		inlinedBalls.sortLinedBalls(*this);
	}

	//自下向上排序
	std::sort(inlinedBallsGroup_.begin(), inlinedBallsGroup_.end(), [](InlinedBalls &line1, InlinedBalls &line2) -> bool {
		return line1.ballsCenterY_ > line2.ballsCenterY_;
	});
}

//数据输出
void BackDataProcessor::outputPosition(DataSender &dataSender)
{
	int data[17] = {0};
	if (!pickedBallsIndex_.empty())
	{
		if (inlinedBallsGroup_.empty())
		{
			Ball &tempBall = detectedBalls_.at(pickedBallsIndex_.at(0));
			data[0] = SINGLE_BALL;
			data[1] = tempBall.cameraPosition_.x;
			data[2] = tempBall.cameraPosition_.y;
			data[3] = tempBall.cameraPosition_.z;
			data[4] = newLabelNum_[tempBall.labelNum_];
		}
		else
		{
			InlinedBalls &inlinedBalls = inlinedBallsGroup_.at(0);
			if (inlinedBalls.ballsIndex_.size() < 4)
			{
				Ball &tempBall = detectedBalls_.at(inlinedBalls.ballsIndex_.at(0));
				data[0] = SINGLE_BALL;
				data[1] = tempBall.cameraPosition_.x;
				data[2] = tempBall.cameraPosition_.y;
				data[3] = tempBall.cameraPosition_.z;
				data[4] = newLabelNum_[tempBall.labelNum_];
			}
			else
			{
				data[0] = MULTIPLE_BALLS;
				for (int i = 0; i < 4; ++i)
				{
					Ball &tempBall = detectedBalls_.at(inlinedBalls.ballsIndex_.at(i));
					data[4 * i + 1] = tempBall.cameraPosition_.x;
					data[4 * i + 2] = tempBall.cameraPosition_.y;
					data[4 * i + 3] = tempBall.cameraPosition_.z;
					data[4 * i + 4] = newLabelNum_[tempBall.labelNum_];
				}
			}
		}
	}
	dataSender.writeToBuffer(0, 17, data);
	std::cout << "[Info] Selected " << inlinedBallsGroup_.size() << " lines of objects. Detect mode: " << data[0] << std::endl;
}

//画图
void BackDataProcessor::drawBoxes(RsCameraLoader *rsCameraArray)
{
	for (int row = 0; row < inlinedBallsGroup_.size(); ++row)
	{
		for (int col = 0; col < inlinedBallsGroup_.at(row).ballsIndex_.size(); ++col)
		{
			Ball &tempBall = detectedBalls_.at(inlinedBallsGroup_.at(row).ballsIndex_.at(col));
			Mat &img = rsCameraArray[tempBall.cameraId_].colorImg_;

			rectangle(img, tempBall, RED, 2);
			putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G") + " R" + std::to_string(row) + " C"
			             + std::to_string(col)
//		+ " x: " + std::to_string(tempBall.cameraPosition_.x).substr(0, 6)
//		+ " y: " + std::to_string(tempBall.cameraPosition_.y).substr(0, 6)
//		+ " z: " + std::to_string(tempBall.cameraPosition_.z).substr(0, 6)
					, Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
		}
	}

	if (!inlinedBallsGroup_.empty())
	{
		InlinedBalls &inlinedBalls = inlinedBallsGroup_.at(0);
		for (int col = 0; col < inlinedBalls.ballsIndex_.size(); ++col)
		{
			Ball &tempBall = detectedBalls_.at(inlinedBalls.ballsIndex_.at(col));
			Mat &img = rsCameraArray[tempBall.cameraId_].colorImg_;

			rectangle(img, tempBall, GREEN, 2);
			putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G") + " R0 C" + std::to_string(col)
//		+ " x: " + std::to_string(tempBall.cameraPosition_.x).substr(0, 6)
//		+ " y: " + std::to_string(tempBall.cameraPosition_.y).substr(0, 6)
//		+ " z: " + std::to_string(tempBall.cameraPosition_.z).substr(0, 6)
					, Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
		}
	}
}

void BackDataProcessor::resetProcessor()
{
	detectedBalls_.clear();
	pickedBallsIndex_.clear();
	candidateBalls_.clear();
	inlinedBallsGroup_.clear();
}