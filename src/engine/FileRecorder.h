#ifndef BACKYARDBRAINS_FILERECORDER_H
#define BACKYARDBRAINS_FILERECORDER_H

#include "RecordingManager.h"
#include <fstream>

namespace BackyardBrains {

class RecordingManager;

class FileRecorder {
public:
	struct MetadataChannel {
		MetadataChannel()
			: threshold(100), colorIdx(1), pos(0.5f), gain(1.f) { }
		int16_t threshold;
		int colorIdx;
		float pos;
		float gain;
	};

	struct MetadataChunk {
		MetadataChunk()
			: timeScale(1.f) { }
		void print();
		float timeScale;
		std::vector<MetadataChannel> channels;
	};


	FileRecorder(RecordingManager &manager);
	~FileRecorder();
	bool start(const char *filename);
	void stop();

	void setMetaData(MetadataChunk *meta);
	static int parseMetaDataStr(MetadataChunk *meta, const char *str);

	bool recording() const;

	float recordTime() const;
	void advance();
private:
	RecordingManager &_manager;
	FILE *_file;

	int64_t _oldPos;
	int _nchan;

	int16_t *_buf;
	int _bufsize;

	MetadataChunk *_metadata;

	void writeMetadata();
};

}

#endif
