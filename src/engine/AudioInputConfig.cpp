//
//  AudioInputConfig.cpp
//  SpikeRecorder
//
//  Created by Stanislav Mircic on 9/18/16.
//  Copyright © 2016 BackyardBrains. All rights reserved.
//

#include "AudioInputConfig.h"
namespace BackyardBrains {
    
    AudioInputConfig::AudioInputConfig(){
        gain = 1.0f;
        timeScale = 0.1f;
        filter60Hz = false;
        filter50Hz = false;
        filterHighPass = 0.0f;
        filterLowPass = 100.0f;
        initialized = false;
        inputType = INPUT_TYPE_STANDARD_AUDIO;
    }
    
    AudioInputConfig::~AudioInputConfig() {
        
    }
}