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

	//删除框内球
	std::sort(pickedBallsIndex_.begin(), pickedBallsIndex_.end(), [this](int index1, int index2) -> bool {
		return detectedBalls_.at(index1).isInBasket_ < detectedBalls_.at(index2).isInBasket_;
	});
	while (!pickedBallsIndex_.empty() && detectedBalls_.at(pickedBallsIndex_.back()).isInBasket_)
	{
		pickedBallsIndex_.pop_back();
	}

	//按距离排序
	std::sort(pickedBallsIndex_.begin(), pickedBallsIndex_.end(), [this](int index1, int index2) -> bool {
		if (detectedBalls_.at(index1).labelNum_ == detectedBalls_.at(index2).labelNum_)
		{
			return detectedBalls_.at(index1).distance_ < detectedBalls_.at(index2).distance_;
		}
		return ballPriority_[detectedBalls_.at(index1).labelNum_] < ballPriority_[detectedBalls_.at(index2).labelNum_];
	});
}

//数据输出
void BackDataProcessor::outputPosition(DataSender &dataSender)
{
	if (!pickedBallsIndex_.empty())
	{
		Ball &tempBall = detectedBalls_.at(pickedBallsIndex_.at(0));
		Point3i cameraPosition = tempBall.cameraPosition_;
		int labelNum = tempBall.labelNum_;

		//to be deprecated
		//对于r2-gen1 当机械爪将球举起时 由于距离过近深度坐标为(0, 0, 0) 获得的相对码盘的坐标即为预设偏移量
		//此处特判球被举起（高度值突变为offsetToEncodingDiskY_）的情况
		for (int index: pickedBallsIndex_)
		{
			tempBall = detectedBalls_.at(index);
			if (std::abs(tempBall.cameraPosition_.y) >= 600)
			{
				cameraPosition = detectedBalls_.at(index).cameraPosition_;
				labelNum = tempBall.labelNum_;
				break;
			}
		}

		int data[4] = {cameraPosition.x, cameraPosition.y, cameraPosition.z, newLabelNum_[labelNum]};
		dataSender.writeToBuffer(0, 4, data);
	}
	else
	{
		int data[4] = {0};
		dataSender.writeToBuffer(0, 4, data);
	}
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
		                                                + std::to_string(tempBall.cameraPosition_.x).substr(0, 6) + " y: "
		                                                + std::to_string(tempBall.cameraPosition_.y).substr(0, 6) + " z: "
		                                                + std::to_string(tempBall.cameraPosition_.z).substr(0, 6)
				, Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
	}
}

void BackDataProcessor::clearBallVectors()
{
	detectedBalls_.clear();
	pickedBallsIndex_.clear();
}