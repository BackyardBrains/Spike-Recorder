//
//  HDFRecorder.hpp
//  SpikeRecorder
//
//  Created by Stanislav on 06/05/2022.
//  Copyright © 2022 BackyardBrains. All rights reserved.
//
//
// Downloaded libraries from:
// https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.13/hdf5-1.13.1/bin/unix/
// downloaded: hdf5-1.13.1-Std-osx1013_64-clang.tar.gz    2022-03-03 08:21    37M
//

#ifndef HDFRecorder_hpp
#define HDFRecorder_hpp

#include "RecordingManager.h"
#include <fstream>
#include "H5Cpp.h"
using namespace H5;
namespace BackyardBrains {

class RecordingManager;

class HDFRecorder {
public:
    HDFRecorder(RecordingManager &manager);
    ~HDFRecorder();
    bool start(const std::string &filename);
    void stop(const MetadataChunk *meta);

    static int parseMetadataStr(MetadataChunk *meta, const char *str, RecordingManager &manager);

    bool recording() const;
    const std::string &filename() const;

    static std::string eventTxtFilename(const std::string &filename);
    float recordTime() const;
    void advance();

    static void parseMarkerTextFile(std::list<std::pair<std::string, int64_t> > &markers, const std::string &filename, int sampleRate);
    int writeMarkerTextFile(const std::string &filename, const std::list<std::pair<std::string, int64_t> > &markers) const;
private:
    RecordingManager &_manager;
    short * mainBuffer;
    void initBuffer(int sampleRate, int numberOfChannels);

    DataSet* audioDataset;
    DataSet* eventDataset;
    std::string _filename;
    H5File* fileHandle;
    int64_t _startPos;
    int64_t _oldPos;
    int _numberOfChannels;
};

}
#endif /* HDFRecorder_hpp */
