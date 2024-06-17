#include "Managers/DataSender.hpp"

int DataSender::timeStamp_ = 0;

DataSender::DataSender(int devIndex)
{
#if defined(WITH_SERIAL)
	portInit(devIndex);
#endif
}

void DataSender::portInit(int devIndex)
{
	fd_ = openUartSerial(("/dev/ttyUSB" + std::to_string(devIndex)).c_str());
	if (fd_ == FAILURE)
	{
		throw std::runtime_error("Error opening serial file");
	}

	if (initUartSerial(fd_, B115200, NO_FLOW_CONTROL, 8, ONE_STOP_BIT, NO_PARITY) == FAILURE)
	{
		throw std::runtime_error("Error initialize serial port");
	}

	LOGGER(Logger::INFO, "Init serial successfully", true);
}

void DataSender::writeToBuffer(int startIndex, int dataNum, const int *inputData)
{
	for (int i = 0; i < dataNum; ++i)
	{
		dataBuffer_[i + startIndex] = inputData[i];
	}
}

void DataSender::sendData()
{
	dataBuffer_[0] = timeStamp_++;
	unsigned char data[WORDCOUNT * 2 + 2];

	data[0] = 0xaa;
	for (int i = 0; i < WORDCOUNT; ++i)
	{
		data[i * 2 + 1] = dataBuffer_[i] >> 8;
		data[i * 2 + 2] = dataBuffer_[i];
	}
	data[WORDCOUNT * 2 + 1] = 0xbb;

	if (sendUartSerial(fd_, data, WORDCOUNT * 2 + 2) == SUCCESS)
	{
		std::cout << "[Info] data:\t\t";
		for (int i: dataBuffer_)
		{
			std::cout << i << " ";
		}
		std::cout << std::endl;
	}
	else
	{
		std::cerr << "[Warning] Send data failed" << std::endl;
	}
}

DataSender::~DataSender()
{
	closeUartSerial(fd_);
}
