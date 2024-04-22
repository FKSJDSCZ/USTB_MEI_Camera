#pragma once

#include "Entity/Basket.hpp"
#include "Util/DataSender.hpp"
#include "CameraLoader/WideFieldCameraLoader.hpp"
#include "Constants.hpp"

class FrontDataProcessor
{
private:
	enum NewLabelTag
	{
		NEW_RED_BALL = 1,
		NEW_BLUE_BALL = 2,
		NEW_PURPLE_BALL = 3,
		NEW_BASKET = 4,
	};
	int newLabelNum_[4] = {NEW_RED_BALL, NEW_BLUE_BALL, NEW_PURPLE_BALL, NEW_BASKET};

public:
	std::vector<Ball> pickedBalls_;
	std::vector<Basket> baskets_;

	void dataProcess(int imgWidth, int imgHeight);

	//数据输出
	void outputPosition(DataSender &dataSender);

	//画图
	void drawBoxes(WideFieldCameraLoader &wideFieldCamera);

	//重置处理器
	void resetProcessor();
};