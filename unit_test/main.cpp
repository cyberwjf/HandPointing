#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include "HandPointing.h"

int main(int argc, char* argv[])
{
    std::vector<DirectionModel> model_in, model_out;

	DirectionModel point_in;
	point_in.centerPoint = { 0.1f, 0.2f, 0.3f };
	point_in.id = 1;

	model_in.push_back(point_in);
	model_in.push_back(point_in);

	writeModel("data.dat", model_in);

	std::string result = modelToString(model_in);
	std::cout << "model_in" << std::endl;
	std::cout << result;

	loadModel("data.dat", model_out);

	result = modelToString(model_out);
	std::cout << "model_out" << std::endl;
	std::cout << result;

    return 0;
}
