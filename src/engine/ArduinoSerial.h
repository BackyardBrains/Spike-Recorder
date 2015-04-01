//
//  BYBArduino.h
//  SpikeRecorder
//
//  Created by Stanislav Mircic on 11/26/14.
//  Copyright (c) 2014 BYB. All rights reserved.
//


#ifndef BACKYARDBRAINS_ARDUINOSERIAL_H
#define BACKYARDBRAINS_ARDUINOSERIAL_H

#define SIZE_OF_CIRC_BUFFER 4024
#include <iostream>
#include <fcntl.h>

#include <list>
#include <string>


//just MAC classes
#ifdef __APPLE__
#include <termios.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOBSD.h>

#elif __linux__
#include <termios.h>


#elif _WIN32
#include <windows.h>
#endif


namespace BackyardBrains {

    class ArduinoSerial {
    public:
	ArduinoSerial();
        int openPort(const char *portName);
        int readPort(int16_t *);
        std::list<std::string> list;
        void getAllPortsList();
        void closeSerial(void);
        void setNumberOfChannelsAndSamplingRate(int numberOfChannels, int samplingRate);
        int writeToPort(const void *ptr, int len);
        int maxSamplingRate();
        int maxNumberOfChannels();
        int numberOfChannels();
        const char * currentPortName();
        std::string errorString;
        bool portOpened();
    private:
        char circularBuffer[SIZE_OF_CIRC_BUFFER];
        int cBufHead;
        int cBufTail;
        int fd;//file descriptor
        int _numberOfChannels;
        int _samplingRate;
        int serialCounter;

        bool returnTailForOneAndCheck();
        bool checkIfNextByteExist();
        bool areWeAtTheEndOfFrame();
        bool checkIfHaveWholeFrame();
        #ifdef __APPLE__
            void macos_ports(io_iterator_t  * PortIterator);

        #elif _WIN32
            HANDLE port_handle;
            COMMCONFIG port_cfg_orig;
            COMMCONFIG port_cfg;
        #endif
        std::string _portName;
        bool _portOpened;
        bool triedToConfigureAgain;
    };

}

#endif /* defined(__SpikeRecorder__BYBArduino__) */
