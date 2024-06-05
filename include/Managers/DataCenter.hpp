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
		RED_BALL = 0,
		BLUE_BALL = 1,
		PURPLE_BALL = 2,
		BASKET = 3
	};
	enum FrontLabel
	{
		FRONT_RED_BALL = 1,
		FRONT_BLUE_BALL = 2,
		FRONT_PURPLE_BALL = 3,
		FRONT_BASKET = 4,
	};
	enum BackLabel
	{
		BACK_PURPLE_BALL = 0,
		BACK_RED_BALL = 1,
		BACK_BLUE_BALL = 2,
		BACK_BASKET = 3,
	};

	int ballPriority_[4] = {
			RED_BALL,
			BLUE_BALL,
			PURPLE_BALL,
			BASKET
	};
	int frontLabel_[4] = {
			FRONT_RED_BALL,
			FRONT_BLUE_BALL,
			FRONT_PURPLE_BALL,
			FRONT_BASKET
	};
	int backLabel_[4] = {
			BACK_RED_BALL,
			BACK_BLUE_BALL,
			BACK_PURPLE_BALL,
			BACK_BASKET
	};
	bool haveBallInFront_ = false;

public:
	std::vector<CameraImage> cameraImages_;
	std::vector<Ball> frontBalls_;
	std::vector<Ball> backBalls_;
	std::vector<Basket> frontBaskets_;

	void setInput(IEngineLoader &engineLoader);

	void getBallData(IEngineLoader &engineLoader);

	void processFrontData();

	void processBackData(std::vector<std::shared_ptr<RsCameraLoader>> &rsCameras);

	void setSenderBuffer(DataSender &dataSender);

	void drawFrontImage();

	void drawBackImage();

	void clearAll();
};


#endif //MEI_DATACENTER_HPP
