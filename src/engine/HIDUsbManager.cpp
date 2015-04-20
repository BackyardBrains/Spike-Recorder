#include "HIDUsbManager.h"
#include <sstream>


#define BYB_VID 0x2047
#define BYB_PID 0x3e0

#define SIZE_OF_MAIN_CIRCULAR_BUFFER 3000

namespace BackyardBrains {
    HIDUsbManager::HIDUsbManager()
    {
        _deviceConnected = false;

    }

    int HIDUsbManager::openDevice()
    {
        std::stringstream sstm;//variable for log
        handle = hid_open(BYB_VID, BYB_PID, NULL);
        if (!handle) {
            sstm << "unable to open HID device.";
            errorString = sstm.str();
            std::cout<<"unable to open HID device.\n";
            return -1;
        }

        std::cout<<"Success. HID device connected\n";

        circularBuffer[0] = '\n';

        cBufHead = 0;
        cBufTail = 0;

        serialCounter = 0;

        _deviceConnected = true;

        setNumberOfChannelsAndSamplingRate(1, maxSamplingRate());
        gettimeofday(&start, NULL);
        
        t1 = std::thread(&HIDUsbManager::readThread, this, this);
        t1.detach();
        return 0;
    }

    void HIDUsbManager::readThread(HIDUsbManager * ref)
    {
        ref->mainHead = 0;
        ref->mainTail = 0;
        ref->mainCircularBuffer = new int16_t[ref->numberOfChannels()*SIZE_OF_MAIN_CIRCULAR_BUFFER];
        int maxSamples = ref->numberOfChannels()*SIZE_OF_MAIN_CIRCULAR_BUFFER;
        int16_t *buffer = new int16_t[256];
        int numberOfFrames;
       // int k = 0;
        while (ref->_deviceConnected) {
            numberOfFrames = ref->readOneBatch(buffer);
            for(int i=0;i<numberOfFrames;i++)
            {
                for(int j=0;j<ref->numberOfChannels();j++)
                {
                    ref->mainCircularBuffer[ref->mainHead++] = buffer[i*ref->numberOfChannels()+j];
                    if(mainHead>=maxSamples)
                    {
                        mainHead = 0;
                    }
                }
            }
        }
        numberOfFrames = 0;
    }
    
    
    int HIDUsbManager::readOneBatch(int16_t * obuffer)
    {
        
        //------------- debug code ------
        /*long mtime, seconds, useconds;
        gettimeofday(&end, NULL);
        
        seconds  = end.tv_sec  - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;
        
        mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
        
        printf("Elapsed time: %ld milliseconds\n", mtime);
        start = end;
        //--------------- end of debug code -------

        */
        
        unsigned char buffer[256];
        
        int writeInteger = 0;
        int obufferIndex = 0;
        int numberOfFrames = 0;
        int size = -1;
        
        //while (1) {
        size = hid_read(handle, buffer, sizeof(buffer));
        if (size == 0)
        {
            std::cout<<"No HID data\n";
            return 0;
        }
        if (size < 0)
        {
            std::cout<<"Error HID: Unable to read\n";
            return -1;
        }
        if(size<3)
        {
            //printf("%.*s",res,buf);
            //std::cout<<buffer;
            return-1;
        }
        //}
        //std::cout<<"Size: "<<size<<"\n";
        
        for(int i=2;i<size;i++)
        {
            circularBuffer[cBufHead++] = buffer[i];
            //uint MSB  = ((uint)(buffer[i])) & 0xFF;
            //std::cout<<"M: " << MSB<<"\n";
            if(cBufHead>=SIZE_OF_CIRC_BUFFER)
            {
                cBufHead = 0;
            }
        }
        
        unsigned int LSB;
        unsigned int MSB;
        
        bool haveData = true;
        while (haveData)
        {
            
            MSB  = ((unsigned int)(circularBuffer[cBufTail])) & 0xFF;
            if(MSB > 127)//if we are at the begining of frame
            {
                if(checkIfHaveWholeFrame())
                {
                    // std::cout<<"Number of frames: "<< numberOfFrames<<"\n";
                    numberOfFrames++;
                    while (1)
                    {
                        //make sample value from two consecutive bytes
                        // std::cout<<"Tail: "<<cBufTail<<"\n";
                        //  MSB  = ((uint)(circularBuffer[cBufTail])) & 0xFF;
                        //std::cout<< cBufTail<<" -M "<<MSB<<"\n";
                        MSB  = ((unsigned int)(circularBuffer[cBufTail])) & 0x7F;
                        
                        cBufTail++;
                        if(cBufTail>=SIZE_OF_CIRC_BUFFER)
                        {
                            cBufTail = 0;
                        }
                        LSB  = ((unsigned int)(circularBuffer[cBufTail])) & 0xFF;
                        //if we have error in frame (lost data)
                        if(LSB>127)
                        {
                            numberOfFrames--;
                            break;//continue as if we have new frame
                        }
                        // std::cout<< cBufTail<<" -L "<<LSB<<"\n";
                        LSB  = ((unsigned int)(circularBuffer[cBufTail])) & 0x7F;
                        
                        MSB = MSB<<7;
                        writeInteger = LSB | MSB;
                        
                        //std::cout<< obufferIndex<<" - "<<MSB<<":"<<LSB<<"\n";
                        obuffer[obufferIndex++] = writeInteger;
                        if(areWeAtTheEndOfFrame())
                        {
                            break;
                        }
                        else
                        {
                            cBufTail++;
                            if(cBufTail>=SIZE_OF_CIRC_BUFFER)
                            {
                                cBufTail = 0;
                            }
                        }
                    }
                }
                else
                {
                    haveData = false;
                    break;
                }
            }
            if(!haveData)
            {
                break;
            }
            cBufTail++;
            if(cBufTail>=SIZE_OF_CIRC_BUFFER)
            {
                cBufTail = 0;
            }
            if(cBufTail==cBufHead)
            {
                haveData = false;
                break;
            }
        }
        return numberOfFrames;
    }
    
    int HIDUsbManager::readDevice(int16_t * obuffer)
    {
        int frames;
        int maxNumOfSamples = _numberOfChannels*SIZE_OF_MAIN_CIRCULAR_BUFFER;
        int tempMainHead = mainHead;//keep head position because input thread will move it.
        
        //std::cout<<mainHead<<" - "<<mainTail;
       if(mainTail>tempMainHead)
       {
           memcpy ( obuffer, &mainCircularBuffer[mainTail], sizeof(int16_t)*(maxNumOfSamples-mainTail));
           memcpy ( &obuffer[maxNumOfSamples-mainTail], mainCircularBuffer, sizeof(int16_t)*(tempMainHead));
           frames = ((maxNumOfSamples-mainTail)+tempMainHead)/_numberOfChannels;
           
       }
       else
       {
           memcpy ( obuffer, &mainCircularBuffer[mainTail], sizeof(int16_t)*(tempMainHead-mainTail));
           frames = (tempMainHead-mainTail)/_numberOfChannels;
       }
        
        mainTail = tempMainHead;

        return frames;
       /* unsigned char buffer[256];

        int writeInteger = 0;
        int obufferIndex = 0;
        int numberOfFrames = 0;
        int size = -1;

        //while (1) {
            size = hid_read(handle, buffer, sizeof(buffer));
            if (size == 0)
            {
                std::cout<<"No HID data\n";
                return 0;
            }
            if (size < 0)
            {
                std::cout<<"Error HID: Unable to read\n";
                return -1;
            }
            if(size<3)
            {
                //printf("%.*s",res,buf);
                //std::cout<<buffer;
                return-1;
            }
        //}
        std::cout<<"Size: "<<size<<"\n";

        for(int i=2;i<size;i++)
        {
            circularBuffer[cBufHead++] = buffer[i];
            //uint MSB  = ((uint)(buffer[i])) & 0xFF;
            //std::cout<<"M: " << MSB<<"\n";
            if(cBufHead>=SIZE_OF_CIRC_BUFFER)
            {
                cBufHead = 0;
            }
        }

        unsigned int LSB;
        unsigned int MSB;

        bool haveData = true;
        while (haveData)
        {

            MSB  = ((unsigned int)(circularBuffer[cBufTail])) & 0xFF;
            if(MSB > 127)//if we are at the begining of frame
            {
                if(checkIfHaveWholeFrame())
                {
                   // std::cout<<"Number of frames: "<< numberOfFrames<<"\n";
                    numberOfFrames++;
                    while (1)
                    {
                        //make sample value from two consecutive bytes
                       // std::cout<<"Tail: "<<cBufTail<<"\n";
                       //  MSB  = ((uint)(circularBuffer[cBufTail])) & 0xFF;
                        //std::cout<< cBufTail<<" -M "<<MSB<<"\n";
                        MSB  = ((unsigned int)(circularBuffer[cBufTail])) & 0x7F;

                        cBufTail++;
                        if(cBufTail>=SIZE_OF_CIRC_BUFFER)
                        {
                            cBufTail = 0;
                        }
                        LSB  = ((unsigned int)(circularBuffer[cBufTail])) & 0xFF;
                        //if we have error in frame (lost data)
                        if(LSB>127)
                        {
                            numberOfFrames--;
                            break;//continue as if we have new frame
                        }
                       // std::cout<< cBufTail<<" -L "<<LSB<<"\n";
                        LSB  = ((unsigned int)(circularBuffer[cBufTail])) & 0x7F;

                        MSB = MSB<<7;
                        writeInteger = LSB | MSB;

                        //std::cout<< obufferIndex<<" - "<<MSB<<":"<<LSB<<"\n";
                        obuffer[obufferIndex++] = writeInteger;
                        if(areWeAtTheEndOfFrame())
                        {
                            break;
                        }
                        else
                        {
                            cBufTail++;
                            if(cBufTail>=SIZE_OF_CIRC_BUFFER)
                            {
                                cBufTail = 0;
                            }
                        }
                    }
                }
                else
                {
                    haveData = false;
                    break;
                }
            }
            if(!haveData)
            {
                break;
            }
            cBufTail++;
            if(cBufTail>=SIZE_OF_CIRC_BUFFER)
            {
                cBufTail = 0;
            }
            if(cBufTail==cBufHead)
            {
                haveData = false;
                break;
            }
        }
        return numberOfFrames;*/
    }


    void HIDUsbManager::getAllDevicesList()
    {
        list.clear();
        struct hid_device_info *devs, *cur_dev;
        std::cout<<"Scan for HID devices... \n";
        devs = hid_enumerate(0x0, 0x0);
        cur_dev = devs;
        while (cur_dev) {
                std::string nameOfHID((char *) cur_dev->product_string);
            list.push_back(nameOfHID);
            std::cout<<"HID device: "<<cur_dev->manufacturer_string<<", "<<cur_dev->product_string<<"\n";
           /* printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
            printf("\n");
            printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
            printf("  Product:      %ls\n", cur_dev->product_string);
            printf("  Release:      %hx\n", cur_dev->release_number);
            printf("  Interface:    %d\n",  cur_dev->interface_number);
            printf("\n");*/
            cur_dev = cur_dev->next;
        }
        hid_free_enumeration(devs);
    }


    void HIDUsbManager::closeDevice()
    {
        if(handle)
        {
            hid_close(handle);
            handle = NULL;
            _deviceConnected = false;
        }
    }


    void HIDUsbManager::setNumberOfChannelsAndSamplingRate(int numberOfChannels, int samplingRate)
    {
        _numberOfChannels = numberOfChannels;
        _samplingRate = samplingRate;
        std::cout<<"HID - Set number of channels:"<<numberOfChannels<<" and sampling rate: "<<samplingRate<<"\n";
        std::stringstream sstm;
        sstm << "conf s:" << samplingRate<<";c:"<<numberOfChannels<<";\n";
        writeToDevice(sstm.str().c_str(),sstm.str().length());
    }
    int HIDUsbManager::writeToDevice(const void *ptr, int len)
    {
        //TODO: write sending commands
        return 0;
    }
    int HIDUsbManager::maxSamplingRate()
    {
        return 10000;
    }

    int HIDUsbManager::maxNumberOfChannels()
    {
        return 3;
    }

    int HIDUsbManager::numberOfChannels()
    {
        return _numberOfChannels;
    }

    const char * HIDUsbManager::currentDeviceName()
    {
        return "Spike Recorder dummy";
    }

    bool HIDUsbManager::deviceOpened()
    {
        return _deviceConnected;
    }

    bool HIDUsbManager::checkIfNextByteExist()
    {
        int tempTail = cBufTail + 1;
        if(tempTail>= SIZE_OF_CIRC_BUFFER)
        {
            tempTail = 0;
        }
        if(tempTail==cBufHead)
        {
            return false;
        }
        return true;
    }

    bool HIDUsbManager::checkIfHaveWholeFrame()
    {
        int tempTail = cBufTail + 1;
        if(tempTail>= SIZE_OF_CIRC_BUFFER)
        {
            tempTail = 0;
        }
        while(tempTail!=cBufHead)
        {
            unsigned int nextByte  = ((unsigned int)(circularBuffer[tempTail])) & 0xFF;
            if(nextByte > 127)
            {
                return true;
            }
            tempTail++;
            if(tempTail>= SIZE_OF_CIRC_BUFFER)
            {
                tempTail = 0;
            }
        }
        return false;
    }

    bool HIDUsbManager::areWeAtTheEndOfFrame()
    {
        int tempTail = cBufTail + 1;
        if(tempTail>= SIZE_OF_CIRC_BUFFER)
        {
            tempTail = 0;
        }
        unsigned int nextByte  = ((unsigned int)(circularBuffer[tempTail])) & 0xFF;
        if(nextByte > 127)
        {
            return true;
        }
        return false;
    }

}
