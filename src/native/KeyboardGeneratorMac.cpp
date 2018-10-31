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
#include <Carbon/Carbon.h>
namespace BackyardBrains {
    void simulateKeyPress()
    {
        const CGKeyCode keyCode = 0x01;
        /*
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
        CFRelease(source);*/
        
        
        
        
        
        /*CGEventFlags flags = kCGEventFlagMaskShift;
        CGEventRef ev;
        CGEventSourceRef source = CGEventSourceCreate (kCGEventSourceStateCombinedSessionState);
        
        //press down
        ev = CGEventCreateKeyboardEvent (source, keyCode, true);
        CGEventSetFlags(ev,flags | CGEventGetFlags(ev)); //combine flags
        CGEventPost(kCGHIDEventTap,ev);
        CFRelease(ev);
        
        //press up
        ev = CGEventCreateKeyboardEvent (source, keyCode, false);
        CGEventSetFlags(ev,flags | CGEventGetFlags(ev)); //combine flags
        CGEventPost(kCGHIDEventTap,ev);
        CFRelease(ev);
        
        CFRelease(source);*/
        
        
        
       /* CGEventRef downEvt = CGEventCreateKeyboardEvent( NULL, 0, true );
        CGEventRef upEvt = CGEventCreateKeyboardEvent( NULL, 0, false );
        UniChar oneChar = '1';
        CGEventKeyboardSetUnicodeString( downEvt, 1, &oneChar );
        CGEventKeyboardSetUnicodeString( upEvt, 1, &oneChar );
        CGEventPost( kCGAnnotatedSessionEventTap, downEvt );
        CGEventPost( kCGAnnotatedSessionEventTap, upEvt );*/
        
        
        
    }
}
