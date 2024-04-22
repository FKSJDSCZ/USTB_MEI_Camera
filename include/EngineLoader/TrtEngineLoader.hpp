#pragma once

#if defined(WITH_CUDA)

#include <fstream>
#include "IEngineLoader.hpp"
#include "Util/MeiLogger.hpp"

class TrtEngineLoader :
		public IEngineLoader
{
private:
	std::unique_ptr<nvinfer1::IRuntime> runtime_ = nullptr;
	std::unique_ptr<nvinfer1::ICudaEngine> cudaEngine_ = nullptr;
	std::unique_ptr<nvinfer1::IExecutionContext> executionContext_ = nullptr;
	void *gpuBuffers_[2];
	float *inputBlob_;
	float *cpuOutputBuffer_;
	std::vector<Ball> detectedBalls_;
	int inputHeight_;
	int inputWidth_;
	int offsetX_;
	int offsetY_;
	int batchSize_;
	int classNum_;
	int inputSize_;
	int outputSize_;
	float imgRatio_;
	int outputMaxNum_;
	float minConfidence_;
	float maxIou_;
	cudaStream_t meiCudaStream_;

	void loadEngine(std::string &enginePath);

	void setInOutputSize();

	void initBuffers();

	void imgProcess(Mat inputImg) override;

	void infer() override;

	void detectDataProcess(std::vector<Ball> &pickedBalls, int cameraId) override;

public:
	explicit TrtEngineLoader(std::string enginePath, float minConfidence, float maxIou);

	void detect(Mat inputImg, std::vector<Ball> &pickedBalls, int cameraId) override;

	~TrtEngineLoader() override;
};

#endif