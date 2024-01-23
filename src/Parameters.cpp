#include "Parameters.hpp"

Parameters::Parameters() = default;

Parameters::Parameters(float offsetToEncodingDiskX, float offsetToEncodingDiskY, float offsetToEncodingDiskZ) :
		offsetToEncodingDiskX_(offsetToEncodingDiskX), offsetToEncodingDiskY_(offsetToEncodingDiskY), offsetToEncodingDiskZ_(offsetToEncodingDiskZ)
{
	parametersInit();
}

void Parameters::parametersInit()
{
	offsetPoint = Point3f(offsetToEncodingDiskX_, offsetToEncodingDiskY_, offsetToEncodingDiskZ_);
}