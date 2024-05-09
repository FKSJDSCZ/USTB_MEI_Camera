#include "Entity/Parameters.hpp"

Parameters::Parameters(float XOffsetToDisk, float YOffsetToDisk, float ZOffsetToDisk, float pitchAngle, float yawAngle, float changeRate) :
		XOffsetToDisk_(XOffsetToDisk), YOffsetToDisk_(YOffsetToDisk), ZOffsetToDisk_(ZOffsetToDisk),
		pitchAngle_(pitchAngle), yawAngle_(yawAngle), changeRate_(changeRate)
{}
