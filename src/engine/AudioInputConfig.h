//
//  AudioInputConfig.hpp
//  SpikeRecorder
//
//  Created by Stanislav Mircic on 9/18/16.
//  Copyright © 2016 BackyardBrains. All rights reserved.
//

#ifndef AudioInputConfig_hpp
#define AudioInputConfig_hpp


#define INPUT_TYPE_STANDARD_AUDIO 0
#define INPUT_TYPE_ARDUINO 1
#define INPUT_TYPE_SB_PRO 2
#define INPUT_TYPE_FILE 3

#include <stdio.h>
namespace BackyardBrains {
    class AudioInputConfig {
    public:
        float gain;
        float timeScale;
        bool filter60Hz;
        bool filter50Hz;
        float filterHighPass;
        float filterLowPass;
        bool initialized;
        int inputType;
        
        AudioInputConfig();
        ~AudioInputConfig();
    private:
       
    };

}

#endif /* AudioInputConfig_hpp */
