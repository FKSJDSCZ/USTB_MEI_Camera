#include "Util/DataSender.hpp"

int DataSender::timeStamp_ = 0;

DataSender::DataSender(int devIndex)
{
#if defined(WITH_SERIAL)
	portInit(devIndex);
#endif
}

void DataSender::portInit(int devIndex)
{
	fd_ = UART0_Open(fd_, ("/dev/ttyUSB" + std::to_string(devIndex)).c_str());
	if (fd_ == FALSE)
	{
		throw std::runtime_error("Error opening serial file");
	}
	int ret = UART0_Init(fd_, 115200, 0, 8, 1, 'N');
	if (ret == FALSE)
	{
		throw std::runtime_error("Error initialize serial port");
	}

	LOGGER(Logger::INFO, "Init serial successfully");
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
	unsigned char data[wordCount_ * 2 + 2];

	data[0] = 0xaa;
	for (int i = 0; i < wordCount_; ++i)
	{
		data[i * 2 + 1] = dataBuffer_[i] >> 8;
		data[i * 2 + 2] = dataBuffer_[i];
	}
	data[wordCount_ * 2 + 1] = 0xbb;

	int len = UART0_Send(fd_, data, wordCount_ * 2 + 2);
	if (len > 0)
	{
		std::cout << "[Info] data:\t\t";
		for (int i = 0; i < wordCount_; ++i)
		{
			std::cout << dataBuffer_[i] << " ";
		}
		std::cout << std::endl;
//		std::cout << "[Info] Send " << len << " data successfully" << std::endl;
	}
	else
	{
		std::cerr << "[Warning] Send data failed" << std::endl;
	}
}