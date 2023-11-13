#include "FrontDataProcessor.hpp"

void FrontDataProcessor::frontDataProcess()
{
	std::sort(pickedBallsIndex_.begin(), pickedBallsIndex_.end(), [this](int index1, int index2) -> bool {
		return detectedBalls_.at(index1).centerX_ < detectedBalls_.at(index2).centerX_;
	});
	//选出框
	for (int index: pickedBallsIndex_)
	{
		if (detectedBalls_.at(index).labelNum_ == 3)
		{
			baskets_.emplace_back(detectedBalls_.at(index));
		}
	}
	//框个数不足，退出
	if (baskets_.size() < 5)
	{
		isFullDetect_ = false;
		return;
	}

	std::sort(baskets_.begin(), baskets_.end(), [](Basket &basket1, Basket &basket2) -> bool {
		if (basket1.cameraId_ == basket2.cameraId_)
		{
			return basket1.centerX_ < basket2.centerX_;
		}
		return basket1.cameraId_ < basket2.cameraId_;
	});
	//去重
	for (auto it = baskets_.begin(); it != baskets_.end(); ++it)
	{
		if (it->cameraId_)
		{
			baskets_.erase(it, std::min(it + baskets_.size() - 5, baskets_.end()));
			break;
		}
	}
	//框个数过多，退出
	if (baskets_.size() > 5)
	{
		isFullDetect_ = false;
		return;
	}

	//筛选框内球
	std::cout << "[Info] Successfully detected 5 baskets" << std::endl;
	isFullDetect_ = true;
	auto pickedIt = pickedBallsIndex_.begin();
	std::vector<int> tempIndex;
	for (Basket &basket: baskets_)
	{
		tempIndex.clear();
		//横向筛选
		while (pickedIt != pickedBallsIndex_.end())
		{
			Ball &tempBall = detectedBalls_.at(*(pickedIt));
			if (tempBall.centerX_ > basket.x)
			{
				if (tempBall.centerX_ < basket.x + basket.width)
				{
					tempIndex.emplace_back(*(pickedIt));
				}
				else
				{
					break;
				}
			}
			pickedIt++;
		}
		//高度升序（y降序）排序
		std::sort(tempIndex.begin(), tempIndex.end(), [this](int index1, int index2) -> bool {
			return detectedBalls_.at(index1).centerY_ > detectedBalls_.at(index2).centerY_;
		});
		//纵向筛选
		auto tempIt = tempIndex.begin();
		while (tempIt != tempIndex.end())
		{
			Ball &tempBall = detectedBalls_.at(*(tempIt));
			if (tempBall.centerY_ < basket.y + basket.height)
			{
				if (tempBall.centerY_ > basket.y - basket.height * 0.5 && basket.containedBalls_.size() < 3)
				{
					basket.containedBalls_.emplace_back(*(tempIt));
				}
				else
				{
					break;
				}
			}
			tempIt++;
		}
	}
}

void FrontDataProcessor::outputPosition(DataSender &dataSender)
{
	int data[15];
	if (isFullDetect_)
	{
		for (int i = 0; i < 5; ++i)
		{
			int j = 0;
			for (; j < baskets_.at(i).containedBalls_.size(); ++j)
			{
				data[i + j * 5] = newLabelNum_[detectedBalls_.at(baskets_.at(i).containedBalls_.at(j)).labelNum_];
			}
			for (; j < 3; ++j)
			{
				data[i + j * 5] = 0;
			}
		}
	}
	else
	{
		std::fill(data, data + 15, 3);
	}
	dataSender.writeToBuffer(4, 15, data);
}

//画图
void FrontDataProcessor::drawBoxes(WideFieldCameraLoader *wideFieldCameraArray)
{
	for (int index: pickedBallsIndex_)
	{
		Ball &tempBall = detectedBalls_.at(index);
		Mat &img = wideFieldCameraArray[tempBall.cameraId_].colorImg_;

		rectangle(img, tempBall, RED, 2);
		putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G")
//		+ " x: " + std::to_string(tempBall.cameraPosition_.x).substr(0, 6)
//		+ " y: " + std::to_string(tempBall.cameraPosition_.y).substr(0, 6)
//		+ " z: " + std::to_string(tempBall.cameraPosition_.z).substr(0, 6)
				, Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
	}

	for (Basket &basket: baskets_)
	{
		Mat &img = wideFieldCameraArray[basket.cameraId_].colorImg_;

		rectangle(img, basket, GREEN, 2);
		putText(img, std::to_string(basket.labelNum_), Point(basket.x, basket.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
		for (int index: basket.containedBalls_)
		{
			Ball &tempBall = detectedBalls_.at(index);

			rectangle(img, tempBall, GREEN, 2);
			putText(img, std::to_string(tempBall.labelNum_), Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
		}
	}
}

void FrontDataProcessor::clearBallVectors()
{
	detectedBalls_.clear();
	pickedBallsIndex_.clear();
	baskets_.clear();
}
