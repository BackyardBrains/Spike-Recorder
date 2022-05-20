//
//  HDFReader.hpp
//  SpikeRecorder
//
//  Created by Stanislav on 12/05/2022.
//  Copyright © 2022 BackyardBrains. All rights reserved.
//

#ifndef HDFReader_hpp
#define HDFReader_hpp
#include "HDFRecorder.h"
#include <vector>
using namespace H5;
namespace BackyardBrains {

class RecordingManager;

class HDFReader {
public:
    HDFReader();
    ~HDFReader();
    bool openHDF5(const char *file, int &nchan, int &samplerate, int &bytespersample);
    int64_t lengthOfFileInSamples(){return _fileSampleLenght;}
    bool readFile(std::vector<std::vector<int16_t> > &channels, int len);
    void readEvents();
    void readSpikes();
private:
   // RecordingManager &_manager;
    DataSet* _audioDataset;
    std::string _filename;
    H5File* fileHandle;
    int64_t _fileSampleLenght;
    int _numberOfChannels;
    float _fileSecondsLength;
    float _sampleRate;
    int _bytesPerSample;
    int64_t lastOffset;
};

}
#endif /* HDFReader_hpp */
