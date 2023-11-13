#pragma once

#include <iostream>
#include "NvInferRuntime.h"

class MeiLogger :
		public nvinfer1::ILogger
{
public:
	explicit MeiLogger(ILogger::Severity severity = ILogger::Severity::kINFO);

	ILogger::Severity severity_;

	void log(ILogger::Severity severity, const char *msg) noexcept override;
};

static MeiLogger meiLogger;