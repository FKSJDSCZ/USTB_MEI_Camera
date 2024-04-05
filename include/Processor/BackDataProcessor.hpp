#pragma once

#include "Entity/Ball.hpp"
#include "CameraLoader/RsCameraLoader.hpp"
#include "Util/Functions.hpp"
#include "Util/DataSender.hpp"
#include "Constants.hpp"

class BackDataProcessor
{
private:
	enum PriorityTag
	{
		RED_BALL = 0,
		BLUE_BALL = 1,
		PURPLE_BALL = 2,
		BASKET = 3
	};
	enum NewLabelTag
	{
		NEW_PURPLE_BALL = 0,
		NEW_RED_BALL = 1,
		NEW_BLUE_BALL = 2,
		NEW_BASKET = 3,
	};
	int ballPriority_[4] = {RED_BALL, BLUE_BALL, PURPLE_BALL, BASKET};
	int newLabelNum_[4] = {NEW_RED_BALL, NEW_BLUE_BALL, NEW_PURPLE_BALL, NEW_BASKET};

public:
	std::vector<Ball> detectedBalls_;
	std::vector<int> pickedBallsIndex_;

	void backDataProcess(RsCameraLoader *rsCameraArray);

	//数据输出
	void outputPosition(DataSender &dataSender);

	//画图
	void drawBoxes(RsCameraLoader *rsCameraArray);

	//重置处理器
	void resetProcessor();
};