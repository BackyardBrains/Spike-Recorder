//
//  EdfRecorder.hpp
//  SpikeRecorder
//
//  Created by Stanislav on 27/04/2022.
//  Copyright © 2022 BackyardBrains. All rights reserved.
//

#ifndef EdfRecorder_hpp
#define EdfRecorder_hpp


#include "RecordingManager.h"
#include <fstream>


namespace BackyardBrains {

class RecordingManager;

class EdfRecorder {
public:
    EdfRecorder(RecordingManager &manager);
    ~EdfRecorder();
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
    int currentHandle;
    std::string _filename;
    int64_t _startPos;
    int64_t _oldPos;
   
};

}

#endif /* EdfRecorder_hpp */
