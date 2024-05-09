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
	std::vector<std::vector<Ball>> detectedBalls_;
	std::vector<std::vector<int>> pickedBallsIndex_;
	void *gpuBuffers_[2];
	float *inputBlob_;
	float *cpuOutputBuffer_;
	int inputHeight_;
	int inputWidth_;
	int offsetX_;
	int offsetY_;
	int batchSize_;
	int classNum_;
	int inputSize_;
	int outputSize_;
	int inputTensorSize_;
	int outputTensorSize_;
	float imgRatio_;
	int outputMaxNum_;
	float minConfidence_;
	float maxIou_;
	cudaStream_t meiCudaStream_;

	void loadEngine(std::string &enginePath);

	void setInOutputSize();

	void initBuffers();

public:
	explicit TrtEngineLoader(std::string enginePath, int batchSize, float minConfidence, float maxIou);

	void imgProcess(Mat inputImg, int imageId) override;

	void infer() override;

	void detectDataProcess() override;

	void getBallsByCameraId(int cameraId, std::vector<Ball> &container) override;

	~TrtEngineLoader() override;
};

#endif