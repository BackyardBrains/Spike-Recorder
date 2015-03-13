#ifndef BACKYARDBRAINS_FILEREADUTIL_H
#define BACKYARDBRAINS_FILEREADUTIL_H

#include <bass.h>
#include <vector>

namespace BackyardBrains {
// Utility functions for opening and reading wav files with bass

bool OpenWAVFile(const char *file, HSTREAM &handle, int &nchan, int &samplerate, int &bytespersample);

bool ReadWAVFile(std::vector<std::vector<int16_t> > &buffers, int len, HSTREAM handle, int nchan, int bytespersample);
}

#endif
