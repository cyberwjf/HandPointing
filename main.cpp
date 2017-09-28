#include <windows.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

#include "pxccursorconfiguration.h"
#include "pxccursordata.h"
#include "pxchandconfiguration.h"
#include "pxchandcursormodule.h"
#include "pxchanddata.h"
#include "pxchandmodule.h"
#include "pxcsensemanager.h"

#include "Definitions.h"

#include "HandPointing.h"

bool g_stop = false;	// user closes application

PXCSession *g_session;
PXCSenseManager *g_senseManager;
PXCHandModule *g_handModule;
PXCHandData *g_handDataOutput;
PXCHandConfiguration *g_handConfiguration;
PXCHandCursorModule *g_handCursorModule;
PXCCursorData *g_cursorDataOutput;
PXCCursorConfiguration *g_cursorConfiguration;
                          
PXCPoint3DF32 LeftSide1 = { 0.110621f, -0.044761f, 0.444373f };
PXCPoint3DF32 LeftSide2 = { 0.111002f, -0.046654f, 0.319506f };
PXCPoint3DF32 LeftSide3 = { 0.144047f, -0.114175f, 0.454760f };
PXCPoint3DF32 LeftSide4 = { 0.103986f, -0.132723f, 0.323677f };

/*
PXCPoint3DF32 LeftSide1 = {0.089525f, -0.039210f, 0.427518f }; // TopLeft
PXCPoint3DF32 LeftSide2 = {0.089967f, -0.047948f, 0.317896f }; // TopRight
PXCPoint3DF32 LeftSide3 = {0.130838f, -0.109801f, 0.444585f }; // BottomLeft
PXCPoint3DF32 LeftSide4 = {0.096176f, -0.121912f, 0.330914f }; // BottomRight
*/

std::vector<uint32_t> g_counter(4);

PXCPoint3DF32 pointSum = { 0 };
uint32_t pointCount = 0;

uint32_t g_frameCount = 0;
uint32_t g_id = 0;

bool g_test = true;
bool g_live = true;
bool g_alerts = true;

std::wstring g_sequencePath;

int main(int argc, char * argv[]);

void releaseAll();

void EnableFullHandTracking(void)
{
	if (g_senseManager->EnableHand(0) != PXC_STATUS_NO_ERROR)
	{
		releaseAll();
		std::printf("Failed Enabling Hand Module\n");
		return;
	}

	g_handModule = g_senseManager->QueryHand();
	if (!g_handModule)
	{
		releaseAll();
		std::printf("Failed Creating PXCHandModule\n");
		return;
	}

	g_handDataOutput = g_handModule->CreateOutput();
	if (!g_handDataOutput)
	{
		releaseAll();
		std::printf("Failed Creating PXCHandData\n");
		return;
	}

	g_handConfiguration = g_handModule->CreateActiveConfiguration();
	if (!g_handConfiguration)
	{
		releaseAll();
		std::printf("Failed Creating PXCHandConfiguration\n");
		return;
	}

	g_handConfiguration->SetTrackingMode(PXCHandData::TRACKING_MODE_FULL_HAND);

	if (true)
	{
		std::printf("-Gestures Are Enabled-\n");
		g_handConfiguration->EnableAllGestures();
	}

	if (true)
	{
		std::printf("-Alerts Are Enabled-\n");
		g_handConfiguration->EnableAllAlerts();
	}

	// Apply configuration setup
	g_handConfiguration->ApplyChanges();

	std::printf("-Skeleton Information Enabled-\n");
}

void PrintFullHandInfo(void)
{
	// Get current hand outputs
	if (g_handDataOutput->Update() == PXC_STATUS_NO_ERROR)
	{
		PXCHandData::IHand *hand;
		PXCHandData::JointData jointData;
		for (int i = 0; i < g_handDataOutput->QueryNumberOfHands(); ++i)
		{
			g_handDataOutput->QueryHandData(PXCHandData::ACCESS_ORDER_BY_TIME, i, hand);
			std::string handSide = "Unknown Hand";
			handSide = hand->QueryBodySide() == PXCHandData::BODY_SIDE_LEFT ? "Left Hand" : "Right Hand";

			std::printf("%s\n==============\n", handSide.c_str());
			for (int j = 0; j < 22; ++j)
			{
				if (hand->QueryTrackedJoint((PXCHandData::JointType)j, jointData) == PXC_STATUS_NO_ERROR)
				{
					std::printf("     %s)\tX: %f, Y: %f, Z: %f \n", Definitions::JointToString((PXCHandData::JointType)j).c_str(), jointData.positionWorld.x, jointData.positionWorld.y, jointData.positionWorld.z);
				}
			}
		}
	}
}

int EnableCursorTracking(void)
{
	// Enable cursor tracking
	if (g_senseManager->EnableHandCursor() != PXC_STATUS_NO_ERROR)
	{
		releaseAll();
		std::printf("Failed EnableHandCursor\n");
		return 1;
	}

	// Get an instance of PXCHandCursorModule 
	g_handCursorModule = g_senseManager->QueryHandCursor();
	if (!g_handCursorModule)
	{
		releaseAll();
		std::printf("Failed QueryHandCursor\n");
		return 1;
	}

	g_cursorDataOutput = g_handCursorModule->CreateOutput();
	if (!g_cursorDataOutput)
	{
		releaseAll();
		std::printf("Failed Creating PXCCursorData\n");
		return 1;
	}

	// Get an instance of PXCCursorConfiguration
	g_cursorConfiguration = g_handCursorModule->CreateActiveConfiguration();
	if (!g_cursorConfiguration)
	{
		releaseAll();
		std::printf("Failed Creating Cursor Configuration\n");
		return 1;
	}

	std::printf("-Alerts Are Enabled-\n");
	g_cursorConfiguration->EnableAlert(PXCCursorData::CURSOR_DETECTED | PXCCursorData::CURSOR_NOT_DETECTED);

	// Make configuration changes and apply them
	//g_cursorConfiguration->EnableEngagement(true);

	// g_cursorConfiguration->EnableAllGestures();

	// бн set other configuration options
	g_cursorConfiguration->ApplyChanges(); // Changes only take effect when you call ApplyChanges

	return 0;
}

float distance(PXCPoint3DF32 a, PXCPoint3DF32 b)
{
	return std::sqrt(std::pow((a.x - b.x), 2)
		+ std::pow((a.y - b.y), 2)
		+ std::pow((a.z - b.z), 2));
}

void CursorInfoHandler(void)
{
	// Get current hand outputs
	if (g_cursorDataOutput->Update() == PXC_STATUS_NO_ERROR)
	{
		PXCCursorData::ICursor *cursor;
		for (int i = 0; i < g_cursorDataOutput->QueryNumberOfCursors(); ++i)
		{
			g_cursorDataOutput->QueryCursorData(PXCCursorData::ACCESS_ORDER_BY_TIME, i, cursor);
			
			if (cursor->QueryBodySide() == PXCHandData::BODY_SIDE_RIGHT)
			{
				PXCPoint3DF32 currentCursor = cursor->QueryCursorWorldPoint();

				float d1, d2, d3, d4, min;

				if (g_test) // Validation the Model
				{
					g_frameCount++;
					if (g_frameCount >= 100)
					{
						d1 = distance(LeftSide1, currentCursor);
						d2 = distance(LeftSide2, currentCursor);
						min = d1 < d2 ? d1 : d2;
						d3 = distance(LeftSide3, currentCursor);
						min = d3 < min ? d3 : min;
						d4 = distance(LeftSide4, currentCursor);
						min = d4 < min ? d4 : min;

						if (min == d1)
						{
							g_counter[0] ++;
							g_counter[1] = 0;
							g_counter[2] = 0;
							g_counter[3] = 0;
							if (g_counter[0] > 50)
							{
								std::printf("LeftSide1\n");
								g_counter[0] = 0;
							}
						}
						else if (min == d2)
						{
							g_counter[0] = 0;
							g_counter[1] ++;
							g_counter[2] = 0;
							g_counter[3] = 0;
							if (g_counter[1] > 50)
							{
								std::printf("LeftSide2\n");
								g_counter[1] = 0;
							}
						}
						else if (min == d3)
						{
							g_counter[0] = 0;
							g_counter[1] = 0;
							g_counter[2] ++;
							g_counter[3] = 0;
							if (g_counter[2] > 50)
							{
								std::printf("LeftSide3\n");
								g_counter[2] = 0;
							}
						}
						else if (min == d4)
						{
							g_counter[0] = 0;
							g_counter[1] = 0;
							g_counter[2] = 0;
							g_counter[3] ++;
							if (g_counter[3] > 50)
							{
								std::printf("LeftSide4\n");
								g_counter[3] = 0;
							}
						}
					}
				}
				else  // Train The Pointing Direction Model
				{
					//std::printf("%s\n==============\n", handSide.c_str());
					//std::printf("Cursor Image Point: X: %f, Y: %f \n", cursor->QueryCursorImagePoint().x, cursor->QueryCursorImagePoint().y);
					g_counter[0] ++;
					if (g_counter[0] >= 100)
					{
						pointSum.x += currentCursor.x;
						pointSum.y += currentCursor.y;
						pointSum.z += currentCursor.z;
						pointCount++;
					}
					//std::printf("Cursor World Point: X: %f, Y: %f, Z: %f \n", cursor->QueryCursorWorldPoint().x, cursor->QueryCursorWorldPoint().y, cursor->QueryCursorWorldPoint().z);
					//std::printf("Cursor Engagement status: %d%c \n\n", cursor->QueryEngagementPercent(), '%');
				}
			}
		}
	}
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:

		// confirm that the user wants to exit. 
	case CTRL_CLOSE_EVENT:
		g_stop = true;
		Sleep(1000);
		releaseAll();
		return(TRUE);

	default:
		return FALSE;
	}
}

std::vector<PXCPoint3DF32> readModel(const char *filename)
{
	std::vector<PXCPoint3DF32> result(4);

	std::ifstream myfile;
	PXCPoint3DF32 point;

	myfile.open(filename, std::ios::in | std::ios::binary);
	do {
		myfile.read((char *)&point, sizeof(PXCPoint3DF32));
		result.push_back(point);
	} while (myfile.eof() == false);
	myfile.close();

	return result;
}

void writeModel(const char *filename, DirectionModel model)
{
	std::ofstream myfile;
	myfile.open(filename, std::ios::out | std::ios::app | std::ios::binary);
	myfile.write((const char *)&(model.centerPoint), sizeof(model.centerPoint));
	myfile.write((const char *)&(model.id), sizeof(model.id));
	myfile.close();
}

/**
 * main - Main Entry of this project.
 * @param argc: Number of arguments.
 * @param argv: Array of argument null terminated strings.
 */
int main(int argc, char* argv[])
{
	// Set Callback to handle Console Windows operations
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

	// Process the command line parameters
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-live") == 0)
		{
			g_live = true;
		}

		if (strcmp(argv[i], "-train") == 0)
		{
			g_test = false;
		}

		if (strcmp(argv[i], "-test") == 0)
		{
			g_test = true;
		}

		if (strcmp(argv[i], "-seq") == 0)
		{
			if (argc == i + 1)
			{
				releaseAll();
				std::printf("Error: Sequence path is missing\n");
				return 1;
			}
			g_live = false;
			std::string tmp(argv[i + 1]);
			
			i++;
			g_sequencePath.clear();
			g_sequencePath.assign(tmp.begin(), tmp.end());

			std::string id = tmp.substr(0, tmp.rfind('.'));
			g_id = std::stoi(id);
			std::cout << "ID is:" << id << std::endl;
			continue;
		}

	}

	if (g_test == true) {
		//readModel("model.bin");
	}

   	// Setup
	g_session = PXCSession::CreateInstance();
	if(!g_session)
	{
		std::printf("Failed Creating PXCSession\n");
		return 1;
	}

    // Create an instance of the SenseManager
    g_senseManager = g_session->CreateSenseManager();
    if(g_senseManager == nullptr)
    {
		releaseAll();
        std::printf("Failed Creating PXCSenseManager\n");
        return 1;
    }

	EnableCursorTracking();

	if (!g_live)
	{
		if (g_senseManager->QueryCaptureManager()->SetFileName(g_sequencePath.c_str(), false) != PXC_STATUS_NO_ERROR)
		{
			releaseAll();
			std::printf("Error: Invalid Sequence/ Sequence path\n");
			return 1;
		}
		g_senseManager->QueryCaptureManager()->SetRealtime(false);
	}

	// First Initializing the sense manager
	if (g_senseManager->Init() == PXC_STATUS_NO_ERROR)
	{
		std::printf("\nPXCSenseManager Initializing OK\n========================\n");

		// Acquiring frames from input device
		while (g_senseManager->AcquireFrame(true) == PXC_STATUS_NO_ERROR && !g_stop)
		{
			// Display alerts
			if (g_alerts)
			{
				PXCCursorData::AlertData alertData;
				for (int i = 0; i < g_cursorDataOutput->QueryFiredAlertsNumber(); ++i)
				{
					if (g_cursorDataOutput->QueryFiredAlertData(i, alertData) == PXC_STATUS_NO_ERROR)
					{
						std::printf("%s was fired at frame %d \n", Definitions::CursorAlertToString(alertData.label).c_str(), alertData.frameNumber);
					}
				}
			}
			CursorInfoHandler();
			g_senseManager->ReleaseFrame();
		}

		if (g_test == false)
		{
			std::printf("Center Point = {%ff, %ff, %ff}\n", pointSum.x / pointCount, pointSum.y / pointCount, pointSum.z / pointCount);

			DirectionModel model;
			model.centerPoint = { pointSum.x / pointCount, pointSum.y / pointCount, pointSum.z / pointCount };
			model.id = g_id;

			writeModel("model.bin", model);
		}

		g_senseManager->Close();
	}
    return 0;
}

void releaseAll()
{
	if(g_handConfiguration)
	{
		g_handConfiguration->Release();
		g_handConfiguration = NULL;
	}

	if(g_cursorConfiguration)
	{
		g_cursorConfiguration->Release();
		g_cursorConfiguration = NULL;
	}

	if(g_handDataOutput)
	{
		g_handDataOutput->Release();
		g_handDataOutput = NULL;
	}

	if(g_cursorDataOutput)
	{
		g_cursorDataOutput->Release();
		g_cursorDataOutput = NULL;
	}

	if (g_senseManager)
	{
		g_senseManager->Close();
		g_senseManager->Release();
		g_senseManager = NULL;
	}

	if(g_session)
	{
		g_session->Release();
		g_session = NULL;
	}
}
