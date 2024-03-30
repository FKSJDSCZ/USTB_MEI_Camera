#include "Processor/BackDataProcessor.hpp"

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
		Ball &originalBall = backDataProcessor.detectedBalls_.at(ballsIndex_.back());
		double gradient = Functions::calcGradient(originalBall, targetBall);
		if (ballsIndex_.size() == 1)
		{
			if (std::abs(gradient) < 1)
			{
				gradient_ = gradient;
				ballsIndex_.push_back(index);
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
				ballsIndex_.push_back(index);
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

bool BackDataProcessor::InlinedBalls::checkDistance(BackDataProcessor &backDataProcessor)
{
	if (ballsIndex_.size() >= 2)
	{
		sortLinedBalls(backDataProcessor);

		auto nextIt = ballsIndex_.begin();
		bool isContinue = false;
		while (true)
		{
			auto it = nextIt++;
			if (nextIt == ballsIndex_.end())
			{
				break;
			}
			Ball &ball1 = backDataProcessor.detectedBalls_.at(*(it));
			Ball &ball2 = backDataProcessor.detectedBalls_.at(*(nextIt));
			Point2f p1 = Point2f(ball1.centerX_, ball1.centerY_);
			Point2f p2 = Point2f(ball2.centerX_, ball2.centerY_);
			if (Functions::calcDistance2f(p1, p2) <= 2.3f * ball1.width / 2)
			{
				isContinue = true;
			}
			else
			{
				if (isContinue)
				{
					while (nextIt != ballsIndex_.end())
					{
						ballsIndex_.erase(nextIt);
					}
					break;
				}
				else
				{
					ballsIndex_.erase(it);
					nextIt--;
				}
			}
		}
		return !ballsIndex_.empty();
	}
	return true;
}

void BackDataProcessor::InlinedBalls::positionRevise(BackDataProcessor &backDataProcessor, RsCameraLoader *rsCameraArray)
{
	std::vector<int> standardBallIndex;
	std::vector<int> wrongBallIndex;
	std::map<int, int> order;
	for (int i = 0; i < ballsIndex_.size(); ++i)
	{
		order.insert(std::make_pair(ballsIndex_.at(i), i));
	}

	for (int index: ballsIndex_)
	{
		Ball &tempBall = backDataProcessor.detectedBalls_.at(index);
		if (rsCameraArray[tempBall.cameraId_].parameters_.offsetPoint == tempBall.cameraPosition_)
		{
			wrongBallIndex.push_back(index);
		}
		else
		{
			standardBallIndex.push_back(index);
		}
	}
	if (standardBallIndex.size() > 1 && standardBallIndex.size() < 4)
	{
		Ball &firstBall = backDataProcessor.detectedBalls_.at(ballsIndex_.front());
		Ball &lastBall = backDataProcessor.detectedBalls_.at(ballsIndex_.back());
		int length = order[ballsIndex_.back()] - order[ballsIndex_.front()];
		float differenceX = (lastBall.cameraPosition_.x - firstBall.cameraPosition_.x) / length;
		float differenceZ = (lastBall.cameraPosition_.z - firstBall.cameraPosition_.z) / length;

		for (int wrongIndex: wrongBallIndex)
		{
			auto it = std::upper_bound(standardBallIndex.begin(), standardBallIndex.end(), order[wrongIndex],
			                           [&order](int standardIndex_, int val_) -> bool {
				                           return order[standardIndex_] < val_;
			                           });
			if (it == standardBallIndex.end())
			{
				int length_ = order[wrongIndex] - order[standardBallIndex.back()];
				Point3f &wrongBallPosition = backDataProcessor.detectedBalls_.at(wrongIndex).cameraPosition_;
				Point3f &standardBallPosition = backDataProcessor.detectedBalls_.at(standardBallIndex.back()).cameraPosition_;
				wrongBallPosition = Point3f(standardBallPosition.x + differenceX * length_, standardBallPosition.y,
				                            standardBallPosition.z + differenceZ * length_);
			}
			else
			{
				int length_ = order[*(it)] - order[wrongIndex];
				Point3f &wrongBallPosition = backDataProcessor.detectedBalls_.at(wrongIndex).cameraPosition_;
				Point3f &standardBallPosition = backDataProcessor.detectedBalls_.at(*(it)).cameraPosition_;
				wrongBallPosition = Point3f(standardBallPosition.x - differenceX * length_, standardBallPosition.y,
				                            standardBallPosition.z - differenceZ * length_);
			}
		}
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
	//参数计算
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

	//计算每排球的几何中心
	for (InlinedBalls &inlinedBalls: inlinedBallsGroup_)
	{
		inlinedBalls.calcBallsCenter(*this);
	}

	//自下向上排序
	std::sort(inlinedBallsGroup_.begin(), inlinedBallsGroup_.end(), [](InlinedBalls &line1, InlinedBalls &line2) -> bool {
		return line1.ballsCenterY_ > line2.ballsCenterY_;
	});

	//整球识别结果处理
	for (auto it = inlinedBallsGroup_.begin(); it != inlinedBallsGroup_.end();)
	{
		if (it->checkDistance(*this))
		{
			it->positionRevise(*this, rsCameraArray);
			it++;
		}
	}

	//去除坐标无效的球。注：这一步不能放在positionRevise()之前
	for (auto it = pickedBallsIndex_.begin(); it != pickedBallsIndex_.end();)
	{
		Ball &tempBall = detectedBalls_.at(*(it));
		if (rsCameraArray[tempBall.cameraId_].parameters_.offsetPoint == tempBall.cameraPosition_)
		{
			pickedBallsIndex_.erase(it);
		}
		else
		{
			it++;
		}
	}

	if (pickedBallsIndex_.empty())
	{
		detectMode_ = NO_BALL;
	}
	else if (inlinedBallsGroup_.empty() || inlinedBallsGroup_.front().ballsIndex_.size() < 4)
	{
		detectMode_ = SINGLE_BALL;

		std::sort(pickedBallsIndex_.begin(), pickedBallsIndex_.end(), [this](int index1, int index2) -> bool {
			Ball &ball1 = detectedBalls_.at(index1);
			Ball &ball2 = detectedBalls_.at(index2);
			if (ball1.labelNum_ == ball2.labelNum_)
			{
				return ball1.distance_ < ball2.distance_;
			}
			return ballPriority_[ball1.labelNum_] < ballPriority_[ball2.labelNum_];
		});

		if (!inlinedBallsGroup_.empty())
		{
			for (int index: inlinedBallsGroup_.front().ballsIndex_)
			{
				int labelNum = detectedBalls_.at(index).labelNum_;
				if (labelNum == 0 || labelNum == 1)
				{
					pickedBallsIndex_.insert(pickedBallsIndex_.begin(), index);
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
		Ball &tempBall = detectedBalls_.at(pickedBallsIndex_.at(0));
		data[1] = tempBall.cameraPosition_.x;
		data[2] = tempBall.cameraPosition_.y;
		data[3] = tempBall.cameraPosition_.z;
		data[4] = newLabelNum_[tempBall.labelNum_];
	}
	else if (detectMode_ == MULTIPLE_BALLS)
	{
		InlinedBalls &inlinedBalls = inlinedBallsGroup_.at(0);
		int size = std::min(4, static_cast<int>(inlinedBalls.ballsIndex_.size()));
		for (int i = 0; i < size; ++i)
		{
			Ball &tempBall = detectedBalls_.at(inlinedBalls.ballsIndex_.at(i));
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
//	for (int &index: pickedBallsIndex_)
//	{
//		Ball &tempBall=detectedBalls_.at(index);
//		Mat &img = rsCameraArray[tempBall.cameraId_].colorImg_;
//
//		rectangle(img, tempBall, RED, 2);
//		putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G")
//		             + " x: " + std::to_string(tempBall.cameraPosition_.x).substr(0, 6)
//		             + " y: " + std::to_string(tempBall.cameraPosition_.y).substr(0, 6)
//		             + " z: " + std::to_string(tempBall.cameraPosition_.z).substr(0, 6)
//				, Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
//	}

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