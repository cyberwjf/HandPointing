#include <windows.h>

#include <iostream>
#include "pxccursorconfiguration.h"
#include "pxccursordata.h"
#include "pxchandconfiguration.h"
#include "pxchandcursormodule.h"
#include "pxchanddata.h"
#include "pxchandmodule.h"
#include "pxcsensemanager.h"

#include "Definitions.h"

bool g_stop = false;	// user closes application

PXCSession *g_session;
PXCSenseManager *g_senseManager;
PXCHandModule *g_handModule;
PXCHandData *g_handDataOutput;
PXCHandConfiguration *g_handConfiguration;
PXCHandCursorModule *g_handCursorModule;
PXCCursorData *g_cursorDataOutput;
PXCCursorConfiguration *g_cursorConfiguration;

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

	// Make configuration changes and apply them
	//g_cursorConfiguration->EnableEngagement(true);

	// g_cursorConfiguration->EnableAllGestures();

	// бн set other configuration options
	g_cursorConfiguration->ApplyChanges(); // Changes only take effect when you call ApplyChanges

	return 0;
}

void PrintCursorInfo(void)
{
	// Get current hand outputs
	if (g_cursorDataOutput->Update() == PXC_STATUS_NO_ERROR)
	{
		PXCCursorData::ICursor *cursor;
		for (int i = 0; i < g_cursorDataOutput->QueryNumberOfCursors(); ++i)
		{
			g_cursorDataOutput->QueryCursorData(PXCCursorData::ACCESS_ORDER_BY_TIME, i, cursor);
			std::string handSide = "Unknown Hand";
			handSide = cursor->QueryBodySide() == PXCHandData::BODY_SIDE_LEFT ? "Left Hand Cursor" : "Right Hand Cursor";

			//std::printf("%s\n==============\n", handSide.c_str());
			//std::printf("Cursor Image Point: X: %f, Y: %f \n", cursor->QueryCursorImagePoint().x, cursor->QueryCursorImagePoint().y);
			std::printf("Cursor World Point: X: %f, Y: %f, Z: %f \n", cursor->QueryCursorWorldPoint().x, cursor->QueryCursorWorldPoint().y, cursor->QueryCursorWorldPoint().z);
			//std::printf("Cursor Engagement status: %d%c \n\n", cursor->QueryEngagementPercent(), '%');
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

/**
 * main - Main Entry of this project.
 * @param argc: Number of arguments.
 * @param argv: Array of argument null terminated strings.
 */

int main(int argc, char* argv[])
{
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

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

	// First Initializing the sense manager
	if (g_senseManager->Init() == PXC_STATUS_NO_ERROR)
	{
		std::printf("\nPXCSenseManager Initializing OK\n========================\n");

		// Acquiring frames from input device
		while (g_senseManager->AcquireFrame(true) == PXC_STATUS_NO_ERROR && !g_stop)
		{
			PrintCursorInfo();

			g_senseManager->ReleaseFrame();
		}
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

