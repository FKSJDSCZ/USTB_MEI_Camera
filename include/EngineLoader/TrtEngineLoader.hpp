#pragma once

#if defined(WITH_CUDA)

#include <fstream>
#include "torch/torch.h"
#include "torchvision/ops/nms.h"
#include "cvcuda/OpStack.hpp"
#include "cvcuda/OpResize.hpp"
#include "cvcuda/OpCvtColor.hpp"
#include "cvcuda/OpConvertTo.hpp"
#include "cvcuda/OpCopyMakeBorder.hpp"
#include "cvcuda/OpReformat.hpp"
#include "cvcuda/OpNonMaximumSuppression.hpp"
#include "nvcv/Tensor.hpp"
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
	nvcv::TensorBatch imageBatch_;
	nvcv::Tensor imageTensor_;
	nvcv::Tensor inputTensor_;

	nvcv::Tensor resizedImageTensor_;
	nvcv::Tensor rgbImageTensor_;
	nvcv::Tensor borderImageTensor_;
	nvcv::Tensor normalizedImageTensor_;
	//cvcuda operators
	cvcuda::Stack stack_;
	cvcuda::Resize resize_;
	cvcuda::CvtColor cvtColor_;
	cvcuda::ConvertTo convertTo_;
	cvcuda::CopyMakeBorder copyMakeBorder_;
	cvcuda::Reformat reformat_;
	//buffers
	float *gpuOutputBuffer_;
	//parameters
	int inputImageHeight_ = 480;
	int inputImageWidth_ = 640;
	int inputLayerHeight_;
	int inputLayerWidth_;
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
	//vectors
	std::vector<std::vector<Ball>> detectedBalls_;

	void loadEngine(std::string &enginePath);

	void setInOutputSize();

	void initBuffers();

public:
	explicit TrtEngineLoader(std::string enginePath, int batchSize, float minConfidence, float maxIou);

	void setInput(cv::Mat &BGRImage, int imageId) override;

	void setInput(uint8_t *rawInput, int imageId) override;

	void preProcess() override;

	void infer() override;

	void postProcess() override;

	void getBallsByCameraId(int cameraId, std::vector<Ball> &container) override;

	~TrtEngineLoader() override;
};

#endif