#ifndef BACKYARDBRAINS_FILEREADUTIL_H
#define BACKYARDBRAINS_FILEREADUTIL_H

#include <bass.h>
#include <vector>
#include <stdint.h>

namespace BackyardBrains {
// Utility functions for opening and reading wav files with bass

bool OpenWAVFile(const char *file, HSTREAM &handle, int &nchan, int &samplerate, int &bytespersample);
bool openAnyFile(const char *file, HSTREAM &handle, int &nchan, int &samplerate, int &bytespersample);
bool OpenBYBFile(const char *file, HSTREAM &handle, int &nchan, int &samplerate, int &bytespersample);
bool openHDF5File(const char *file, HSTREAM &handle, int &nchan, int &samplerate, int &bytespersample);
bool readAnyFile(std::vector<std::vector<int16_t> > &channels, int len, HSTREAM handle, int nchan, int bytespersample);
bool ReadWAVFile(std::vector<std::vector<int16_t> > &buffers, int len, HSTREAM handle, int nchan, int bytespersample);
bool readHDF5File(std::vector<std::vector<int16_t> > &channels, int len);
int64_t anyFilesLength(HSTREAM handle, int bytespersample, int channels);
const char * readEventsAndSpikesForAnyFile(HSTREAM handle);
const char * readEventsAndSpikesForWav(HSTREAM handle);
void readEventsAndSpikesForHDF5File();
}

#endif
