#pragma once

#if defined(WITH_CUDA)

#include <fstream>
#include "IEngineLoader.hpp"
#include "Entity/Ball.hpp"
#include "Util/Functions.hpp"
#include "Util/MeiLogger.hpp"
#include "Constants.hpp"

using namespace cv;

class TrtEngineLoader :
		public IEngineLoader
{
private:
	nvinfer1::IRuntime *meiRuntime_ = nullptr;
	nvinfer1::ICudaEngine *meiCudaEngine_ = nullptr;
	nvinfer1::IExecutionContext *meiExecutionContext_ = nullptr;
	void *gpuBuffers_[2];
	float *cpuOutputBuffer_;
	int inputHeight_;
	int inputWidth_;
	int offsetX_;
	int offsetY_;
	int batchSize_;
	int classNum_;
	int inputSize_;
	int outputSize_;
	float imgRatio_;
	float *inputBlob_;
	int outputMaxNum_;
	float minObjectness_;
	float minConfidence_;
	float maxIou_;
	cudaStream_t meiCudaStream_;

	// 读取模型，反序列化成engine
	void loadEngine(std::string &enginePath);

	//获取网络输出层结构
	void setOutputSize();

	//分配相关内存
	void initBuffers();

public:
	explicit TrtEngineLoader(std::string enginePath, float minObjectness, float minConfidence, float maxIou);

	~TrtEngineLoader() override;

	//图像预处理
	void imgProcess(Mat inputImg) override;

	// 推理
	void infer() override;

	//后处理推理数据
	void detectDataProcess(std::vector<Ball> &detectedBalls, std::vector<int> &pickedBallsIndex, int cameraId) override;

	void detect(cv::Mat inputImg, std::vector<Ball> &detectedBalls, std::vector<int> &pickedBallsIndex, int cameraId) override;
};

#endif