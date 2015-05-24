#include "HIDUsbManager.h"
#include <sstream>


#define BYB_VID 0x2047
#define BYB_PID 0x3e0

#define SIZE_OF_MAIN_CIRCULAR_BUFFER 10000

namespace BackyardBrains {
    
    
    //
    // Constructor of HID USB manager
    //
    HIDUsbManager::HIDUsbManager()
    {
        _deviceConnected = false;

    }

    //
    // Open USB connection to BYB device based on PID and VID
    //
    int HIDUsbManager::openDevice()
    {
        std::stringstream sstm;//variable for log
        handle = hid_open(BYB_VID, BYB_PID, NULL);
        if (!handle) {
             sstm << "Unable to open HID USB device. Please plug in the BackyardBrains USB device and try again.";
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

        //set number of channels and sampling rate on micro
        setNumberOfChannelsAndSamplingRate(2, maxSamplingRate());
        //send start command to micro
        startDevice();
        
        //start thread that will periodicaly read
        t1 = std::thread(&HIDUsbManager::readThread, this, this);
        t1.detach();
        return 0;
    }


    //
    //Thread that periodicaly read new data from microcontroller
    //read must be executed at least 1000 times per second.
    // Thread stops and close connection to microcontroller
    // when _deviceConnected is FALSE
    //
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
            //std::cout<<numberOfFrames<<"-";
            for(int i=0;i<numberOfFrames;i++)
            {
                //we copy here head position since we dont want to cut the frame
                //in half in reading thread. (thread race problem)
                int indexOfHead=ref->mainHead;
                
                for(int j=0;j<ref->numberOfChannels();j++)
                {
                    ref->mainCircularBuffer[indexOfHead++] = buffer[i*ref->numberOfChannels()+j];
                    
                    if(indexOfHead>=maxSamples)
                    {
                        indexOfHead = 0;
                    }
                }
                //update head position after writting walue
                //so that we always have shole frame when reading
                ref->mainHead = indexOfHead;
            }
        }
        //realy disconnect from device here
        if(!_deviceConnected)
        {
            ref->stopDevice();
            hid_close(ref->handle);
            ref->handle = NULL;
        }
        numberOfFrames = 0;
    }


    //
    // Read one batch of data from HID usb
    //
    int HIDUsbManager::readOneBatch(int16_t * obuffer)
    {
        unsigned char buffer[256];

        int writeInteger = 0;
        int obufferIndex = 0;
        int numberOfFrames = 0;
        int size = -1;


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
            return-1;
        }
        //get number of bytes
        unsigned int sizeOfPackage =((unsigned int)buffer[1]& 0xFF);

        for(int i=2;i<sizeOfPackage+2;i++)
        {
            circularBuffer[cBufHead++] = buffer[i];

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
                    for(int channelind=0;channelind<_numberOfChannels;channelind++)
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

                        //write decoded integer to buffer
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

    //
    //  Read newly arrived data from circular buffer.
    // And return number of readed frames
    // (not samples, one frame can contains multiple interleaved samples)
    //
    int HIDUsbManager::readDevice(int16_t * obuffer)
    {
        int frames;
        int maxNumOfSamples = _numberOfChannels*SIZE_OF_MAIN_CIRCULAR_BUFFER;
        int tempMainHead = mainHead;//keep head position because input thread will move it.

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
    }


    //
    // Get list of all HID devices attached to computer
    //
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

    //
    // Close connection with HID device
    // This close connection just logicaly (puts flag _deviceConnected to false)
    // Actual connection is closed later in fatching thread since we
    // don't want to close connection while fetching thread is fatching data
    // that will lead to crash
    //
    void HIDUsbManager::closeDevice()
    {
        if(handle && _deviceConnected)
        {
            _deviceConnected = false;
            //hid_close(handle);
            //handle = NULL;
            //_deviceConnected = false;
        }
    }

    //
    //Send start command to microcontroller: "start:;"
    //It starts sampling and sending data
    //
    void HIDUsbManager::startDevice()
    {
        std::stringstream sstm;
        sstm << "start:"<<";\n";
        writeToDevice((unsigned char*)(sstm.str().c_str()),sstm.str().length());
    }
    
    //
    // Send stop command to microcontroller "h:;"
    //
    void HIDUsbManager::stopDevice()
    {
        std::stringstream sstm;
        sstm << "h:"<<";\n";
        writeToDevice((unsigned char*)(sstm.str().c_str()),sstm.str().length());
    }
    
    //
    // Ask microcontroller for it's capabilities
    // (sampling rate, number of channels, firmware version)
    //
    void HIDUsbManager::askForCapabilities()
    {
        std::stringstream sstm;
        sstm << "?:"<<";\n";
        writeToDevice((unsigned char*)(sstm.str().c_str()),sstm.str().length());
    }

    //
    // Sends command for seting number of channels and sampling rate on micro.
    //
    void HIDUsbManager::setNumberOfChannelsAndSamplingRate(int numberOfChannels, int samplingRate)
    {
        _numberOfChannels = numberOfChannels;
        _samplingRate = samplingRate;
        std::cout<<"HID - Set number of channels:"<<numberOfChannels<<" and sampling rate: "<<samplingRate<<"\n";
        std::stringstream sstm;
        sstm << "conf s:" << samplingRate<<";c:"<<numberOfChannels<<";\n";
        writeToDevice((unsigned char*)(sstm.str().c_str()),sstm.str().length());
    }

    //
    // Send string to device. Currently it supports strings up to 62 characters
    //
    int HIDUsbManager::writeToDevice(const unsigned char *ptr, size_t len)
    {
        unsigned char outbuff[64];
        outbuff[0] = 0x3f;
        outbuff[1] = 62;
        for(size_t i=0;i<len;i++)
        {
            outbuff[i+2] = ptr[i];
        }
        int res = hid_write(handle, outbuff, 64);
        if (res < 0) {
            std::stringstream sstm;//variable for log
            sstm << "Could not write to device. Error reported was: " << hid_error(handle);
            errorString = sstm.str();
            std::cout<<"Error HID write: \n"<<sstm.str();
        }
        return 0;
    }

    //
    //Max sampling rate of microcontroller
    //
    int HIDUsbManager::maxSamplingRate()
    {
        return 10000;
    }

    //
    // Max number of channels that microcontroller supports
    //
    int HIDUsbManager::maxNumberOfChannels()
    {
        return 2;
    }

    //
    // Current number of channels on HID device
    //
    int HIDUsbManager::numberOfChannels()
    {
        return _numberOfChannels;
    }

    //
    // Selected device name
    //
    const char * HIDUsbManager::currentDeviceName()
    {
        return "Spike Recorder USB HID";
    }

    //
    //Flag that is true when HID device is connected
    //it does not mean that it is sampling data
    //
    bool HIDUsbManager::deviceOpened()
    {
        return _deviceConnected;
    }

    //
    //Helper function
    //Check if we have more data in circular buffer
    //
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

    //
    //Check if we have at least one whole frame in circular buffer
    // It searches for additional start-of-frame flag after current tail
    // position. If it finds it than current frame is complete.
    //
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
    
    //
    //Check if newxt byte contains start-of-frame flag (8th bit set to 1)
    //
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
