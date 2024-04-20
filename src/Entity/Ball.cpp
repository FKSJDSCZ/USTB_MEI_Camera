#include "Entity/Ball.hpp"

Ball::Ball(int id, float centerX, float centerY, int labelNum, float confidence, int cameraId, bool isInBasket) :
		id_(id), centerX_(centerX), centerY_(centerY), labelNum_(labelNum), confidence_(confidence), cameraId_(cameraId), isInBasket_(isInBasket)
{}

void Ball::toMillimeter()
{
	cameraPosition_ *= 1000;
}

void Ball::offsetToEncodingDisk(Parameters &parameters)
{
	cameraPosition_.x += parameters.offsetToEncodingDiskX_;
	cameraPosition_.y += parameters.offsetToEncodingDiskY_;
	cameraPosition_.z += parameters.offsetToEncodingDiskZ_;
}