//
//  EdfRecorder.cpp
//  SpikeRecorder
//
//  Created by Stanislav on 27/04/2022.
//  Copyright © 2022 BackyardBrains. All rights reserved.
//

#include "EdfRecorder.h"
#include "edflib.h"
#include "WavTxtRecorder.h"

namespace BackyardBrains {

    EdfRecorder::EdfRecorder(RecordingManager &manager): _manager(manager) {
        currentHandle = -1;
    }

    EdfRecorder::~EdfRecorder() {
       
    }

    void EdfRecorder::initBuffer(int sampleRate, int numberOfChannels)
    {
        mainBuffer = new short[sampleRate];
    }

    bool EdfRecorder::start(const std::string &filename) {
        
        //get position of buffer
        _oldPos = _manager.pos();
        _startPos = _oldPos;
        
        int i, _nchan;
        
        size_t dotpos = filename.find_last_of('.');
        _filename = filename.substr(0,dotpos) + ".edf";

        //count number of channels
        _nchan = 0;
        for( i = 0; i < _manager.virtualDevices().size(); i++) {
            if(_manager.virtualDevices()[i].bound)
                _nchan++;
        }

        currentHandle = edfopen_file_writeonly_with_params(_filename.c_str(), EDFLIB_FILETYPE_EDFPLUS, _nchan, _manager.sampleRate(), -0.1, "mV");

        if(currentHandle<0)
        {
            printf("error: edfopen_file_writeonly()\n");
            return(false);
        }
        
        initBuffer(_manager.sampleRate(), _nchan);
        
        
        for(i=0; i<_nchan; i++)
          {
            if(edf_set_physical_maximum(currentHandle, i, 100.0))
            {
              printf("error: edf_set_physical_maximum()\n");

              return(1);
            }
          }

          for(i=0; i<_nchan; i++)
          {
            if(edf_set_digital_maximum(currentHandle, i, 32768))
            {
              printf("error: edf_set_digital_maximum()\n");

              return(1);
            }
          }

          for(i=0; i<_nchan; i++)
          {
            if(edf_set_digital_minimum(currentHandle, i, -32768))
            {
              printf("error: edf_set_digital_minimum()\n");

              return(1);
            }
          }

          for(i=0; i<_nchan; i++)
          {
            if(edf_set_physical_minimum(currentHandle, i, -100.0))
            {
              printf("error: edf_set_physical_minimum()\n");

              return(1);
            }
          }


        
        

        int j=0;
        for(i = 0; i < _manager.virtualDevices().size(); i++) {
            if(_manager.virtualDevices()[i].bound)
            {
                if(edf_set_label(currentHandle, j, _manager.virtualDevices()[i].name.c_str()))
                {
                    printf("error: edf_set_label()\n");

                    return(false);
                }
                j++;
            }
        }
         
        return true;
    }

    void EdfRecorder::stop(const MetadataChunk *meta) {
        
        
        
        
        //meta->markers
        std::list<std::pair<std::string, int64_t> >::const_iterator it;
        for(it = meta->markers.begin(); it != meta->markers.end(); it++)
        {
            if(it->second - _startPos >= 0)
            {
                // it->first.c_str()
                //(it->second-_startPos)/(float)_manager.sampleRate());
                edfwrite_annotation_latin1(currentHandle, (int)(((it->second-_startPos)/(float)_manager.sampleRate())*10000), -1, it->first.c_str());
            }
        }
        edfclose_file(currentHandle);
        currentHandle=-1;
        delete[] mainBuffer;
    }



    const std::string &EdfRecorder::filename() const {
        return _filename;
    }

    std::string EdfRecorder::eventTxtFilename(const std::string &filename) {

        return filename;
    }

    int EdfRecorder::writeMarkerTextFile(const std::string &filename, const std::list<std::pair<std::string, int64_t> > &markers) const {
        return 0;
    }

    void EdfRecorder::parseMarkerTextFile(std::list<std::pair<std::string, int64_t> > &markers, const std::string &filename, int sampleRate)
    {
        
    }

    int EdfRecorder::parseMetadataStr(MetadataChunk *meta, const char *str, RecordingManager &manager) {
        return 0;
    }

    bool EdfRecorder::recording() const {
        return currentHandle>=0;
    }

    float EdfRecorder::recordTime() const {
            return ((float)(_manager.pos()  - _startPos))/(float)_manager.sampleRate();
    }

    void EdfRecorder::advance() {
       
        if(!recording())
            return;

        int64_t len = _manager.pos() - _oldPos;

        if(len<_manager.sampleRate()) {
            return;
        }


        for(unsigned int i = 0; i < _manager.virtualDevices().size(); i++) {
            if(_manager.virtualDevices()[i].bound) {
                _manager.getData(i, _oldPos, _manager.sampleRate(),mainBuffer);
                //this always records one second
                edfwrite_digital_short_samples(currentHandle, mainBuffer);
            }
        }

        _oldPos = _oldPos+_manager.sampleRate();
    }

}//namespace BYB

