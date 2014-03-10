#ifndef BACKYARDBRAINS_FILERECORDER_H
#define BACKYARDBRAINS_FILERECORDER_H

#include "RecordingManager.h"
#include <fstream>

namespace BackyardBrains {

class RecordingManager;

class FileRecorder {
public:
	FileRecorder(RecordingManager &manager);
	~FileRecorder();
	bool start(const char *filename);
	void stop();

	bool recording() const;

	void advance();
private:

	RecordingManager &_manager;
	FILE *_file;

	int64_t _oldPos;
	int _nchan;

	int16_t *_buf;
	int _bufsize;
};

}

#endif
