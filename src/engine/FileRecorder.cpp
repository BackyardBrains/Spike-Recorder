#include "FileRecorder.h"
#include "RecordingManager.h"
#include <bass.h>
#include <cstdio>
#include <cerrno>
#include <cstring>

namespace BackyardBrains {

#ifdef __BIG_ENDIAN__
static uint32_t le_32(uint32_t v) {
	return (v>>24)|((v>>8)&0xff00)|((v&0xff00)<<8)|(v<<24);
}
static uint16_t le_16(uint16_t v) {
	return (v>>8)|(v<<8);
}
#else
#define le_32(v) (v)
#define le_16(v) (v)
#endif

FileRecorder::FileRecorder(RecordingManager &manager) : _manager(manager), _file(0), _buf(0), _bufsize(0) {
}

FileRecorder::~FileRecorder() {
	if(_file)
		fclose(_file);
	delete[] _buf;
}

static void put32(uint32_t num, FILE *f) {
	num = le_32(num);
	fwrite(&num, 4, 1, f);
}

static void put16(uint16_t num, FILE *f) {
	num = le_16(num);
	fwrite(&num, 2, 1, f);
}

bool FileRecorder::start(const char *filename) {
	_oldPos = _manager.pos();
	_file = fopen(filename, "w+b");
	if(_file == 0) {
		fprintf(stderr, "Record Error: Opening '%s' for saving the recording failed: %s\n", filename, strerror(errno));
		return false;
	}
	fwrite("RIFF\0\0\0\0WAVEfmt ", 16, 1, _file);

	put32(16, _file);
	put16(1, _file);

	_nchan = 0;
	for(unsigned int i = 0; i < _manager.recordingDevices().size(); i++) {
		if(_manager.recordingDevices()[i].bound)
			_nchan++;
	}

	put16(_nchan, _file);
	put32(RecordingManager::SAMPLE_RATE, _file);

	int blockalign = _nchan*((16+7)/8);
	put32(RecordingManager::SAMPLE_RATE*blockalign, _file);
	put16(blockalign, _file);
	put16(16, _file);
	fwrite("data\0\0\0\0", 8, 1, _file);

	return true;
}

void FileRecorder::stop() {
	fclose(_file);
	_file = NULL;
}

bool FileRecorder::recording() const {
	return _file != NULL;
}

void FileRecorder::advance() {
	if(!recording())
		return;

	int len = _manager.pos() - _oldPos;

	if(_bufsize < len) {
		delete[] _buf;
		_buf = new int16_t[_nchan*len];
		_bufsize = len;
	}

	int chani = 0;
	for(unsigned int i = 0; i < _manager.recordingDevices().size(); i++) {
		if(_manager.recordingDevices()[i].bound) {
			_manager.getData(i, _oldPos, len, _buf+chani*_bufsize);
			chani++;
		}
	}

	for(int i = 0; i < len; i++) {
		for(int j = 0; j < _nchan; j++)
			put16(_buf[i+j*_bufsize], _file);
	}

	_oldPos = _manager.pos();
}

}
