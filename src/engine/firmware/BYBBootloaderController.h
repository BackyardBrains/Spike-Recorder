//
//  BYBBootloaderController.hpp
//  SpikeRecorder
//
//
//  Created by Stanislav on 01/04/2022.
//  Copyright © 2022 BackyardBrains. All rights reserved.
//
//  Made to update STM32 microcontrollers. Works with custom bootloader that BYB made.
//  It is used for devices like Human SpikerBox, Neuron SpikerBox (Mfi) and Spike Station 
//  Update goes like this:
//  ArduinoSerial calls Recording Manager startBootloaderProcess() if bootloaderPort is found 
//  On Windows bootloaderPort is searched in registry by VID and PID
//                        VID "VID_2E73";
//                        PID Human: "PID_0005";
//                        PID Neuron: "PID_000A";
//                        PID Spike Station: "PID_000B"
//  On Mac bootloaderPort is filtered by USB device name:
//                        Neuron   "Neuron SB Bootloader"
//                        Human    "STM32L4_Boot"    
//                        Spike Station "Spike Station Bootloader"
// Therefore every time we add new bootloader we have to add USB Name (for MacOS) and VID/PID (for Windows) to the app

#ifndef BYBBootloaderController_h
#define BYBBootloaderController_h

#define BOOTLOADER_STAGE_OFF 0
#define BOOTLOADER_STAGE_INITIALIZED 1
#include <fstream>
#include <string>
#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <list>
#include <string>
#include "constants.h"

//just MAC classes
#ifdef _WIN32
    #include <windows.h>
#endif

#define SIZE_OF_PAGE 8
namespace BackyardBrains {
struct HexRecord
{
    int address;//address where page sshould be put in memory on micro
    uint8_t dataPage[SIZE_OF_PAGE];//one page of data
};

class BYBBootloaderController
{
    public:
    
    BYBBootloaderController();
    int stage;
    std::string firmwarePath;
    std::string portName;
    #ifdef _WIN32
        HANDLE portHandle;
    #else
        int portHandle;
    #endif
    
    void startUpdateProcess();
    bool isFirmwareAvailable();
    std::list<HexRecord> dataFromFile;
    int percentOfUpdateProgress(){return progress;};
    protected:
    
    void parseLineOfHex(char * lineFromFile);
    int checkChecksum(char * lineFromFile);
    void getDataRecord(char * lineFromFile, int lengthOfData);
    void initTransferOfFirmware();
    int writeDataToSerialPort(const void *ptr, int len);
    int readDataFromSerialPort(char * buffer);
    private:
    
    FILE *_file;
    int currentIndexInsidePage;
    int currentAddress;
    HexRecord * newHexRecord;
    int progress = 0;
    
    

};
}
#endif /* BYBBootloaderController_hpp */
