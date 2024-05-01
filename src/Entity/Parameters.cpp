#include "Entity/Parameters.hpp"

Parameters::Parameters() = default;

Parameters::Parameters(float offsetToEncodingDiskX, float offsetToEncodingDiskY, float offsetToEncodingDiskZ, float changeRate) :
		offsetToEncodingDiskX_(offsetToEncodingDiskX), offsetToEncodingDiskY_(offsetToEncodingDiskY), offsetToEncodingDiskZ_(offsetToEncodingDiskZ),
		changeRate_(changeRate)
{
	parametersInit();
}

void Parameters::parametersInit()
{
	offsetPoint = Point3f(offsetToEncodingDiskX_, offsetToEncodingDiskY_, offsetToEncodingDiskZ_);
}