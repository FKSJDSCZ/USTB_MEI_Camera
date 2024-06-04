#include "Processor/FrontDataProcessor.hpp"

void FrontDataProcessor::dataProcess()
{
	std::sort(pickedBalls_.begin(), pickedBalls_.end(), [](Ball &ball1, Ball &ball2) -> bool {
		return ball1.graphCenter().x < ball2.graphCenter().x;
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
//	std::cout << "[Info] Successfully detected " << baskets_.size() << " basket(s)" << std::endl;

	//筛选框内球
	auto ballIt = pickedBalls_.begin();
	for (Basket &basket: baskets_)
	{
		//横向筛选
		cv::Rect2f filterRect = cv::Rect2f(basket.graphRect().x, basket.graphRect().y - basket.graphRect().height * 0.5f,
		                                   basket.graphRect().width, basket.graphRect().height * 1.5f);
		for (; ballIt != pickedBalls_.end(); ++ballIt)
		{
			if (filterRect.contains(ballIt->graphCenter()))
			{
				basket.containedBalls_.push_back(*(ballIt));
			}
			else if (ballIt->graphCenter().x >= filterRect.x + filterRect.width)
			{
				break;
			}
		}
		//高度升序（y降序）排序
		std::sort(basket.containedBalls_.begin(), basket.containedBalls_.end(), [](Ball &ball1, Ball &ball2) -> bool {
			return ball1.graphCenter().y > ball2.graphCenter().y;
		});
	}
}

void FrontDataProcessor::outputData(DataSender &dataSender)
{
	int data[18];
	std::fill(data, data + 18, 3);
	if (baskets_.size() == 1)
	{
		int ballCount = std::min(3, static_cast<int>(baskets_.front().containedBalls_.size()));
		for (int i = 0; i < ballCount; ++i)
		{
			data[15 + i] = newLabelNum_[baskets_.front().containedBalls_.at(i).labelNum_];
		}
	}
	else if (baskets_.size() == 5)
	{
		for (int i = 0; i < 5; ++i)
		{
			int j = 0;
			int ballCount = std::min(3, static_cast<int>(baskets_.at(i).containedBalls_.size()));
			for (; j < ballCount; ++j)
			{
				data[i + j * 5] = newLabelNum_[baskets_.at(i).containedBalls_.at(j).labelNum_];
			}
			for (; j < 3; ++j)
			{
				data[i + j * 5] = 0;
			}
		}
	}

	//输出框中球的状态
//	for (int i = 2; i >= 0; --i)
//	{
//		std::cout << "[Info] " << "[Baskets Row " << std::to_string(i) << "] ";
//		for (int j = 0; j < 5; ++j)
//		{
//			std::cout << data[5 * i + j] << " ";
//		}
//		std::cout << std::endl;
//	}
	dataSender.writeToBuffer(10, 18, data);
}

//画图
void FrontDataProcessor::drawBoxes(std::vector<std::shared_ptr<RsCameraLoader>> &rsCameras)
{
	cv::Mat &img = rsCameras.front()->colorImg_;

	for (Ball &tempBall: pickedBalls_)
	{
		rectangle(img, tempBall.graphRect(), RED, 2);
		putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G"),
		        cv::Point(tempBall.graphRect().x, tempBall.graphRect().y),
		        cv::FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
	}

	for (Basket &basket: baskets_)
	{
		rectangle(img, basket.graphRect(), GREEN, 2);
		putText(img, std::to_string(basket.labelNum_), cv::Point(basket.graphRect().x, basket.graphRect().y),
		        cv::FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
		for (Ball &tempBall: basket.containedBalls_)
		{
			rectangle(img, tempBall.graphRect(), GREEN, 2);
			putText(img, std::to_string(tempBall.labelNum_), cv::Point(tempBall.graphRect().x, tempBall.graphRect().y),
			        cv::FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
		}
	}
}

void FrontDataProcessor::resetProcessor()
{
	pickedBalls_.clear();
	baskets_.clear();
}
