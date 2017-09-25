#include <iostream>
#include "pxccursorconfiguration.h"
#include "pxccursordata.h"
#include "pxchandconfiguration.h"
#include "pxchandcursormodule.h"
#include "pxchanddata.h"
#include "pxchandmodule.h"
#include "pxcsensemanager.h"

PXCSession *g_session;
PXCSenseManager *g_senseManager;
PXCHandModule *g_handModule;
PXCHandData *g_handDataOutput;
PXCHandConfiguration *g_handConfiguration;
PXCHandCursorModule *g_handCursorModule;
PXCCursorData *g_cursorDataOutput;
PXCCursorConfiguration *g_cursorConfiguration;

void releaseAll();

/**
 * main - Main Entry of this project.
 * @param argc: Number of arguments.
 * @param argv: Array of argument null terminated strings.
 */

int main(int argc, char* argv[])
{
   	// Setup
	PXCSession *g_session = PXCSession::CreateInstance();
	if(!g_session)
	{
		std::printf("Failed Creating PXCSession\n");
		return 1;
	}

    // Create an instance of the SenseManager
    PXCSenseManager* g_senseManager = g_session->CreateSenseManager();
    if(g_senseManager == nullptr)
    {
        std::printf("Failed Creating PXCSenseManager\n");
        return 2;
    }

    // Enable cursor tracking
    g_senseManager->EnableHandCursor();

    // Get an instance of PXCHandCursorModule 
    PXCHandCursorModule * cursorModule = g_senseManager->QueryHandCursor();

    // Get an instance of PXCCursorConfiguration
    PXCCursorConfiguration* cursorConfig = cursorModule->CreateActiveConfiguration();

    // Make configuration changes and apply them
    cursorConfig->EnableEngagement(true);
    cursorConfig->EnableAllGestures();

    // бн set other configuration options
    cursorConfig->ApplyChanges(); // Changes only take effect when you call ApplyChanges

    // Initialize the SenseManager pipeline
    g_senseManager->Init();

    std::cout << "Hello World!" << std::endl;
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

