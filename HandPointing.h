#pragma once

#include "pxcdefs.h"
#include <string>

struct DirectionModel {
	PXCPoint3DF32 centerPoint;
	uint32_t id;
};

class HandPointing {
public:
	HandPointing();
	void train(std::string file);
private:

};