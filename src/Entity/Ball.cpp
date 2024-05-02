#include "Entity/Ball.hpp"

Point3f Ball::cameraPosition()
{
	if (ballPositions_.empty())
	{
		throw std::runtime_error("Call cameraPosition() of a Ball with empty ballPositions");
	}
	return ballPositions_.front().cameraPosition_;
}

Rect2f Ball::graphRect()
{
	if (ballPositions_.empty())
	{
		throw std::runtime_error("Call graphRect() of a Ball with empty ballPositions");
	}
	return ballPositions_.front().graphRect_;
}

Point2f Ball::graphCenter()
{
	if (ballPositions_.empty())
	{
		throw std::runtime_error("Call graphCenter() of a Ball with empty ballPositions");
	}
	return ballPositions_.front().graphCenter_;
}

void Ball::merge(Ball &ball)
{
	for (const BallPosition &ballPosition: ball.ballPositions_)
	{
		ballPositions_.emplace_back(ballPosition);
	}
	if (ball.confidence_ > confidence_)
	{
		confidence_ = ball.confidence_;
		labelNum_ = ball.labelNum_;
		isInBasket_ = ball.isInBasket_;
	}
}

void Ball::addGraphPosition(float centerX, float centerY, float width, float height, float confidence, int labelNum, int cameraId, bool isInBasket)
{
	ballPositions_.emplace_back(centerX, centerY, width, height, confidence, cameraId);
	if (confidence > confidence_)
	{
		confidence_ = confidence;
		labelNum_ = labelNum;
		isInBasket_ = isInBasket;
	}
}

void Ball::setCameraPosition(RsCameraLoader *rsCameraArray)
{
	isValid_ = false;

	for (BallPosition &ballPosition: ballPositions_)
	{
		ballPosition.setCameraPosition(rsCameraArray[ballPosition.cameraId_].getCameraPosition(ballPosition.graphCenter_));
		isValid_ |= ballPosition.isValid_;
	}
}

void Ball::toMillimeter()
{
	for (BallPosition &ballPosition: ballPositions_)
	{
		ballPosition.toMillimeter();
	}
}

void Ball::offsetToEncodingDisk(RsCameraLoader *rsCameraArray)
{
	for (BallPosition &ballPosition: ballPositions_)
	{
		ballPosition.offsetToEncodingDisk(rsCameraArray[ballPosition.cameraId_].parameters_);
	}
}

void Ball::calcDistance()
{
	float summary = 0;
	for (BallPosition &ballPosition: ballPositions_)
	{
		summary += ballPosition.calcDistance();
	}
	distance_ = summary / ballPositions_.size();
}
