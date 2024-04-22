#include "Processor/BackDataProcessor.hpp"

void BackDataProcessor::dataProcess(RsCameraLoader *rsCameraArray)
{
	for (Ball &tempBall: pickedBalls_)
	{
		rsCameraArray[tempBall.cameraId_].getCameraPosition(tempBall.centerX_, tempBall.centerY_, tempBall.cameraPosition_);
		tempBall.toMillimeter();//转毫米
		tempBall.offsetToEncodingDisk(rsCameraArray[tempBall.cameraId_].parameters_);//偏移到码盘
		tempBall.distance_ = Functions::calcDistance3f(tempBall.cameraPosition_, Point3f(0, 0, 0));
	}

	//删除框内球、框和坐标无效的球
	for (auto ballIt = pickedBalls_.begin(); ballIt != pickedBalls_.end();)
	{
		if (ballIt->isInBasket_ || ballIt->labelNum_ == 3 || rsCameraArray[ballIt->cameraId_].parameters_.offsetPoint == ballIt->cameraPosition_)
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

	//判断前进路线上是否有球
	if (!pickedBalls_.empty())
	{
		Point3f firstBallPosition = pickedBalls_.front().cameraPosition_;
		float leftLimit = std::min(-ROBOT_WIDTH_LIMIT, firstBallPosition.x) - 2 * RADIUS;
		float rightLimit = std::max(ROBOT_WIDTH_LIMIT, firstBallPosition.x) + 2 * RADIUS;
		float frontLimit = firstBallPosition.z - RADIUS;
		for (Ball &tempBall: pickedBalls_)
		{
			Point3f &cameraPosition = tempBall.cameraPosition_;
			if (cameraPosition.x > leftLimit && cameraPosition.x < rightLimit && cameraPosition.z < frontLimit)
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
	if (!pickedBalls_.empty())
	{
		Point3i cameraPosition = pickedBalls_.front().cameraPosition_;

		data[0] = cameraPosition.x;
		data[1] = cameraPosition.y;
		data[2] = cameraPosition.z;
		data[3] = newLabelNum_[pickedBalls_.front().labelNum_];
	}
	dataSender.writeToBuffer(0, 4, data);
	dataSender.writeToBuffer(19, 1, (int *) &haveBallInFront_);
}

//画图
void BackDataProcessor::drawBoxes(RsCameraLoader *rsCameraArray)
{
	for (Ball &tempBall: pickedBalls_)
	{
		Mat &img = rsCameraArray[tempBall.cameraId_].colorImg_;

		rectangle(img, tempBall, GREEN, 2);
		std::string text = std::to_string(tempBall.labelNum_)
		                   + (tempBall.isInBasket_ ? " B" : " G")
//		                   + " x: " + std::to_string(tempBall.cameraPosition_.x).substr(0, 6)
//		                   + " y: " + std::to_string(tempBall.cameraPosition_.y).substr(0, 6)
//		                   + " z: " + std::to_string(tempBall.cameraPosition_.z).substr(0, 6)
		;
		putText(img, text, Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
	}
}

void BackDataProcessor::resetProcessor()
{
	pickedBalls_.clear();
	haveBallInFront_ = false;
}