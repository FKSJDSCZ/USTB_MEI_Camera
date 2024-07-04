#pragma once

#if defined(WITH_CUDA)

#include <fstream>
#include <NvInferPlugin.h>
#include "cvcuda/OpStack.hpp"
#include "cvcuda/OpResize.hpp"
#include "cvcuda/OpCvtColor.hpp"
#include "cvcuda/OpConvertTo.hpp"
#include "cvcuda/OpCopyMakeBorder.hpp"
#include "cvcuda/OpReformat.hpp"
#include "cvcuda/OpNonMaximumSuppression.hpp"
#include "nvcv/TensorBatch.hpp"
#include "nvcv/TensorDataAccess.hpp"
#include "IEngineLoader.hpp"
#include "Util/MeiLogger.hpp"

class TrtEngineLoader :
		public IEngineLoader
{
private:
	//tensorrt components
	std::unique_ptr<nvinfer1::IRuntime> runtime_ = nullptr;
	std::unique_ptr<nvinfer1::ICudaEngine> cudaEngine_ = nullptr;
	std::unique_ptr<nvinfer1::IExecutionContext> executionContext_ = nullptr;
	cudaStream_t meiCudaStream_;

	//nvcv components
	std::vector<nvcv::Tensor> imageTensors_;
	std::vector<nvcv::Tensor> resizedImageTensors_;
	std::vector<nvcv::Tensor> rgbImageTensors_;
	std::vector<nvcv::Tensor> borderImageTensors_;
	std::vector<nvcv::Tensor> normalizedImageTensors_;
	std::vector<nvcv::Tensor> reformattedImageTensors_;
	nvcv::TensorBatch inputTensorBatch_;
	nvcv::Tensor inputTensor_;

	//cvcuda operators
	cvcuda::Resize resize_;
	cvcuda::CvtColor cvtColor_;
	cvcuda::CopyMakeBorder copyMakeBorder_;
	cvcuda::ConvertTo convertTo_;
	cvcuda::Reformat reformat_;
	cvcuda::Stack stack_;

	//buffers for GPU
	int *numDetBuffer_;
	float *detBoxesBuffer_;
	float *detScoresBuffer_;
	int *detClassesBuffer_;

	//buffers for CPU
	int *cpuNumDetBuffer_;
	float *cpuDetBoxesBuffer_;
	float *cpuDetScoresBuffer_;
	int *cpuDetClassesBuffer_;

	//parameters
	std::vector<LetterboxParameter> letterboxParameters_;
	std::vector<std::vector<Ball>> detectedBalls_;
	int inputLayerHeight_;
	int inputLayerWidth_;
	int batchSize_;
	int inputSize_;
	int inputTensorSize_;
	int maxOutputNumber_;

	void loadEngine(std::string &enginePath);

	void setInOutputSize();

public:
	explicit TrtEngineLoader(std::string enginePath, int batchSize);

	void setLetterboxParameters(std::vector<std::shared_ptr<ICameraLoader>> &cameras);

	void init();

	void setInput(std::vector<CameraImage> &cameraImages);

	void setInput(cv::Mat &BGRImage, int imageId) override;

	void preProcess() override;

	void infer() override;

	void postProcess() override;

	void getBallsByCameraId(int cameraId, std::vector<Ball> &container) override;

	~TrtEngineLoader() override;
};

#endif