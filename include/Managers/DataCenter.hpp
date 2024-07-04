#ifndef MEI_DATACENTER_HPP
#define MEI_DATACENTER_HPP

#include "Loaders/IEngineLoader.hpp"
#include "Entity/Basket.hpp"
#include "Managers/DataSender.hpp"
#include "define.hpp"


class DataCenter
{
private:
	enum PriorityTag
	{
		RED_BALL_PRIORITY = 0,
		BLUE_BALL_PRIORITY = 1,
		PURPLE_BALL_PRIORITY = 2,
		BASKET_PRIORITY = 3,
	};
	enum FrontLabelToSend
	{
		FRONT_BASKET = 0,
		FRONT_RED_BALL = 1,
		FRONT_BLUE_BALL = 2,
		FRONT_PURPLE_BALL = 3,
	};
	enum BackLabelToSend
	{
		BACK_BASKET = 0,
		BACK_RED_BALL = 1,
		BACK_BLUE_BALL = 2,
		BACK_PURPLE_BALL = 3,
	};

	int ballPriority_[4] = {
			RED_BALL_PRIORITY,
			BLUE_BALL_PRIORITY,
			PURPLE_BALL_PRIORITY,
			BASKET_PRIORITY,
	};
	int frontLabel_[4] = {
			FRONT_RED_BALL,
			FRONT_BLUE_BALL,
			FRONT_PURPLE_BALL,
			FRONT_BASKET,
	};
	int backLabel_[4] = {
			BACK_RED_BALL,
			BACK_BLUE_BALL,
			BACK_PURPLE_BALL,
			BACK_BASKET,
	};
	bool haveBallInFront_ = false;

public:
	std::vector<CameraImage> cameraImages_;
	std::vector<Ball> frontBalls_;
	std::vector<Ball> backBalls_;
	std::vector<Basket> frontBaskets_;

	void getBallData(IEngineLoader &engineLoader);

	void processFrontData();

	void processBackData(std::vector<std::shared_ptr<ICameraLoader>> &cameras);

	void setSenderBuffer(DataSender &dataSender);

	void drawFrontImage();

	void drawBackImage();

	void clearAll();
};


#endif //MEI_DATACENTER_HPP
