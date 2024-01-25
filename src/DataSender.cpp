#include "DataSender.hpp"

DataSender::DataSender(int devIndex)
{
#if defined(WITH_SERIAL)
	portInit(devIndex);
#endif
}

void DataSender::portInit(int devIndex)
{
	fd_ = UART0_Open(fd_, ("/dev/ttyUSB" + std::to_string(devIndex)).c_str());
	int err;
	do
	{
		err = UART0_Init(fd_, 115200, 0, 8, 1, 'N');
		std::cout << "[Info] Set port exactly" << std::endl;
	} while (FALSE == err || FALSE == fd_);
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
	unsigned char data[dataNum_ * 2 + 2];

	data[0] = 0xaa;
	for (int i = 0; i < dataNum_; ++i)
	{
		data[i * 2 + 1] = dataBuffer_[i] >> 8;
		data[i * 2 + 2] = dataBuffer_[i];
	}
	data[dataNum_ * 2 + 1] = 0xbb;

	int len = UART0_Send(fd_, data, dataNum_ * 2 + 2);
	if (len > 0)
	{
		std::cout << "[Info] data: ";
		for (int i = 0; i < dataNum_; ++i)
		{
			std::cout << dataBuffer_[i] << " ";
		}
		std::cout << std::endl;
		std::cout << "[Info] Send " << len << " data successfully" << std::endl;
	}
	else
	{
		std::cerr << "[Warning] Send data failed" << std::endl;
	}
}