#pragma once

#if defined(WITH_OPENVINO)

#include "IEngineLoader.hpp"
#include "openvino/openvino.hpp"

class OvEngineLoader :
		public IEngineLoader
{
private:
	std::vector<std::vector<Ball>> detectedBalls_;
	std::vector<std::vector<int>> pickedBallsIndex_;
	ov::InferRequest inferRequest_;
	int batchSize_;
	int inputHeight_;
	int inputWidth_;
	int inputSize_;
	int outputSize_;
	int offsetX_;
	int offsetY_;
	int classNum_;
	float imgRatio_;
	int outputMaxNum_;
	float minConfidence_;
	float maxIou_;

	void loadEngine(std::string &modelPath, std::string &binPath, std::string &device);

	void setInOutputSize();

public:
	OvEngineLoader(std::string modelPath, std::string binPath, std::string device, int batchSize, float minConfidence, float maxIou);

	void imgProcess(Mat inputImg, int imageId) override;

	void infer() override;

	void detectDataProcess() override;

	void getBallsByCameraId(int cameraId, std::vector<Ball> &container) override;
};

#endif