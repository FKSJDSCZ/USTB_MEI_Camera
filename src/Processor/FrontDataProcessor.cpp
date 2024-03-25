#include "Processor/FrontDataProcessor.hpp"

void FrontDataProcessor::frontDataProcess(int imgWidth, int imgHeight, bool detectRoi)
{
	//该函数涉及std::vector的erase()方法的多次调用 考虑到前场视野中球数量不会太多 O(n)复杂度的实现不会有较大影响
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
//		else if (!detectedBalls_.at(*(pickedIt)).isInBasket_)
//		{
//			pickedBallsIndex_.erase(pickedIt);
//		}
		else
		{
			pickedIt++;
		}
	}

	//没有框，退出
	if (baskets_.empty())
	{
		return;
	}
	std::cout << "[Info] Successfully detected " << baskets_.size() << " basket(s)" << std::endl;

	//检测框ROI
	if (detectRoi)
	{
		int x = static_cast<int>(baskets_.front().x) - padding_;
		int y = static_cast<int>(baskets_.front().y) - padding_;
		int width = static_cast<int>(baskets_.back().x - baskets_.front().x + baskets_.back().width) + padding_ * 2;
		int height = static_cast<int>(baskets_.front().height) + padding_ * 2;
		basketRoi_ = Rect_<int>(x, y, width, height);
		basketRoi_ &= Rect_<int>(0, 0, imgWidth, imgHeight);
		return;
	}

	//筛选框内球
	auto pickedIt = pickedBallsIndex_.begin();
	for (Basket &basket: baskets_)
	{
		//横向筛选
		for (; pickedIt != pickedBallsIndex_.end(); ++pickedIt)
		{
			Ball &tempBall = detectedBalls_.at(*(pickedIt));
			if (tempBall.centerX_ > basket.x && tempBall.centerX_ < basket.x + basket.width && tempBall.centerY_ < basket.y + basket.height
			    && tempBall.centerY_ > basket.y - basket.height * 0.5)
			{
				basket.containedBalls_.emplace_back(*(pickedIt));
			}
			else if (tempBall.centerX_ >= basket.x + basket.width)
			{
				break;
			}
		}
		//高度升序（y降序）排序
		std::sort(basket.containedBalls_.begin(), basket.containedBalls_.end(), [this](int index1, int index2) -> bool {
			return detectedBalls_.at(index1).centerY_ > detectedBalls_.at(index2).centerY_;
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
	dataSender.writeToBuffer(17, 15, data);
}

//画图
void FrontDataProcessor::drawBoxes(WideFieldCameraLoader &wideFieldCamera)
{
	for (int index: pickedBallsIndex_)
	{
		Ball tempBall = detectedBalls_.at(index);
		tempBall.x += basketRoi_.x;
		tempBall.y += basketRoi_.y;
		Mat &img = wideFieldCamera.colorImg_;

		rectangle(img, tempBall, RED, 2);
		putText(img, std::to_string(tempBall.labelNum_) + (tempBall.isInBasket_ ? " B" : " G")
//		+ " x: " + std::to_string(tempBall.cameraPosition_.x).substr(0, 6)
//		+ " y: " + std::to_string(tempBall.cameraPosition_.y).substr(0, 6)
//		+ " z: " + std::to_string(tempBall.cameraPosition_.z).substr(0, 6)
				, Point(tempBall.x, tempBall.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
	}

	for (Basket &basket: baskets_)
	{
		Basket basket_ = basket;
		basket_.x += basketRoi_.x;
		basket_.y += basketRoi_.y;
		Mat &img = wideFieldCamera.colorImg_;

		rectangle(img, basket_, GREEN, 2);
		putText(img, std::to_string(basket_.labelNum_), Point(basket_.x, basket_.y), FONT_HERSHEY_SIMPLEX, 0.6, GREEN, 2);
		for (int index: basket_.containedBalls_)
		{
			Ball tempBall = detectedBalls_.at(index);
			tempBall.x += basketRoi_.x;
			tempBall.y += basketRoi_.y;

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
	basketRoi_ = Rect_<int>(0, 0, 0, 0);
}
