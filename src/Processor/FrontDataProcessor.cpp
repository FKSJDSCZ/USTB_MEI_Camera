#include "Processor/FrontDataProcessor.hpp"

void FrontDataProcessor::dataProcess(int imgWidth, int imgHeight)
{
	std::sort(pickedBalls_.begin(), pickedBalls_.end(), [this](Ball &ball1, Ball &ball2) -> bool {
		if (ball1.cameraId_ == ball2.cameraId_)
		{
			return ball1.centerX_ < ball2.centerX_;
		}
		return ball1.cameraId_ < ball2.cameraId_;
	});

	//选出框 删除非框中球
	for (auto ballIt = pickedBalls_.begin(); ballIt != pickedBalls_.end();)
	{
		if (ballIt->labelNum_ == 3)
		{
			baskets_.emplace_back(*(ballIt));
			pickedBalls_.erase(ballIt);
		}
//		else if (!detectedBalls_.at(*(pickedIt)).isInBasket_)
//		{
//			pickedBallsIndex_.erase(pickedIt);
//		}
		else
		{
			ballIt++;
		}
	}

	//没有框，退出
	if (baskets_.empty())
	{
		return;
	}
	std::cout << "[Info] Successfully detected " << baskets_.size() << " basket(s)" << std::endl;

	//筛选框内球
	auto ballIt = pickedBalls_.begin();
	for (Basket &basket: baskets_)
	{
		//横向筛选
		for (; ballIt != pickedBalls_.end(); ++ballIt)
		{
			if (ballIt->centerX_ > basket.x && ballIt->centerX_ < basket.x + basket.width && ballIt->centerY_ < basket.y + basket.height
			    && ballIt->centerY_ > basket.y - basket.height * 0.5)
			{
				basket.containedBalls_.push_back(*(ballIt));
			}
			else if (ballIt->centerX_ >= basket.x + basket.width)
			{
				break;
			}
		}
		//高度升序（y降序）排序
		std::sort(basket.containedBalls_.begin(), basket.containedBalls_.end(), [](Ball &ball1, Ball &ball2) -> bool {
			return ball1.centerY_ > ball2.centerY_;
		});
	}
}

void FrontDataProcessor::outputPosition(DataSender &dataSender)
{
	int data[15];
	if (baskets_.size() == 5)
	{
		for (int i = 0; i < 5; ++i)
		{
			int j = 0;
			int size = std::min(3, static_cast<int>(baskets_.at(i).containedBalls_.size()));
			for (; j < size; ++j)
			{
				data[i + j * 5] = newLabelNum_[baskets_.at(i).containedBalls_.at(j).labelNum_];
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
void FrontDataProcessor::drawBoxes(WideFieldCameraLoader &wideFieldCamera)
{
	Mat &img = wideFieldCamera.colorImg_;

	for (Ball &tempBall: pickedBalls_)
	{
		rectangle(img, tempBall, RED, 2);
		putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G")
				, Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
	}

	for (Basket &basket: baskets_)
	{
		rectangle(img, basket, GREEN, 2);
		putText(img, std::to_string(basket.labelNum_), Point(basket.x, basket.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
		for (Ball &tempBall: basket.containedBalls_)
		{
			rectangle(img, tempBall, GREEN, 2);
			putText(img, std::to_string(tempBall.labelNum_), Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
		}
	}
}

void FrontDataProcessor::resetProcessor()
{
	pickedBalls_.clear();
	baskets_.clear();
}
