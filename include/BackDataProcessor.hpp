#pragma once

#include <map>
#include "Ball.hpp"
#include "RsCameraLoader.hpp"
#include "Functions.hpp"
#include "DataSender.hpp"
#include "Constants.hpp"

class BackDataProcessor
{
private:
	class InlinedBalls
	{
	private:
		void sortLinedBalls(BackDataProcessor &backDataProcessor);

	public:
		float ballsCenterX_;
		float ballsCenterY_;
		double gradient_;
		std::vector<int> ballsIndex_;

		InlinedBalls(int index);

		int appendBall(int index, BackDataProcessor &backDataProcessor);

		void calcBallsCenter(BackDataProcessor &backDataProcessor);

		bool checkDistance(BackDataProcessor &backDataProcessor);

		void positionRevise(BackDataProcessor &backDataProcessor, RsCameraLoader *rsCameraArray);
	};

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
	enum DetectMode
	{
		NO_BALL = 0,
		SCATTERED_BALL = 1,
		MATRIX_BALL = 2
	};

	int detectMode_;
	int ballPriority_[4] = {RED_BALL, BLUE_BALL, PURPLE_BALL, BASKET};
	int newLabelNum_[4] = {NEW_RED_BALL, NEW_BLUE_BALL, NEW_PURPLE_BALL, NEW_BASKET};

public:
	std::vector<Ball> detectedBalls_;
	std::vector<int> pickedBallsIndex_;
	std::vector<int> candidateBalls_;
	std::vector<InlinedBalls> inlinedBallsGroup_;

	void backDataProcess(RsCameraLoader *rsCameraArray);

	//数据输出
	void outputPosition(DataSender &dataSender);

	//画图
	void drawBoxes(RsCameraLoader *rsCameraArray);

	//重置处理器
	void resetProcessor();
};