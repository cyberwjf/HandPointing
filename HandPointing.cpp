#include "HandPointing.h"
#include <iostream>
#include <fstream>

void loadModel(const char *filename, std::vector<DirectionModel>& model)
{
	std::ifstream myfile;
	DirectionModel point;

	myfile.open(filename, std::ios::in | std::ios::binary);
	uint32_t size;

	myfile.read((char *)&size, sizeof(size));
	for (uint32_t i = 0; i < size; i++)
	{
		myfile.read((char *)&point, sizeof(point));
		model.push_back(point);
	}

	myfile.close();
}

void writeModel(const char *filename, std::vector<DirectionModel>& model)
{
	std::ofstream myfile;
	myfile.open(filename, std::ios::out | std::ios::binary);

	uint32_t size = model.size();
	myfile.write((const char *)&size, sizeof(size));
	for (DirectionModel point : model)
		myfile.write((const char *)&(point), sizeof(point));

	myfile.close();
}

std::string modelToString(std::vector<DirectionModel>& model)
{
	char buf[512] = { 0 };
	char *p = buf;

	int size = snprintf(p, sizeof(buf), "count : %d\n", model.size());

	for (DirectionModel point : model)
	{
		p = p + size;
		size = snprintf(p, (sizeof(buf) - (p - buf)), "{ %d : %ff, %ff, %ff }\n", point.id, point.centerPoint.x, point.centerPoint.y, point.centerPoint.z);
	}

	return std::string(buf);
}

HandPointing::HandPointing()
{
}

void HandPointing::train(std::string file)
{

}
