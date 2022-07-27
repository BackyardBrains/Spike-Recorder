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

    recorder = new BYBFileRecorder(manager);
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

    return BYBFileRecorder::eventTxtFilename(filename);
}

int FileRecorder::writeMarkerTextFile(const std::string &filename, const std::list<std::pair<std::string, int64_t> > &markers) const {
    return recorder->writeMarkerTextFile(filename, markers);
}

void FileRecorder::parseMarkerTextFile(std::list<std::pair<std::string, int64_t> > &markers, const std::string &filename, int sampleRate)
{
    BYBFileRecorder::parseMarkerTextFile(markers, filename, sampleRate);
}

int FileRecorder::parseMetadataStr(MetadataChunk *meta, const char *str, RecordingManager &manager) {
    return BYBFileRecorder::parseMetadataStr(meta, str, manager);
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
