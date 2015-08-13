//
//  RTConfigView.h
//  SpikeRecorder
//
//  Created by Stanislav Mircic on 8/13/15.
//  Copyright (c) 2015 BackyardBrains. All rights reserved.
//

#ifndef __SpikeRecorder__RTConfigView__
#define __SpikeRecorder__RTConfigView__

#include <stdio.h>
#include "widgets/Widget.h"
#include "widgets/PushButton.h"

namespace BackyardBrains {
    
    class RecordingManager;
    class AudioView;
    
    class RTConfigView : public Widgets::Widget {
    public:
        RTConfigView(RecordingManager &mngr, AudioView &audioView, Widget *parent = NULL);
    private:

        Widgets::PushButton *_repeatCKBox;
        void repeatPressed();

        RecordingManager &_manager;
        AudioView &_audioView;
        
        void paintEvent();
        void closePressed();
    };
}

#endif /* defined(__SpikeRecorder__RTConfigView__) */
