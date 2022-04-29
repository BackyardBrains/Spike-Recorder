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
#include <cerrno>
namespace BackyardBrains {



FileRecorder::FileRecorder(RecordingManager &manager) {
#ifdef USE_EDF
    recorder = new EdfRecorder(manager);
#else
    recorder = new WavTxtRecorder(manager);
#endif
    
}

FileRecorder::~FileRecorder() {
    delete recorder;
}


bool FileRecorder::start(const std::string &filename) {
	
    recorder->start(filename);
	return true;
}

void FileRecorder::stop(const MetadataChunk *meta) {
    recorder->stop(meta);
}



const std::string &FileRecorder::filename() const {
	return recorder->filename();
}

std::string FileRecorder::eventTxtFilename(const std::string &filename) {

    return WavTxtRecorder::eventTxtFilename(filename);
}

int FileRecorder::writeMarkerTextFile(const std::string &filename, const std::list<std::pair<std::string, int64_t> > &markers) const {
    return recorder->writeMarkerTextFile(filename, markers);
}

void FileRecorder::parseMarkerTextFile(std::list<std::pair<std::string, int64_t> > &markers, const std::string &filename, int sampleRate)
{
    WavTxtRecorder::parseMarkerTextFile(markers, filename, sampleRate);
}

int FileRecorder::parseMetadataStr(MetadataChunk *meta, const char *str, RecordingManager &manager) {
    return WavTxtRecorder::parseMetadataStr(meta, str, manager);
}

bool FileRecorder::recording() const {
	return recorder->recording();
}

float FileRecorder::recordTime() const {
		return recorder->recordTime();
}

void FileRecorder::advance() {
    recorder->advance();
}

}
