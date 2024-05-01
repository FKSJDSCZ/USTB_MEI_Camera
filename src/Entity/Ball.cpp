#include "Entity/Ball.hpp"

Ball::Ball(float centerX, float centerY, int labelNum, float confidence, int cameraId, bool isInBasket) :
		centerX_(centerX), centerY_(centerY), labelNum_(labelNum), confidence_(confidence), cameraId_(cameraId), isInBasket_(isInBasket)
{}

void Ball::toMillimeter()
{
	cameraPosition_ *= 1000;
}

void Ball::offsetToEncodingDisk(Parameters &parameters)
{
	cameraPosition_.x = parameters.changeRate_ * (cameraPosition_.x + parameters.offsetToEncodingDiskX_);
	cameraPosition_.y = cameraPosition_.y + parameters.offsetToEncodingDiskY_;
	cameraPosition_.z = cameraPosition_.z + parameters.offsetToEncodingDiskZ_;
}