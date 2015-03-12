#include "FileRecorder.h"
#include "RecordingManager.h"
#include "Log.h"
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

	for(std::list<std::pair<std::string, int64_t> >::iterator it = markers.begin(); it != markers.end(); it++) {
		std::cout << "Marker " << it->first << ": " << it->second << "\n";
	}

}

FileRecorder::FileRecorder(RecordingManager &manager) : _manager(manager), _file(0), _startPos(0), _buf(0), _bufsize(0) {
}

FileRecorder::~FileRecorder() {
	if(_file)
		stop(NULL);
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

static void padbyte(FILE *f) {
	if(ftell(f)&1)
		fputc(0, f);
}

bool FileRecorder::start(const std::string &filename) {
	_oldPos = _manager.pos();
	_startPos = _oldPos;
	_file = fopen(filename.c_str(), "wb");
	_filename = filename;
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

void FileRecorder::stop(const MetadataChunk *meta) {
	uint32_t size = ftell(_file);

	padbyte(_file);

	if(meta != NULL)
		writeMetadata(meta);

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

void FileRecorder::writeMetadata(const MetadataChunk *meta) {
	std::stringstream poss, threshs, gains, colors, names, markernums, markertimes;

	for(unsigned int i = 0; i < meta->channels.size(); i++) {
		const MetadataChannel &c = meta->channels[i];
		poss << c.pos << ';';
		threshs << c.threshold << ';';
		gains << c.gain << ';';
		colors << c.colorIdx << ';';
		names << c.name << ';';
	}

	int markers = 0;
	for(std::list<std::pair<std::string,int64_t> >::const_iterator it = meta->markers.begin(); it != meta->markers.end(); it++) {
		if(it->second - _startPos >= 0) {
			markers++;
			break;
		}
	}

	if(markers > 0)
		writeMarkerTextFile(eventTxtFilename(_filename), meta->markers);

	std::stringstream timeScale;
	timeScale << meta->timeScale << ';';

	fwrite("LIST\0\0\0\0", 8, 1, _file);
	uint32_t sizepos = ftell(_file)-4;

	fwrite("INFO", 4, 1, _file);

	write_subchunk("cpos", poss.str(), _file);
	write_subchunk("ctrs", threshs.str(), _file);
	write_subchunk("cgin", gains.str(), _file);
	write_subchunk("cclr", colors.str(), _file);
	write_subchunk("ctms", timeScale.str(), _file);
	write_subchunk("cnam", names.str(), _file);

	uint32_t size = ftell(_file)-sizepos-4;
	fseek(_file, sizepos, SEEK_SET);
	put32(size, _file);
	fseek(_file, 0, SEEK_END);
}

const std::string &FileRecorder::filename() const {
	return _filename;
}

std::string FileRecorder::eventTxtFilename(const std::string &filename) {
	size_t dotpos = filename.find_last_of('.');
	return filename.substr(0,dotpos) + "-events.txt";
}

void FileRecorder::writeMarkerTextFile(const std::string &filename, const std::list<std::pair<std::string, int64_t> > &markers) const {
	FILE *f = fopen(filename.c_str(),"w");
	fprintf(f,"# Marker IDs can be arbitrary strings.\n");
	fprintf(f,"# Marker ID,\tTime (in s)\n");

	std::list<std::pair<std::string, int64_t> >::const_iterator it;
	for(it = markers.begin(); it != markers.end(); it++)
		if(it->second - _startPos >= 0)
			fprintf(f, "%s,\t%.4f\n", it->first.c_str(), (it->second-_startPos)/(float)_manager.sampleRate());

	fclose(f);
}

void FileRecorder::parseMarkerTextFile(std::list<std::pair<std::string, int64_t> > &markers, const std::string &filename, int sampleRate) {
	FILE *f = fopen(filename.c_str(), "r");
	if(f == NULL) // Loading markers is optional and only takes place if an event file is present.
		return;

	markers.clear();
	std::vector<char> buf(521);
	unsigned int i = 0;
	enum {
		MSTART,
		MCOMMENT,
		MKEY,
		MVAL
	} mode = MSTART;

	int c;
	int line = 1;
	while((c = fgetc(f)) != EOF) {
		if(c == '#') {
			if(mode == MKEY || (mode == MVAL && i == 0)) {
				Log::error("Marker Parser Error: line %d: Key missing value.",line);
				return;
			}
			if(mode == MVAL) {
				markers.back().second = atof(std::string(buf.begin(),buf.begin()+i).c_str())*sampleRate;
			}

			mode = MCOMMENT;
		}

		if(c == '\n') {
			if(mode == MKEY) {
				Log::error("Marker Parser Error: line %d: Key missing value.", line);
				return;
			}

			if(mode == MVAL) {
				markers.back().second = atof(std::string(buf.begin(),buf.begin()+i).c_str())*sampleRate;
			}

			mode = MSTART;
			line++;
			i = 0;
			continue;
		}

		if(mode == MCOMMENT)
			continue;

		if(c == ',') {
			if(mode == MVAL) {
				Log::error("Marker Parser Error: line %d: Keys must not contain ','.", line);
				return;
			}

			if(i == 0) {
				Log::error("Marker Parser Error: line %d: Keys must not be empty.", line);
				return;
			}

			markers.push_back(std::make_pair("", 0));
			markers.back().first = std::string(buf.begin(), buf.begin()+i);
			i = 0;
			mode = MVAL;
			continue;
		}

		if(mode == MSTART)
			mode = MKEY;

		buf[i] = c;
		i++;
		if(i >= buf.size())
			buf.resize(2*buf.size());
	}

	fclose(f);
}

int FileRecorder::parseMetadataStr(MetadataChunk *meta, const char *str) {
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
			CNAM
		} keytype = CINVAL;

		const char *beg = p;
		int entry = 0;
		while(*p != 0) {
			if(*p == '=') {
				if(mode != MKEY) {
					Log::error("Metadata Parser Error: unexpected '='.");
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
				else {
					Log::error("Metadata Parser Error: skipped key '%s'.", std::string(beg,p).c_str());
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
				case CINVAL:
					assert(false);
				}
				beg=p+1;
				entry++;
			}

			p++;
		}

		if(mode == MKEY) {
			Log::error("Metadata Parser Error: expected value after key!");
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
