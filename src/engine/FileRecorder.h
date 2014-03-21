#ifndef BACKYARDBRAINS_FILERECORDER_H
#define BACKYARDBRAINS_FILERECORDER_H

#include "RecordingManager.h"
#include <fstream>

/* Metadata Format
 *
 * Spike Recorder saves metadata to the files it exports. This is done to keep
 * record of color, position, threshold and other properties of the channels
 * recorded so those can be restored when the file is loaded again.
 *
 * This information is written in the LIST INFO chunk of the RIFF format, which
 * allows to store it in a key/value way.
 *
 * The keys used for this are
 *
 * cpos		channel positions (float)
 * ctrs		channel thresholds (int)
 * cgin		channel gains (float)
 * cclr		channel colors index (int)
 * cnam		channel names (string)
 *
 * ctms		global time scale (see AudioView.cpp) (float)
 *
 * cmrn		marker numbers (int)
 * cmrt		marker times (int)
 *
 * The values associated with those keys have the following format
 *
 * 		val0;val1;val2;...;valn;
 *
 * Where val0 to valn are ascii strings containing the value for the n-th
 * channel. floats/ints are represented in a format understood by atof()/atoi()
 *
 * Example:
 *
 * cpos   0.243;0.123;0.4565;
 *
 * channel 1 is at position 0.243, channel 2 at position 0.123 and channel 3
 * at position 0.4565.
 *
 *
 * Global values like ctms are specified the same way, just with only one value:
 *
 * ctms 0.0768;
 *
 */

namespace BackyardBrains {

class RecordingManager;

struct MetadataChannel {
	MetadataChannel()
		: threshold(100), colorIdx(1), pos(0.5f), gain(1.f) { }
	std::string name;
	int threshold;
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
	std::list<std::pair<std::string, int64_t> > markers;
};

class FileRecorder {
public:
	FileRecorder(RecordingManager &manager);
	~FileRecorder();
	bool start(const char *filename);
	void stop(const MetadataChunk *meta);

	static int parseMetadataStr(MetadataChunk *meta, const char *str);

	bool recording() const;

	float recordTime() const;
	void advance();
private:
	RecordingManager &_manager;
	FILE *_file;
	std::string _filename;

	int64_t _startPos;
	int64_t _oldPos;
	int _nchan;

	int16_t *_buf;
	int _bufsize;

	void writeMetadata(const MetadataChunk *meta);
	void writeMarkerTextFile(const std::list<std::pair<std::string, int64_t> > &markers) const;

	static void parseMarkerTextFile(std::list<std::pair<std::string, int64_t> > &markers, const char *filename);
};

}

#endif
