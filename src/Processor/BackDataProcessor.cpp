#include "Processor/BackDataProcessor.hpp"

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

	//删除框内球、框和坐标无效的球
	for (auto it = pickedBallsIndex_.begin(); it != pickedBallsIndex_.end();)
	{
		Ball &tempBall = detectedBalls_.at(*(it));
		if (tempBall.isInBasket_ || tempBall.labelNum_ == 3 || rsCameraArray[tempBall.cameraId_].parameters_.offsetPoint == tempBall.cameraPosition_)
		{
			pickedBallsIndex_.erase(it);
		}
		else
		{
			it++;
		}
	}

	//按距离排序
	std::sort(pickedBallsIndex_.begin(), pickedBallsIndex_.end(), [this](int index1, int index2) -> bool {
		if (detectedBalls_.at(index1).labelNum_ == detectedBalls_.at(index2).labelNum_)
		{
			return detectedBalls_.at(index1).distance_ < detectedBalls_.at(index2).distance_;
		}
		return ballPriority_[detectedBalls_.at(index1).labelNum_] < ballPriority_[detectedBalls_.at(index2).labelNum_];
	});

	//判断前进路线上是否有球
	if (!pickedBallsIndex_.empty())
	{
		Point3f firstBallPosition = detectedBalls_.at(pickedBallsIndex_.front()).cameraPosition_;
		float leftLimit = std::min(-ROBOT_WIDTH_LIMIT, firstBallPosition.x);
		float rightLimit = std::max(ROBOT_WIDTH_LIMIT, firstBallPosition.x);
		for (int index: pickedBallsIndex_)
		{
			Point3f &cameraPosition = detectedBalls_.at(index).cameraPosition_;
			if (cameraPosition.x > leftLimit && cameraPosition.x < rightLimit && cameraPosition.z < firstBallPosition.z)
			{
				haveBallInFront_ = true;
				break;
			}
		}
	}
}

//数据输出
void BackDataProcessor::outputPosition(DataSender &dataSender)
{
	int data[4] = {0};
	if (!pickedBallsIndex_.empty())
	{
		Ball &tempBall = detectedBalls_.at(pickedBallsIndex_.front());
		Point3i cameraPosition = tempBall.cameraPosition_;

		data[0] = cameraPosition.x;
		data[1] = cameraPosition.y;
		data[2] = cameraPosition.z;
		data[3] = newLabelNum_[tempBall.labelNum_];
	}
	dataSender.writeToBuffer(0, 4, data);
	dataSender.writeToBuffer(19, 1, (int *) &haveBallInFront_);
}

//画图
void BackDataProcessor::drawBoxes(RsCameraLoader *rsCameraArray)
{
	for (int index: pickedBallsIndex_)
	{
		Ball &tempBall = detectedBalls_.at(index);
		Mat &img = rsCameraArray[tempBall.cameraId_].colorImg_;

		rectangle(img, tempBall, GREEN, 2);
		putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G") + " x: "
		             + std::to_string(tempBall.cameraPosition_.x).substr(0, 6) + " y: " + std::to_string(tempBall.cameraPosition_.y).substr(0, 6)
		             + " z: " + std::to_string(tempBall.cameraPosition_.z).substr(0, 6), Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6,
		        GREEN, 2);
	}
}

void BackDataProcessor::resetProcessor()
{
	detectedBalls_.clear();
	pickedBallsIndex_.clear();
	haveBallInFront_ = false;
}