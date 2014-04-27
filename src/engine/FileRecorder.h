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

/* Marker File Format
 *
 * The time markers of a recording are saved in a separate plain text file
 * with the same filename as the audio file suffixed with "-events.txt".
 *
 * e.g.:
 *
 * Audio File: test.wav
 * Marker File: test-events.txt
 *
 * If a file like that is present in the same directory as the loaded audio
 * file, the markers are read from it and displayed in the audio view.
 *
 * The structure of the Marker File is CSV-like. There are two columns,
 * separated by ','. The first column represents the markers name, the second
 * column represents it’s position in seconds from the beginning.
 *
 * There are some rules for the name: it must not contain a comma and it must
 * not be empty.
 * Names starting with '_neuron' are considered spike train data and displayed
 * differently.
 * Apart from that it can be an arbitrary string.
 *
 *
 *
 * The marker position is a floating point number as understood by atof and
 * must not be empty either. Any amount of white space in the second column is
 * ignored.
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
	bool start(const std::string &filename);
	void stop(const MetadataChunk *meta);

	static int parseMetadataStr(MetadataChunk *meta, const char *str);

	bool recording() const;
	const std::string &filename() const;

	static std::string eventTxtFilename(const std::string &filename);
	float recordTime() const;
	void advance();

	static void parseMarkerTextFile(std::list<std::pair<std::string, int64_t> > &markers, const std::string &filename, int sampleRate);
	void writeMarkerTextFile(const std::string &filename, const std::list<std::pair<std::string, int64_t> > &markers) const;
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
};

}

#endif
