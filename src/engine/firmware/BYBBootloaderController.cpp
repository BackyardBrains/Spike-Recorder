//
//  BYBBootloaderController.cpp
//  SpikeRecorder
//
//  Created by Stanislav on 01/04/2022.
//  Copyright © 2022 BackyardBrains. All rights reserved.
//
//  Made to update STM32 microcontrollers. Works with custom bootloader that BYB made.
//  It is used for devices like Human SpikerBox, Neuron SpikerBox (Mfi) and Spike Station 
//  Update goes like this
//  ArduinoSerial calls Recording Manager startBootloaderProcess() if bootloaderPort is found 
//  On Windows bootloaderPort is searched in registry by VID and PID
//                        VID "VID_2E73";
//                        PID Human: "PID_0005";
//                        PID Neuron: "PID_000A";
//  On Mac bootloaderPort is filtered by USB device name:
//                        Neuron   "Neuron SB Bootloader"
//                        Human    "STM32L4_Boot"    
// Therefore every time we add new bootloader we have to add USB Name (for MacOS) and VID/PID (for Windows) to the app


#include "BYBBootloaderController.h"
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include "stdint.h"
//#include <stdint.h>
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

    bool BYBBootloaderController::isFirmwareAvailable()
    {
        if (FILE *file = fopen(firmwarePath.c_str(), "r"))
        {
            fclose(file);
            return true;
        }
        else
        {
            return false;
        }
    }


    int getline2(char ** lineptr, size_t * n, FILE * stream)
    {

            size_t count;
            const int maxLineSize  = 256;
            *lineptr = (char *)malloc(sizeof(char) *maxLineSize);
            int c = 0;
            for (count = 0; count <maxLineSize-1; count++) 
            {
                c = getc(stream);

                if (c == EOF ) 
                {
                    if (count == 0)
                    {
                        return -1;
                    }
                    
                    break;
                }
                
                (*lineptr)[count] = (char) c;
                if( c=='\n' || c=='\r')
                {
                    break;
                }
            }
            count++;
            (*lineptr)[count] = '\0';
            *n = count;
            return (ssize_t) count;
    }

    void BYBBootloaderController::startUpdateProcess()
    {
        stage = BOOTLOADER_STAGE_INITIALIZED;
        dataFromFile.clear();
        currentAddress = 0;
        progress = 1;
        //load HEX file into memory
        _file = fopen(firmwarePath.c_str(), "r");
        if(_file == 0) {
            stage = BOOTLOADER_STAGE_OFF;
            return;
        }
        
        currentIndexInsidePage = 0;
        char *contents = NULL;
        size_t len = 0;
        #if defined(__APPLE__)
            while (getline(&contents, &len, _file) != -1)
        {
        #elif _WIN32
         while (getline2(&contents, &len, _file) != -1)
        {
        #endif
            //printf("%s", contents);
            parseLineOfHex(contents);
            //free(contents);

        }


        initTransferOfFirmware();
        stage = BOOTLOADER_STAGE_OFF;
    }

    void BYBBootloaderController::initTransferOfFirmware()
    {
        char tempBuffer[256];
        int readLength = 0;
        printf("Waiting for serial to send g\n");
        while((readLength = readDataFromSerialPort(tempBuffer))<=0)
        {}
        if(tempBuffer[0]=='g')
        {
            printf("Received g\n");
        }
        else
        {
            printf("Bootloader error. Received %c instead of g\n", tempBuffer[0]);
        }
       
       
        while(1)
        {
            tempBuffer[0] = 'n';
            writeDataToSerialPort(tempBuffer, 1);
            while((readLength = readDataFromSerialPort(tempBuffer))<=0)
            {}
            if(tempBuffer[0]=='n')
            {
                printf("Received n\n");
                break;
            }
            else
            {
                printf("Bootloader error. Received %c instead of n\n", tempBuffer[0]);
            }
        }
        uint32_t programSize = (uint32_t)dataFromFile.size();
        tempBuffer[0] = programSize & 0x000000FF;
        tempBuffer[1] = ((programSize & 0x0000FF00) >> 8);
        tempBuffer[2] = ((programSize & 0x00FF0000) >> 16);
        tempBuffer[3] = ((programSize & 0xFF000000) >> 24);
        writeDataToSerialPort(tempBuffer, 4);
        
        //upload firmware
        int localProgress = 0;
        std::list<HexRecord>::iterator it;
        it = dataFromFile.begin();
        while(1)//it != dataFromFile.end()
        {
            while((readLength = readDataFromSerialPort(tempBuffer))<=0)
            {}
            if(tempBuffer[0]=='e')
            {
                printf("Programming finished");
                progress = 100;
                break;
            }
            else if(tempBuffer[0]=='x')
            {
                printf("Pages %d out of %d\n", localProgress, programSize);
                
                int tempProgress = (int)100.0f*((float)localProgress/(float)programSize);
                if(tempProgress>99)
                {
                    tempProgress =99;
                }
                progress = tempProgress;

                writeDataToSerialPort(it->dataPage, 8);
            }
            tempBuffer[0]=' ';
            it++;
            localProgress++;
        }
        progress = 100;
    }


    int BYBBootloaderController::readDataFromSerialPort(char * buffer)
    {
        ssize_t size = -1;
        #if defined(__APPLE__) || defined(__linux__)
            // Initialize file descriptor sets
            fd_set read_fds, write_fds, except_fds;
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);
            FD_ZERO(&except_fds);
            FD_SET(portHandle, &read_fds);
            // Set timeout to 60 ms
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 60000;
            if (select(portHandle + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1)
            {
                size = read(portHandle, buffer, 32768 );
            }
            if (size < 0)
            {
                if(errno == EAGAIN)
                {
                    return -1;
                }
                if(errno == EINTR)
                {
                    return -1;
                }
            }
        #endif // defined
        #if defined(__linux__)
            int bits;
            if (size == 0 && ioctl(fd, TIOCMGET, &bits) < 0)
            {
                //error
            }
        #endif
        #if defined(_WIN32)
            COMSTAT st;
            DWORD errmask=0, num_read, num_request;
            OVERLAPPED ov;
            if (!ClearCommError(portHandle, &errmask, &st))
            {
                return -1;
            }
            unsigned long numInCueue = st.cbInQue;
            num_request = 10000;
            ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (ov.hEvent == NULL) return -1;
            ov.Internal = ov.InternalHigh = 0;
            ov.Offset = ov.OffsetHigh = 0;
            if (ReadFile(portHandle, buffer, num_request, &num_read, &ov)) {
                size = num_read;
            } else {
                if (GetLastError() == ERROR_IO_PENDING) {
                    if (GetOverlappedResult(portHandle, &ov, &num_read, TRUE)) {
                        size = num_read;
                    } else {
                        size = -1;
                    }
                } else {
                    size = -1;
                }
            }
            CloseHandle(ov.hEvent);
            
        #endif // defined

        return (int)size;
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
                printf("bootloader write to port: %d bytes\n", (int)written);
                return (int)written;
        #elif defined(_WIN32)
            int r;
            DWORD num_written;
            OVERLAPPED ov;

            ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (ov.hEvent == NULL) return -1;
            ov.Internal = ov.InternalHigh = 0;
            ov.Offset = ov.OffsetHigh = 0;

            if (WriteFile(portHandle, ptr, len, &num_written, &ov))
            {
                r = num_written;
            }
            else
            {
                if (GetLastError() == ERROR_IO_PENDING) {
                    if (GetOverlappedResult(portHandle, &ov, &num_written, TRUE)) {
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
        int tempCurrentAddress;
        sscanf(&lineFromFile[3],"%4x",&tempCurrentAddress);
        
        //fill the void in memory (if hex jums over some memory)
        if(currentAddress!=0 && tempCurrentAddress!=currentAddress && tempCurrentAddress>currentAddress)
        {
           
                int difference =tempCurrentAddress-currentAddress;
                
                for (int i=0;i<difference;i++)
                {
                    
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
                    newHexRecord->dataPage[currentIndexInsidePage]=255;
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
        else
        {
            currentAddress =tempCurrentAddress;
        }
        
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
        return 0;
    }
}
