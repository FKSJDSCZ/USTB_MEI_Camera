#include "BackDataProcessor.hpp"

void BackDataProcessor::backDataProcess(RsCameraLoader *rsCameraArray)
{
	for (int index: pickedBallsIndex_)
	{
		Ball &tempBall = detectedBalls_.at(index);
		rsCameraArray[tempBall.cameraId_].getCameraPosition(tempBall.centerX_, tempBall.centerY_, tempBall.cameraPosition_);
		tempBall.toMillimeter();//转毫米
		tempBall.offsetToEncodingDisk(rsCameraArray[tempBall.cameraId_].parameters_);//偏移到码盘
		tempBall.distance_ = Functions::calcDistance(tempBall.cameraPosition_, Point3f(0, 0, 0));
	}

	//删除框内球和框，并拷贝0号摄像头的球
	for (auto it = pickedBallsIndex_.begin(); it != pickedBallsIndex_.end();)
	{
		Ball &tempBall = detectedBalls_.at(*(it));
		if (tempBall.isInBasket_ || tempBall.labelNum_ == 3)
		{
			pickedBallsIndex_.erase(it);
		}
		else if (tempBall.cameraId_ == 0)
		{
			matrixBallsIndex_.push_back(*(it));
			it++;
		}
		else
		{
			it++;
		}
	}

	//筛选成排的球
	std::sort(matrixBallsIndex_.begin(), matrixBallsIndex_.end(), [this](int index1, int index2) -> bool {
		return detectedBalls_.at(index1).centerY_ > detectedBalls_.at(index2).centerY_;
	});
	//直线判定
	for (int index: matrixBallsIndex_)
	{
		Ball &tempBall = detectedBalls_.at(index);
		if (selectedBallsIndex_.empty())
		{
			selectedBallsIndex_.push_back(index);
		}
		else
		{
			if (std::abs(tempBall.cameraPosition_.z - detectedBalls_.at(selectedBallsIndex_.back()).cameraPosition_.z) <= RADIUS / 2)
			{
				selectedBallsIndex_.push_back(index);
			}
			else
			{
				break;
			}
		}
		if (selectedBallsIndex_.size() == 4)
		{
			detectMode_ = MULTIPLE_BALLS;
			std::sort(selectedBallsIndex_.begin(), selectedBallsIndex_.end(), [this](int index1, int index2) -> bool {
				return detectedBalls_.at(index1).centerX_ < detectedBalls_.at(index2).centerX_;
			});
			for (int i = 0; i <= 2; ++i)
			{
				Ball &tempBall1 = detectedBalls_.at(selectedBallsIndex_.at(i));
				Ball &tempBall2 = detectedBalls_.at(selectedBallsIndex_.at(i + 1));
				if (tempBall2.cameraPosition_.x - tempBall1.cameraPosition_.x > 11 * RADIUS / 4)
				{
					detectMode_ = SINGLE_BALL;
					break;
				}
			}
		}
	}

	//对选择结果进行判断
	if (detectMode_ == SINGLE_BALL)
	{
		selectedBallsIndex_.clear();
		if (pickedBallsIndex_.empty())
		{
			detectMode_ = NO_BALL;
		}
		else
		{
			std::sort(pickedBallsIndex_.begin(), pickedBallsIndex_.end(), [this](int index1, int index2) -> bool {
				Ball &tempBall1 = detectedBalls_.at(index1);
				Ball &tempBall2 = detectedBalls_.at(index2);
				if (tempBall1.labelNum_ == tempBall2.labelNum_)
				{
					return detectedBalls_.at(index1).distance_ < detectedBalls_.at(index2).distance_;
				}
				return ballPriority_[tempBall1.labelNum_] < ballPriority_[tempBall2.labelNum_];
			});
			selectedBallsIndex_.push_back(pickedBallsIndex_.at(0));
		}
	}
}

//数据输出
void BackDataProcessor::outputPosition(DataSender &dataSender)
{
	int data[17] = {0};
	data[0] = detectMode_;
	if (detectMode_ == SINGLE_BALL)
	{
		if (!selectedBallsIndex_.empty())
		{
			Ball &tempBall = detectedBalls_.at(selectedBallsIndex_.at(0));
			data[1] = tempBall.cameraPosition_.x;
			data[2] = tempBall.cameraPosition_.y;
			data[3] = tempBall.cameraPosition_.z;
			data[4] = newLabelNum_[tempBall.labelNum_];
		}
	}
	else
	{
		for (int i = 0; i < std::min(4, static_cast<int>(selectedBallsIndex_.size())); ++i)
		{
			Ball &tempBall = detectedBalls_.at(selectedBallsIndex_.at(i));
			data[4 * i + 1] = tempBall.cameraPosition_.x;
			data[4 * i + 2] = tempBall.cameraPosition_.y;
			data[4 * i + 3] = tempBall.cameraPosition_.z;
			data[4 * i + 4] = newLabelNum_[tempBall.labelNum_];
		}
	}
	dataSender.writeToBuffer(0, 17, data);
	std::cout << "[Info] Selected " << selectedBallsIndex_.size() << " objects" << std::endl;
}

//画图
void BackDataProcessor::drawBoxes(RsCameraLoader *rsCameraArray)
{
	for (int index: pickedBallsIndex_)
	{
		Ball &tempBall = detectedBalls_.at(index);
		Mat &img = rsCameraArray[tempBall.cameraId_].colorImg_;

		rectangle(img, tempBall, RED, 2);
		putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G")
//		+ " x: " + std::to_string(tempBall.cameraPosition_.x).substr(0, 6)
//		+ " y: " + std::to_string(tempBall.cameraPosition_.y).substr(0, 6)
//		+ " z: " + std::to_string(tempBall.cameraPosition_.z).substr(0, 6)
				, Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, RED, 2);
	}

	for (int index: selectedBallsIndex_)
	{
		Ball &tempBall = detectedBalls_.at(index);
		Mat &img = rsCameraArray[tempBall.cameraId_].colorImg_;

		rectangle(img, tempBall, GREEN, 2);
		putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G")
//		+ " x: " + std::to_string(tempBall.cameraPosition_.x).substr(0, 6)
//		+ " y: " + std::to_string(tempBall.cameraPosition_.y).substr(0, 6)
//		+ " z: " + std::to_string(tempBall.cameraPosition_.z).substr(0, 6)
				, Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
	}
}

void BackDataProcessor::resetProcessor()
{
	detectedBalls_.clear();
	pickedBallsIndex_.clear();
	matrixBallsIndex_.clear();
	selectedBallsIndex_.clear();
	detectMode_ = SINGLE_BALL;
}