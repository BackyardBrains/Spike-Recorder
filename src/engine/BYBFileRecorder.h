//
//  BYBFileRecorder.hpp
//  SpikeRecorder
//
//  Created by Stanislav on 26.7.22..
//  Copyright © 2022 BackyardBrains. All rights reserved.
//

#ifndef BYBFileRecorder_hpp
#define BYBFileRecorder_hpp


#include <stdio.h>
#include "RecordingManager.h"
#include <fstream>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint;
#define BUF_SIZE (1024 * 1024)
static uint8 s_inbuf[BUF_SIZE];
static uint8 s_outbuf[BUF_SIZE];


namespace BackyardBrains {

class RecordingManager;

struct MetadataChannel {
    MetadataChannel()
        : threshold(100), colorIdx(1), pos(0.5f), gain(1.f) { }
    std::string name;
    int threshold;
    int colorIdx;
    float pos;
    float gain;
};

struct MetadataChunk {
    MetadataChunk()
        : timeScale(1.f) { }
    void print();
    float timeScale;
    int deviceType;
    std::vector<MetadataChannel> channels;
    std::list<std::pair<std::string, int64_t> > markers;
};

class BYBFileRecorder {
public:
    BYBFileRecorder(RecordingManager &manager);
    ~BYBFileRecorder();
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
    FILE *_file;
    std::string _filename;
    bool ensure_file_exists_and_is_readable(const char *pFilename);
    int64_t _startPos;
    int64_t _oldPos;
    int _nchan;

    int16_t *_buf;
    int _bufsize;

    


    void writeMetadata(const MetadataChunk *meta);
};

}

#endif /* BYBFileRecorder_hpp */
