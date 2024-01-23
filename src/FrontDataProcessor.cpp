#include "FrontDataProcessor.hpp"

void FrontDataProcessor::frontDataProcess()
{
	//该函数涉及vector的erase()方法的多次调用 考虑到前场视野中球数量不会太多 O(n)复杂度不会有较大影响
	//依据ROBOCON2023比赛规则 III区黄区不会出现球 故不会出现“球框的正后方有球的情况” 因此凭像素坐标+神经网络分离框内外球是可行的

	std::sort(pickedBallsIndex_.begin(), pickedBallsIndex_.end(), [this](int index1, int index2) -> bool {
		Ball &ball1 = detectedBalls_.at(index1);
		Ball &ball2 = detectedBalls_.at(index2);
		if (ball1.cameraId_ == ball2.cameraId_)
		{
			return ball1.centerX_ < ball2.centerX_;
		}
		return ball1.cameraId_ < ball2.cameraId_;
	});

	//选出框 删除非框中球
	for (auto pickedIt = pickedBallsIndex_.begin(); pickedIt != pickedBallsIndex_.end();)
	{
		if (detectedBalls_.at(*(pickedIt)).labelNum_ == 3)
		{
			baskets_.emplace_back(detectedBalls_.at(*(pickedIt)));
			pickedBallsIndex_.erase(pickedIt);
		}
		else if (!detectedBalls_.at(*(pickedIt)).isInBasket_)
		{
			pickedBallsIndex_.erase(pickedIt);
		}
		else
		{
			pickedIt++;
		}
	}

	//框个数不足，退出
	if (baskets_.size() < 5)
	{
		isFullDetect_ = false;
		return;
	}

	std::cout << "[Info] Successfully detected 5 baskets" << std::endl;
	isFullDetect_ = true;
	//筛选框内球
	auto pickedIt = pickedBallsIndex_.begin();
	for (Basket &basket: baskets_)
	{
		//横向筛选 由于使用神经网络 不需要纵向筛选
		for (; pickedIt != pickedBallsIndex_.end(); ++pickedIt)
		{
			Ball &tempBall = detectedBalls_.at(*(pickedIt));
			if (basket.cameraId_ == tempBall.cameraId_ && tempBall.centerX_ > basket.x && tempBall.centerX_ < basket.x + basket.width)
			{
				basket.containedBalls_.emplace_back(*(pickedIt));
			}
			else
			{
				break;
			}
		}
		//高度升序（y降序）排序
		std::sort(basket.containedBalls_.begin(), basket.containedBalls_.end(), [this](int index1, int index2) -> bool {
			return detectedBalls_.at(index1).centerY_ > detectedBalls_.at(index2).centerY_;
		});
	}

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

	//输出框中球的状态
	for (int i = 2; i >= 0; --i)
	{
		std::cout << "[Info] " << "[Baskets Row " << std::to_string(i) << "] ";
		for (int j = 0; j < 5; ++j)
		{
			std::cout << data[5 * i + j] << " ";
		}
		std::cout << std::endl;
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

void FrontDataProcessor::resetProcessor()
{
	detectedBalls_.clear();
	pickedBallsIndex_.clear();
	baskets_.clear();
}
