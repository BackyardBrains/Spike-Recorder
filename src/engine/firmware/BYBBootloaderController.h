//
//  BYBBootloaderController.hpp
//  SpikeRecorder
//
//  Created by Stanislav on 01/04/2022.
//  Copyright © 2022 BackyardBrains. All rights reserved.
//

#ifndef BYBBootloaderController_h
#define BYBBootloaderController_h

#define BOOTLOADER_STAGE_OFF 0
#define BOOTLOADER_STAGE_INITIALIZED 1
#include <fstream>
#include <string>
#include <stdio.h>
#include <list>
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
    int portHandle;
    void startUpdateProcess();
    std::list<HexRecord> dataFromFile;

    protected:
    
    void parseLineOfHex(char * lineFromFile);
    int checkChecksum(char * lineFromFile);
    void getDataRecord(char * lineFromFile, int lengthOfData);
    void initTransferOfFirmware();
    int writeDataToSerialPort(const void *ptr, int len);
    
    private:
    
    FILE *_file;
    int currentIndexInsidePage;
    int currentAddress;
    HexRecord * newHexRecord;
    

};
}
#endif /* BYBBootloaderController_hpp */
