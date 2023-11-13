#pragma once

#include "Ball.hpp"

class Basket :
		public Ball
{
public:
	std::vector<int> containedBalls_;

	explicit Basket(Ball ball);
};