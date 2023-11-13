#pragma once

#include "Basket.hpp"
#include "DataSender.hpp"
#include "WideFieldCameraLoader.hpp"
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
	bool success_ = false;

public:
	std::vector<Ball> detectedBalls_;
	std::vector<int> pickedBallsIndex_;
	std::vector<Basket> baskets_;

	void frontDataProcess();

	void outputPosition(DataSender &dataSender);

	//画图
	void drawBoxes(WideFieldCameraLoader *wideFieldCameraArray);

	void clearBallVectors();
};