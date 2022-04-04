//
//  BYBBootloaderController.cpp
//  SpikeRecorder
//
//  Created by Stanislav on 01/04/2022.
//  Copyright © 2022 BackyardBrains. All rights reserved.
//

#include "BYBBootloaderController.h"

#if defined(__APPLE__)
//added for port scan
    #include <sys/uio.h>
    #include <stdio.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <errno.h>
    #include <termios.h>
    #include <sysexits.h>
    #include <sys/param.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <time.h>
#elif _WIN32
    #include <initguid.h>
    #include <devguid.h>
    #include <setupapi.h>
    #include <string>
    #include <locale>
    #include <algorithm>
    #include <windows.h>
    typedef unsigned int uint;
    #include <cstdio>
#endif

namespace BackyardBrains {

    BYBBootloaderController::BYBBootloaderController()
    {
        stage = BOOTLOADER_STAGE_OFF;
  
    }

    void BYBBootloaderController::startUpdateProcess()
    {
        stage = BOOTLOADER_STAGE_INITIALIZED;
        
        //load HEX file into memory
        _file = fopen(firmwarePath.c_str(), "r");
        if(_file == 0) {
            stage = BOOTLOADER_STAGE_OFF;
            return;
        }
        
        currentIndexInsidePage = 0;
        char *contents = NULL;
        size_t len = 0;
        while (getline(&contents, &len, _file) != -1){
            printf("%s", contents);
            parseLineOfHex(contents);
        }

        initTransferOfFirmware();
    }

    void BYBBootloaderController::initTransferOfFirmware()
    {
        
        
        
    }


    int BYBBootloaderController::writeDataToSerialPort(const void *ptr, int len)
    {

        #if defined(__APPLE__) || defined(__linux__)
                long n, written=0;
                fd_set wfds;
                struct timeval tv;
                while (written < len) {
                    n = write(portHandle, (const char *)ptr + written, len - written);
                    if (n < 0 && (errno == EAGAIN || errno == EINTR))
                    {
                       n = 0;
                    }
                    if (n < 0)
                    {
                      return -1;
                    }
                    if (n > 0) {
                        written += n;
                    } else {
                        tv.tv_sec = 10;
                        tv.tv_usec = 0;
                        FD_ZERO(&wfds);
                        FD_SET(portHandle, &wfds);
                        n = select(portHandle+1, NULL, &wfds, NULL, &tv);
                        if (n < 0 && errno == EINTR) n = 1;
                        if (n <= 0) return -1;
                    }
                }
                printf("bootloader write to port: %d bytes", (int)written);
                return (int)written;
        #elif defined(_WIN32)
            DWORD num_written;
            OVERLAPPED ov;
            int r;
            ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (ov.hEvent == NULL) return -1;
            ov.Internal = ov.InternalHigh = 0;
            ov.Offset = ov.OffsetHigh = 0;

            if (WriteFile(port_handle, ptr, len, &num_written, &ov))
            {
                r = num_written;
            }
            else
            {
                if (GetLastError() == ERROR_IO_PENDING) {
                    if (GetOverlappedResult(port_handle, &ov, &num_written, TRUE)) {
                        r = num_written;
                    } else {
                        r = -1;
                    }
                } else {
                    r = -1;
                }
            };
            CloseHandle(ov.hEvent);
            return r;
        #endif
    }



    void BYBBootloaderController::parseLineOfHex(char * lineFromFile)
    {
        //:02 0000 04 0800 F2
        checkChecksum(lineFromFile);
        int lengthOfData;
        sscanf(&lineFromFile[1],"%2x",&lengthOfData);
        
        sscanf(&lineFromFile[3],"%4x",&currentAddress);
        int typeOfRecord;
        sscanf(&lineFromFile[7],"%2x",&typeOfRecord);
        switch (typeOfRecord) {
            case 0:
                //data record
                getDataRecord(lineFromFile, lengthOfData);
                break;
            case 1:
                //end of file record
                break;
            case 2:
                //Extended Segment Address Record
                break;
            case 3:
                //Start Segment Address Record: Segment address STM32 is not used
                break;
            case 4:
                //Extended Linear Address Record: Used to identify the extended linear address
                break;
            case 5:
                //Start Linear Address Record: The address where the program starts and runs
                break;
            default:
                break;
        }
    }

//
// Get data pages from one line (usualy 2 pages = 16 bytes in one line)
//
    void BYBBootloaderController::getDataRecord(char * lineFromFile, int lengthOfData)
    {
        for(int recordIndex = 0;recordIndex<lengthOfData;recordIndex++)
        {
            //parse one page (8 bytes - 16 ascii characters )
            if(currentIndexInsidePage==0)
            {
                newHexRecord = new HexRecord();
                //clear memory
                for(int i=0;i<SIZE_OF_PAGE;i++)
                {
                    newHexRecord->dataPage[i] = 0;
                }
                //address where data should be put in memory on microcontroller
                newHexRecord->address =currentAddress;
            }

            uint tempByte;
            int indexInHexText =9+recordIndex*2;
            sscanf(&lineFromFile[indexInHexText],"%2x",&tempByte);
            newHexRecord->dataPage[currentIndexInsidePage]=tempByte;
            currentAddress++;
            
            currentIndexInsidePage++;
            if(currentIndexInsidePage==SIZE_OF_PAGE)
            {
                currentIndexInsidePage = 0;
                //add one page to memory list
                dataFromFile.push_back(*newHexRecord);
            }
        }
    }


    int BYBBootloaderController::checkChecksum(char * lineFromFile)
    {
        if (lineFromFile[0] != ':')
        {
            printf("Bootloader Error: line does not start with :\n" );
        }
        

        //get length of data
        int tempIntData;
        //sscanf(&lineFromFile[1],"%2x",&lengthOfData);
        int i=1;
        uint8_t checksum = 0;
        while(lineFromFile[i]!='\n' && lineFromFile[i]!='\r' )
        {
            sscanf(&lineFromFile[i],"%2x",&tempIntData);
            checksum +=tempIntData;
            i+=2;
        }
        if(checksum!=0)
        {
            printf("Bootloader Error: checksum error\n" );
            return 1;
        }
        /*
        for (int i = 0; i < n; i++)
        {
            checksum += byte.Parse(new string(data.ToCharArray(1 + i * 2, 2)), System.Globalization.NumberStyles.HexNumber);
        }
        if (checksum != 0)
        {
            throw new Exception("Checksum error.");
        }*/
        return 0;
    }
}
