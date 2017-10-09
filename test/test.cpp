#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

#include "gtest/gtest.h"
#include "HandPointing.h"
#include "test.h"

TEST(ModelTest, BasicReadWriteTest) {
	HandPointing model_in, model_out;

    DirectionModel point_in;
    point_in.centerPoint = { 0.1f, 0.2f, 0.3f };
    point_in.id = 1;

    model_in.addDirection(point_in);
    model_in.addDirection(point_in);

    model_in.saveModel("data.dat");

	std::string in = model_in.toString();

    model_out.loadModel("data.dat");

    std::string out = model_out.toString();

    EXPECT_STRCASEEQ(in.c_str(), out.c_str());

    remove("data.dat");
}

TEST(ModelTest, TrainOnFile)
{
	HandPointing model;

	model.fitRawData();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
