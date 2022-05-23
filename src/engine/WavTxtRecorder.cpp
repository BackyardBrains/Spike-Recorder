//
//  WavTxtRecorder.cpp
//  SpikeRecorder
//
//  Created by Stanislav on 27/04/2022.
//  Copyright © 2022 BackyardBrains. All rights reserved.
//

#include "WavTxtRecorder.h"
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
#include <cerrno>
#include "miniz.h"

#define my_max(a,b) (((a) > (b)) ? (a) : (b))
#define my_min(a,b) (((a) < (b)) ? (a) : (b))

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
    for(unsigned int i = 0; i < channels.size(); i++)    {
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

WavTxtRecorder::WavTxtRecorder(RecordingManager &manager) : _manager(manager), _file(0), _startPos(0), _buf(0), _bufsize(0) {
}

WavTxtRecorder::~WavTxtRecorder() {
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
    //if it is even number of bytes add one zero
    //and make odd
    if(ftell(f)&1)
        fputc(0, f);
}

bool WavTxtRecorder::start(const std::string &filename) {
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
    for(unsigned int i = 0; i < _manager.virtualDevices().size(); i++) {
        if(_manager.virtualDevices()[i].bound)
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

bool WavTxtRecorder::ensure_file_exists_and_is_readable(const char *pFilename)
{
  FILE *p = fopen(pFilename, "rb");
  if (!p)
    return false;

    fseek(p, 0, SEEK_END);
    uint32_t src_file_size = ftell(p);
    fseek(p, 0, SEEK_SET);

  if (src_file_size)
  {
    char buf[1];
    if (fread(buf, 1, 1, p) != 1)
    {
      fclose(p);
      return false;
    }
  }
  fclose(p);
  return true;
}


void WavTxtRecorder::stop(const MetadataChunk *meta) {
    //get number of bytes
    uint32_t size = ftell(_file);

    //make odd number of bytes if it is even
    padbyte(_file);

    //write metadata to file (num of channels)
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
    
    //----------- ZIP --------------------------


      // Open input file.

    size_t dotpos = _filename.find_last_of('.');
    std::string zipfilename = _filename.substr(0,dotpos) + ".byb";

   
    
    std::string wavfileInsideZipName = "signal.wav";
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
     
      if (!mz_zip_writer_init_file(&zip, zipfilename.c_str(), 0))
      {
        printf("Failed creating zip archive ");
        
      }
    if (ensure_file_exists_and_is_readable(_filename.c_str()))
        mz_zip_writer_add_file(&zip,wavfileInsideZipName.c_str(), _filename.c_str(), "no comment", (uint16)strlen("no comment"), MZ_BEST_COMPRESSION);
    
    std::string eventsfilename = _filename.substr(0,dotpos) + "-events.txt";
    
    std::string eventsfileInsideZipName = "signal-events.txt";
    if (ensure_file_exists_and_is_readable(eventsfilename.c_str()))
        mz_zip_writer_add_file(&zip,eventsfileInsideZipName.c_str(), eventsfilename.c_str(), "no comment", (uint16)strlen("no comment"), MZ_BEST_COMPRESSION);
    
    
    
     
    
    
    
    mz_zip_writer_finalize_archive(&zip);

    mz_zip_writer_end(&zip);

    //std::string testsstring = "<fileheader><headerversion>1.0.0</headerversion></fileheader>";
    
    
    std::ostringstream sstream;
    sstream << "<fileheader><headerversion>1.0.0</headerversion><samplerate>" << _manager.sampleRate()<<"</samplerate><numchannels>"<<_manager.numberOfChannels()<<"</numchannels>"<<"</fileheader>";
    std::string testsstring  = sstream.str();
    
    std::string fileInsideZipName = "header.xml";
    mz_bool test = mz_zip_add_mem_to_archive_file_in_place(zipfilename.c_str(), fileInsideZipName.c_str(), testsstring.c_str(), testsstring.length(),"no comment", (uint16)strlen("no comment"), MZ_BEST_COMPRESSION);
    
    //------------ END OF ZIP ------------------

}

static void write_subchunk(const char *id, const std::string &content, FILE *f) {
    fwrite(id, 4, 1, f);
    put32(content.size()+1, f);
    fwrite(content.c_str(), content.size()+1, 1, f);
    padbyte(f);
}

void WavTxtRecorder::writeMetadata(const MetadataChunk *meta) {
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
    if(_manager.isCalibrated())
    {
        std::stringstream calibrationCoefficient;
        calibrationCoefficient << _manager.getCalibrationCoeficient() << ';';
        write_subchunk("clbr", calibrationCoefficient.str(), _file);
    }
    std::stringstream deviceTypeID;
    deviceTypeID << _manager.getCurrentInputType() << ';';
    write_subchunk("cdev", deviceTypeID.str(), _file);
    
    uint32_t size = ftell(_file)-sizepos-4;
    fseek(_file, sizepos, SEEK_SET);
    put32(size, _file);
    fseek(_file, 0, SEEK_END);
}

const std::string &WavTxtRecorder::filename() const {
    return _filename;
}

std::string WavTxtRecorder::eventTxtFilename(const std::string &filename) {
    size_t dotpos = filename.find_last_of('.');
    return filename.substr(0,dotpos) + "-events.txt";
}

int WavTxtRecorder::writeMarkerTextFile(const std::string &filename, const std::list<std::pair<std::string, int64_t> > &markers) const {
#ifdef __APPLE__
    size_t lastSlash =  filename.find_last_of("/");
    if(lastSlash == std::string::npos || lastSlash==0 )
    {
        Log::warn("Could not create marker file: %s", strerror(errno));
        return 1;
    }
    std::string fullDirPath = filename.substr(0, lastSlash);
    size_t secondToLastSlash =  fullDirPath.find_last_of("/");
    
    if(lastSlash == std::string::npos )
    {
        Log::warn("Could not create marker file: %s", strerror(errno));
        return 1;
    }
    std::string parrentDir = filename.substr(secondToLastSlash, lastSlash-secondToLastSlash);
    int resultOfComparison = strcmp("/Spike Recorder", parrentDir.c_str());
    if(resultOfComparison !=0)
    {
        Log::warn("Could not create marker file: %s", strerror(errno));
        return 1;
    }
    FILE *f = fopen(filename.c_str(),"w+");
#else
    FILE *f = fopen(filename.c_str(),"w+");
#endif
    
    if(f == 0) {
        Log::warn("Could not create marker file: %s", strerror(errno));
        return 1;
    }
    fprintf(f,"# Marker IDs can be arbitrary strings.\n");
    fprintf(f,"# Marker ID,\tTime (in s)\n");

    std::list<std::pair<std::string, int64_t> >::const_iterator it;
    for(it = markers.begin(); it != markers.end(); it++)
        if(it->second - _startPos >= 0)
            fprintf(f, "%s,\t%.4f\n", it->first.c_str(), (it->second-_startPos)/(float)_manager.sampleRate());

    fclose(f);
    return 0;
}

void WavTxtRecorder::parseMarkerTextFile(std::list<std::pair<std::string, int64_t> > &markers, const std::string &filename, int sampleRate) {
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

int WavTxtRecorder::parseMetadataStr(MetadataChunk *meta, const char *str, RecordingManager &manager) {
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
            CLBR,
            CDEV
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
                else if(strncmp(beg, "clbr", p-beg) == 0)
                    keytype = CLBR;
                else if(strncmp(beg, "cdev", p-beg) == 0)
                    keytype = CDEV;
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
                        entry--;
                    break;
                case CLBR:
                        manager.setCalibrationCoeficient(atof(val.c_str()));
                        entry--;
                    break;
                case CDEV:
                        meta->deviceType = atoi(val.c_str());
                        entry--;
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

bool WavTxtRecorder::recording() const {
    return _file != NULL;
}

float WavTxtRecorder::recordTime() const {
    if(_file == NULL || _nchan == 0)
        return 0.f;

    return (ftell(_file)-44)/(float)_nchan/sizeof(int16_t)/_manager.sampleRate();
}

void WavTxtRecorder::advance() {
    if(!recording())
        return;

    int len = _manager.pos() - _oldPos;

    if(_bufsize < len) {
        delete[] _buf;
        _buf = new int16_t[_nchan*len];
        _bufsize = len;
    }

    int chani = 0;
    for(unsigned int i = 0; i < _manager.virtualDevices().size(); i++) {
        if(_manager.virtualDevices()[i].bound) {
            _manager.getData(i, _oldPos, len, _buf+chani*_bufsize);
            chani++;
        }
    }

    for(int i = 0; i < len; i++) {
        //std::cout<<_buf[i]<<" : "<<_buf[i+_bufsize]<<"\n";
        for(int j = 0; j < _nchan; j++)
        {
            put16(_buf[i+j*_bufsize], _file);
        }
    
    }

    _oldPos = _manager.pos();
}

}
