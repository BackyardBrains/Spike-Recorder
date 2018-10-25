//
//  KeyboardGeneratorMac.cpp
//  SpikeRecorder
//
//  Created by Stanislav on 10/24/18.
//  Copyright © 2018 BackyardBrains. All rights reserved.
//

#include <stdio.h>
#include "KeyboardGenerator.h"
#include <CoreGraphics/CoreGraphics.h>
namespace BackyardBrains {
    void simulateKeyPress()
    {
        const CGKeyCode keyCode = 0x12;
        
        CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
        CGEventRef eventDown = CGEventCreateKeyboardEvent(source, keyCode, true);
        CGEventSetFlags(eventDown, kCGEventFlagMaskCommand);
        CGEventRef eventUp = CGEventCreateKeyboardEvent(source, keyCode, false);
        
        CGEventPost(kCGSessionEventTap, eventDown);
        
        //    Keep it down for a bit
        sleep(1);
        
        CGEventPost(kCGSessionEventTap, eventUp);
        
        CFRelease(eventUp);
        CFRelease(eventDown);
        CFRelease(source);
    }
}
