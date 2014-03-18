#include "FileRecorder.h"
#include "RecordingManager.h"
#include <bass.h>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cassert>

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

void MetadataChunk::print() {
	for(unsigned int i = 0; i < channels.size(); i++)	{
		std::cout << "Channel " << i << "\n";
		std::cout << "pos: " << channels[i].pos << "\n";
		std::cout << "thresh: " << channels[i].threshold << "\n";
		std::cout << "name: " << channels[i].name << "\n";
		std::cout << "gain: " << channels[i].gain << "\n";
		std::cout << "colorIdx: " << channels[i].colorIdx << "\n\n";
	}
	std::cout << "timeScale: " << timeScale << "\n\n";

	for(std::map<uint8_t, int64_t>::iterator it = markers.begin(); it != markers.end(); it++) {
		std::cout << "Marker " << (int)it->first << ": " << it->second << "\n";
	}

}

FileRecorder::FileRecorder(RecordingManager &manager) : _manager(manager), _file(0), _buf(0), _bufsize(0), _metadata(NULL) {
}

FileRecorder::~FileRecorder() {
	if(_file)
		stop();
	delete[] _buf;
	delete _metadata;
}

static void put32(uint32_t num, FILE *f) {
	num = le_32(num);
	fwrite(&num, 4, 1, f);
}

static void put16(uint16_t num, FILE *f) {
	num = le_16(num);
	fwrite(&num, 2, 1, f);
}

static void padbyte(FILE *f) {
	if(ftell(f)&1)
		fputc(0, f);
}

void FileRecorder::setMetaData(MetadataChunk *meta) {
	delete _metadata;
	_metadata = meta;
}

bool FileRecorder::start(const char *filename) {
	_oldPos = _manager.pos();
	_startPos = _oldPos;
	_file = fopen(filename, "wb");
	if(_file == 0) {
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
	put32(_manager.sampleRate(), _file);

	int blockalign = _nchan*((16+7)/8);
	put32(_manager.sampleRate()*blockalign, _file);
	put16(blockalign, _file);
	put16(16, _file);

	fwrite("data\0\0\0\0", 8, 1, _file);

	return true;
}

void FileRecorder::stop() {
	uint32_t size = ftell(_file);

	padbyte(_file);

	if(_metadata != NULL)
		writeMetadata();

	fseek(_file, 40, SEEK_SET);
	put32(size-44, _file); // data chunk size
	fseek(_file, 0, SEEK_END);
	size = ftell(_file);

	fseek(_file, 4, SEEK_SET);
	put32(size-8, _file); // RIFF chunk size

	fclose(_file);
	_file = NULL;
}

static void write_subchunk(const char *id, const std::string &content, FILE *f) {
	fwrite(id, 4, 1, f);
	put32(content.size()+1, f);
	fwrite(content.c_str(), content.size()+1, 1, f);
	padbyte(f);
}

void FileRecorder::writeMetadata() {
	std::stringstream poss, threshs, gains, colors, names, markers;

	for(unsigned int i = 0; i < _metadata->channels.size(); i++) {
		MetadataChannel &c = _metadata->channels[i];
		poss << c.pos << ';';
		threshs << c.threshold << ';';
		gains << c.gain << ';';
		colors << c.colorIdx << ';';
		names << c.name << ';';
	}

	if(!_metadata->markers.empty()) {
		uint8_t max = _metadata->markers.begin()->first;
		for(std::map<uint8_t,int64_t>::iterator it = _metadata->markers.begin(); it != _metadata->markers.end(); it++) {
			if(it->first > max)
				max = it->first;
		}

		for(unsigned int i = 0; i <= max; i++) {
			if(_metadata->markers.count(i) != 0)
				markers << _metadata->markers[i]-_startPos;
			markers << ';';
		}
	}

	std::stringstream timeScale;
	timeScale << _metadata->timeScale << ';';

	fwrite("LIST\0\0\0\0", 8, 1, _file);
	uint32_t sizepos = ftell(_file)-4;

	fwrite("INFO", 4, 1, _file);

	write_subchunk("cpos", poss.str(), _file);
	write_subchunk("ctrs", threshs.str(), _file);
	write_subchunk("cgin", gains.str(), _file);
	write_subchunk("cclr", colors.str(), _file);
	write_subchunk("ctms", timeScale.str(), _file);
	write_subchunk("cnam", names.str(), _file);
	write_subchunk("cmrk", markers.str(), _file);

	uint32_t size = ftell(_file)-sizepos-4;
	fseek(_file, sizepos, SEEK_SET);
	put32(size, _file);
	fseek(_file, 0, SEEK_END);
}

int FileRecorder::parseMetaDataStr(MetadataChunk *meta, const char *str) {
	const char *p = str;
	while(*p != 0) {
		enum {
			MKEY,
			MVAL
		} mode = MKEY;

		enum {
			CINVAL,
			CPOS,
			CTRS,
			CGIN,
			CCLR,
			CTMS,
			CNAM,
			CMRK
		} keytype = CINVAL;

		const char *beg = p;
		int entry = 0;
		while(*p != 0) {
			if(*p == '=') {
				if(mode != MKEY) {
					std::cerr << "Metadata Parser Error: unexpected '='.\n";
					return 1;
				}

				if(strncmp(beg, "cpos", p-beg) == 0)
					keytype = CPOS;
				else if(strncmp(beg, "ctrs", p-beg) == 0)
					keytype = CTRS;
				else if(strncmp(beg, "cgin", p-beg) == 0)
					keytype = CGIN;
				else if(strncmp(beg, "cclr", p-beg) == 0)
					keytype = CCLR;
				else if(strncmp(beg, "ctms", p-beg) == 0)
					keytype = CTMS;
				else if(strncmp(beg, "cnam", p-beg) == 0)
					keytype = CNAM;
				else if(strncmp(beg, "cmrk", p-beg) == 0)
					keytype = CMRK;
				else {
					std::cerr << "Metadata Parser Error: skipped key '" << std::string(beg,p) << "'.\n";
					mode = MVAL;
					break;
				}
				beg = p+1;
				mode = MVAL;
			}

			if(*p == ';' && mode == MVAL) {
				if(keytype == CPOS || keytype == CTRS || keytype == CGIN || keytype == CCLR || keytype == CNAM) {
					if((int)meta->channels.size() <= entry)
						meta->channels.resize(entry+1);
				}

				std::string val(beg, p);

				switch(keytype) {
				case CPOS:
					meta->channels[entry].pos = atof(val.c_str());
					break;
				case CTRS:
					meta->channels[entry].threshold = atoi(val.c_str());
					break;
				case CGIN:
					meta->channels[entry].gain = atof(val.c_str());
					break;
				case CCLR:
					meta->channels[entry].colorIdx = atoi(val.c_str());
					break;
				case CNAM:
					meta->channels[entry].name = val;
					break;
				case CTMS:
					meta->timeScale = atof(val.c_str());
					break;
				case CMRK:
					if(val.size() != 0)
						meta->markers[entry] = atoi(val.c_str());
					break;
				case CINVAL:
					assert(false);
				}
				beg=p+1;
				entry++;
			}

			p++;
		}

		if(mode == MKEY) {
			std::cerr << "Metadata Parser Error: expected value after key!\n";
			return 1;
		}

		p += strlen(p)+1;
	}

	return 0;
}

bool FileRecorder::recording() const {
	return _file != NULL;
}

float FileRecorder::recordTime() const {
	if(_file == NULL || _nchan == 0)
		return 0.f;
	
	return (ftell(_file)-44)/(float)_nchan/sizeof(int16_t)/_manager.sampleRate();
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
