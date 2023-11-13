#pragma once

#include <iostream>
#include "serial.hpp"

class DataSender
{
private:
	int fd_;
	int dataNum_ = 19;
	int dataBuffer_[19];

public:
	explicit DataSender(int devIndex);

	void portInit(int devIndex);

	void writeToBuffer(int startIndex, int dataNum, const int *inputData);

	void sendData();
};