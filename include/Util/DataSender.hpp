#pragma once

#include <iostream>
#include "serial.hpp"
#include "Util/Logger.hpp"

class DataSender
{
private:
	int fd_;
	static constexpr int dataNum_ = 20;
	int dataBuffer_[dataNum_];

public:
	explicit DataSender(int devIndex);

	void portInit(int devIndex);

	void writeToBuffer(int startIndex, int dataNum, const int *inputData);

	void sendData();
};