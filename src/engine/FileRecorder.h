#ifndef BACKYARDBRAINS_RECORDINGMANAGER_H
#define BACKYARDBRAINS_RECORDINGMANAGER_H

#include "RecordingManager.h"

namespace BackyardBrains {

class FileRecorder {
public:
	FileRecorder(RecordingManager &manager);
	void start(const char *filename);
	void stop();

private:
	RecordingManager &_manager;
	
