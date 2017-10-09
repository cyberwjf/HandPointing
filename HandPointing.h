#pragma once

#include "pxcdefs.h"


#include <vector>
#include <string>

class PXCSenseManager;
class PXCSession;
class PXCHandCursorModule;
class PXCCursorConfiguration;
class PXCCursorData;

struct DirectionModel {
	PXCPoint3DF32 centerPoint;
	uint32_t id;
};

class HandPointing {
public:
	HandPointing();
	std::string toString();
	void createInstance();
	void release();
	int enableCursorTracking();
	void addDirection(DirectionModel direction);
	void loadModel(const char *filename);
	void saveModel(const char *filename);
	void fitRawData(const char *filename, uint32_t id);
	void trainModel(std::vector<std::string> videoFiles);
	void setFileName(const char *filename);

private:
	bool m_live;

	std::vector<DirectionModel> m_model;
	PXCSession *m_session;
	PXCSenseManager *m_senseManager;
	PXCHandCursorModule *m_handCursorModule;
	PXCCursorConfiguration *m_cursorConfiguration;
	PXCCursorData *m_cursorDataOutput;
};