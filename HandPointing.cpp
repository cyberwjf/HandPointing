#include "HandPointing.h"

#include "pxcsensemanager.h"
#include "pxccursorconfiguration.h"
#include "pxccursordata.h"
#include "pxchandconfiguration.h"
#include "pxchandcursormodule.h"
#include "pxchanddata.h"
#include "pxchandmodule.h"

#include "Definitions.h"

#include <iostream>
#include <fstream>

HandPointing::HandPointing() : m_live(false)
{
}

std::string HandPointing::toString()
{
	char buf[512] = { 0 };
	char *p = buf;

	int size = snprintf(p, sizeof(buf), "count : %d\n", m_model.size());

	for (DirectionModel point : m_model)
	{
		p = p + size;
		size = snprintf(p, (sizeof(buf) - (p - buf)), "{ %d : %ff, %ff, %ff }\n", point.id, point.centerPoint.x, point.centerPoint.y, point.centerPoint.z);
	}

	return std::string(buf);
}

void HandPointing::createInstance()
{
	// Setup
	m_session = PXCSession::CreateInstance();
	if (!m_session)
	{
		std::printf("Failed Creating PXCSession\n");
		return;
	}

	// Create an instance of the SenseManager
	m_senseManager = m_session->CreateSenseManager();
	if (m_senseManager == nullptr)
	{
		m_session->Release();
		m_session = nullptr;
		std::printf("Failed Creating PXCSenseManager\n");
		return;
	}
}

void HandPointing::release()
{
	if (m_cursorDataOutput != nullptr)
	{
		m_cursorDataOutput->Release();
		m_cursorDataOutput = nullptr;
	}

	if (m_cursorConfiguration != nullptr)
	{
		m_cursorConfiguration->Release();
		m_cursorConfiguration = nullptr;
	}

	if (m_handCursorModule != nullptr)
	{
		m_handCursorModule->Release();
		m_handCursorModule = nullptr;
	}

	if (m_senseManager != nullptr)
	{
		m_senseManager->Release();
		m_senseManager = nullptr;
	}

	if (m_session != nullptr)
	{
		m_session->Release();
	}

}

int HandPointing::enableCursorTracking()
{
	// Enable cursor tracking
	if (m_senseManager->EnableHandCursor() != PXC_STATUS_NO_ERROR)
	{
		std::printf("Failed EnableHandCursor\n");
		return 1;
	}

	// Get an instance of PXCHandCursorModule 
	auto m_handCursorModule = m_senseManager->QueryHandCursor();
	if (!m_handCursorModule)
	{
		std::printf("Failed QueryHandCursor\n");
		return 1;
	}

	m_cursorDataOutput = m_handCursorModule->CreateOutput();
	if (!m_cursorDataOutput)
	{
		std::printf("Failed Creating PXCCursorData\n");
		return 1;
	}

	// Get an instance of PXCCursorConfiguration
	m_cursorConfiguration = m_handCursorModule->CreateActiveConfiguration();
	if (!m_cursorConfiguration)
	{
		std::printf("Failed Creating Cursor Configuration\n");
		return 1;
	}

	std::printf("-Alerts Are Enabled-\n");
	m_cursorConfiguration->EnableAlert(PXCCursorData::CURSOR_DETECTED | PXCCursorData::CURSOR_NOT_DETECTED);

	// Make configuration changes and apply them
	//g_cursorConfiguration->EnableEngagement(true);

	// g_cursorConfiguration->EnableAllGestures();

	// бн set other configuration options
	m_cursorConfiguration->ApplyChanges(); // Changes only take effect when you call ApplyChanges

	return 0;
}

void HandPointing::addDirection(DirectionModel direction)
{
	m_model.push_back(direction);
}

void HandPointing::loadModel(const char * filename)
{
	std::ifstream myfile;
	DirectionModel point;

	myfile.open(filename, std::ios::in | std::ios::binary);
	uint32_t size;

	myfile.read((char *)&size, sizeof(size));
	for (uint32_t i = 0; i < size; i++)
	{
		myfile.read((char *)&point, sizeof(point));
		m_model.push_back(point);
	}

	myfile.close();
}

void HandPointing::saveModel(const char * filename)
{
	std::ofstream myfile;
	myfile.open(filename, std::ios::out | std::ios::binary);

	uint32_t size = m_model.size();
	myfile.write((const char *)&size, sizeof(size));
	for (DirectionModel point : m_model)
		myfile.write((const char *)&(point), sizeof(point));

	myfile.close();
}

void HandPointing::fitRawData(const char * filename, uint32_t id)
{
	auto counter = 0, pointCount = 0;
	PXCPoint3DF32 pointSum = { 0 };

	if (filename != nullptr)
	{
		setFileName(filename);
	}

	if (m_senseManager->Init() == PXC_STATUS_NO_ERROR)
	{
		while (m_senseManager->AcquireFrame(true) == PXC_STATUS_NO_ERROR)
		{
			if (m_cursorDataOutput->Update() == PXC_STATUS_NO_ERROR)
			{
				PXCCursorData::AlertData alertData;
				for (int i = 0; i < m_cursorDataOutput->QueryFiredAlertsNumber(); ++i)
				{
					if (m_cursorDataOutput->QueryFiredAlertData(i, alertData) == PXC_STATUS_NO_ERROR)
					{
						std::printf("%s was fired at frame %d \n", Definitions::CursorAlertToString(alertData.label).c_str(), alertData.frameNumber);
						if (alertData.label == PXCHandData::ALERT_HAND_NOT_DETECTED)
						{
							m_senseManager->ReleaseFrame();
							goto end;
						}
					}
				}

				PXCCursorData::ICursor *cursor;
				for (int i = 0; i < m_cursorDataOutput->QueryNumberOfCursors(); ++i)
				{
					m_cursorDataOutput->QueryCursorData(PXCCursorData::ACCESS_ORDER_BY_TIME, i, cursor);
					if (cursor->QueryBodySide() == PXCHandData::BODY_SIDE_RIGHT)
					{
						PXCPoint3DF32 currentCursor = cursor->QueryCursorWorldPoint();

						counter ++;
						if (counter >= 100)
						{
							pointSum.x += currentCursor.x;
							pointSum.y += currentCursor.y;
							pointSum.z += currentCursor.z;
							pointCount++;
						}
					}
				}
			}
			m_senseManager->ReleaseFrame();
		}

end:
		DirectionModel direction;
		direction.centerPoint = { pointSum.x / pointCount, pointSum.y / pointCount, pointSum.z / pointCount };
		direction.id = id;

		std::printf("%d : Center Point = {%ff, %ff, %ff}\n", direction.id, direction.centerPoint.x, direction.centerPoint.y, direction.centerPoint.z);

		addDirection(direction);

		m_senseManager->Release();
	}
}

void HandPointing::trainModel(std::vector<std::string> videoFiles)
{
	auto i = 1;
	for (std::string filename : videoFiles)
	{
		createInstance();

		if (enableCursorTracking() != 0)
		{
			release();
			return;
		}

		fitRawData(filename.c_str(), i);
		i++;
	}
}

void HandPointing::setFileName(const char * filename)
{
	std::string temp(filename);
	std::wstring file;
	file.assign(temp.begin(), temp.end());

	if (m_senseManager->QueryCaptureManager()->SetFileName(file.c_str(), false) != PXC_STATUS_NO_ERROR)
	{
		std::printf("Error: Invalid Sequence/ Sequence path\n");
		return;
	}
	m_senseManager->QueryCaptureManager()->SetRealtime(false);
}
