#pragma once

#include "pxcdefs.h"

#include <vector>
#include <string>

struct DirectionModel {
	PXCPoint3DF32 centerPoint;
	uint32_t id;
};

void loadModel(const char *filename, std::vector<DirectionModel>& model);
void writeModel(const char *filename, std::vector<DirectionModel>& model);
std::string modelToString(std::vector<DirectionModel>& model);

class HandPointing {
public:
	HandPointing();
	void train(std::string file);
private:

};