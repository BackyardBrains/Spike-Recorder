#include "HIDUsbManager.h"
#include <sstream>
#include "RecordingManager.h"

#define BYB_VID 0x2047
#define BYB_PID 0x3e0

#define SIZE_OF_MAIN_CIRCULAR_BUFFER 40000

#define BOARD_WITH_ADDITIONAL_INPUTS 1

namespace BackyardBrains {


    //
    // Constructor of HID USB manager
    //
    HIDUsbManager::HIDUsbManager()
    {

        _deviceConnected = false;

        escapeSequence[0] = 255;
        escapeSequence[1] = 255;
        escapeSequence[2] = 1;
        escapeSequence[3] = 1;
        escapeSequence[4] = 128;
        escapeSequence[5] = 255;

        endOfescapeSequence[0] = 255;
        endOfescapeSequence[1] = 255;
        endOfescapeSequence[2] = 1;
        endOfescapeSequence[3] = 1;
        endOfescapeSequence[4] = 129;
        endOfescapeSequence[5] = 255;

        firmwareVersion = "";
        hardwareVersion = "";
        hardwareType = "";
        _samplingRate = 10000;
        _numberOfChannels = 2;
        restartDevice = false;
        handle = NULL;

    }




    //
    // Open USB connection to BYB device based on PID and VID
    //
    int HIDUsbManager::openDevice(RecordingManager * managerin)
    {
        _manager = managerin;
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

        escapeSequenceDetectorIndex = 0;
        weAreInsideEscapeSequence = false;
        messageBufferIndex =0;
        currentAddOnBoard = 0;
        _deviceConnected = true;

        askForCapabilities();//ask for firmware version etc.
        askForMaximumRatings(); //ask for sample rate and number of channels

        //askForRTRepeat();//ask if RT board is repeating stimmulation
        //set number of channels and sampling rate on micro (this will not work with firmware V0.1)
        setNumberOfChannelsAndSamplingRate(2, maxSamplingRate());
        //send start command to micro
        startDevice();

        //start thread that will periodicaly read HID
        t1 = std::thread(&HIDUsbManager::readThread, this, this);
        t1.detach();

        askForBoard();//ask if any board is connected



        return 0;
    }

    //
    // Detect start-of-message escape sequence and end-of-message sequence
    // and set up weAreInsideEscapeSequence.
    // When we detect end-of-message sequence call executeContentOfMessageBuffer()
    //
    void HIDUsbManager::testEscapeSequence(unsigned int newByte, int offset)
    {



        if(weAreInsideEscapeSequence)
        {

            if(messageBufferIndex>=SIZE_OF_MESSAGES_BUFFER)
            {
                weAreInsideEscapeSequence = false; //end of escape sequence
                executeContentOfMessageBuffer(offset+tempHeadAndTailDifference);
                escapeSequenceDetectorIndex = 0;//prepare for detecting begining of sequence
            }
            else if(endOfescapeSequence[escapeSequenceDetectorIndex] == newByte)
            {
                escapeSequenceDetectorIndex++;
                if(escapeSequenceDetectorIndex ==  ESCAPE_SEQUENCE_LENGTH)
                {
                    weAreInsideEscapeSequence = false; //end of escape sequence
                    executeContentOfMessageBuffer(offset+tempHeadAndTailDifference);
                    escapeSequenceDetectorIndex = 0;//prepare for detecting begining of sequence
                }
            }
            else
            {
                escapeSequenceDetectorIndex = 0;
            }

        }
        else
        {
            if(escapeSequence[escapeSequenceDetectorIndex] == newByte)
            {
                escapeSequenceDetectorIndex++;
                if(escapeSequenceDetectorIndex ==  ESCAPE_SEQUENCE_LENGTH)
                {
                    weAreInsideEscapeSequence = true; //found escape sequence
                    for(int i=0;i<SIZE_OF_MESSAGES_BUFFER;i++)
                    {
                        messagesBuffer[i] = 0;
                    }
                    messageBufferIndex = 0;//prepare for receiving message
                    escapeSequenceDetectorIndex = 0;//prepare for detecting end of esc. sequence

                    //rewind writing head and effectively delete escape sequence from data
                    for(int i=0;i<ESCAPE_SEQUENCE_LENGTH;i++)
                    {
                        cBufHead--;
                        if(cBufHead<0)
                        {
                            cBufHead = SIZE_OF_CIRC_BUFFER-1;
                        }
                    }
                }
            }
            else
            {
                escapeSequenceDetectorIndex = 0;
            }
        }

    }

    //
    // Parse and check what we need to do with message that we received
    // from microcontroller
    //
    void HIDUsbManager::executeContentOfMessageBuffer(int offset)
    {
        bool stillProcessing = true;
        int currentPositionInString = 0;
        char message[SIZE_OF_MESSAGES_BUFFER];
        for(int i=0;i<SIZE_OF_MESSAGES_BUFFER;i++)
        {
            message[i] = 0;
        }
        int endOfMessage = 0;
        int startOfMessage = 0;



        while(stillProcessing)
        {
            std::cout<<"----- MB: "<< currentPositionInString<<"     :"<<messagesBuffer<<"\n";
            if(messagesBuffer[currentPositionInString]==';')
            {
               //we have message, parse it
                for(int k=0;k<endOfMessage-startOfMessage;k++)
                {
                    if(message[k]==':')
                    {

                        std::string typeOfMessage(message, k);
                        std::string valueOfMessage(message+k+1, (endOfMessage-startOfMessage)-k-1);
                        executeOneMessage(typeOfMessage, valueOfMessage, offset);
                        break;
                    }
                }
                startOfMessage = endOfMessage+1;
                currentPositionInString++;
                endOfMessage++;

            }
            else
            {
                message[currentPositionInString-startOfMessage] = messagesBuffer[currentPositionInString];
                currentPositionInString++;
                endOfMessage++;

            }

            if(currentPositionInString>=SIZE_OF_MESSAGES_BUFFER)
            {
                stillProcessing = false;
            }
        }

        //free(message);

    }

    int HIDUsbManager::powerRailIsState()
    {
        return _powerRailState;
    }

    void HIDUsbManager::executeOneMessage(std::string typeOfMessage, std::string valueOfMessage, int offsetin)
    {
        std::cout<<"\nMESSAGE: "<<typeOfMessage<<" - "<<valueOfMessage<<"\n";
        if(typeOfMessage == "FWV")
        {
            firmwareVersion = valueOfMessage;
        }
        if(typeOfMessage == "HWT")
        {
            hardwareType = valueOfMessage;
        }

        if(typeOfMessage == "HWV")
        {
            hardwareVersion = valueOfMessage;
        }

        if(typeOfMessage == "PWR")
        {
            _powerRailState = (int)((unsigned int)valueOfMessage[0]-48);
        }

        if(typeOfMessage == "EVNT")
        {
            int mnum = (int)((unsigned int)valueOfMessage[0]-48);
            int64_t offset = 0;
           /* if(!_manager.fileMode())
            {

                offset = _audioView->offset();
            }*/
            _manager->addMarker(std::string(1, mnum+'0'), offset+offsetin);

        }
        if(typeOfMessage == "BRD")
        {
            currentAddOnBoard = (int)((unsigned int)valueOfMessage[0]-48);
            if(currentAddOnBoard == BOARD_WITH_ADDITIONAL_INPUTS)
            {
                //if(_samplingRate != 7500)
                //{
                    _samplingRate = 5000;
                    _numberOfChannels  =4;
                    restartDevice = true;
                //}
            }
            else
            {
                if(_samplingRate != 10000)
                {
                    _samplingRate = 10000;
                    _numberOfChannels  =2;
                    restartDevice = true;
                }
            }
        }
        if(typeOfMessage == "RTR")
        {
            if(((int)((unsigned int)valueOfMessage[0]-48)) == 1)
            {
                _rtReapeating = true;
            }
            else
            {
                _rtReapeating = false;
            }
        }
        if(typeOfMessage == "MSF")
        {
           //TODO: implement maximum sample rate
        }
        if(typeOfMessage == "MNC")
        {
            //TODO: implement maximum number of channels
        }

    }

    int HIDUsbManager::addOnBoardPressent()
    {
        return currentAddOnBoard;
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
        ref->mainCircularBuffer = new int32_t[ref->numberOfChannels()*SIZE_OF_MAIN_CIRCULAR_BUFFER];
        ref->maxSamples = ref->numberOfChannels()*SIZE_OF_MAIN_CIRCULAR_BUFFER;
        int32_t *buffer = new int32_t[2256];
        int numberOfFrames;
       // int k = 0;

        tempHeadAndTailDifference-=SIZE_OF_MAIN_CIRCULAR_BUFFER;
        while (ref->_deviceConnected) {

            try{
                numberOfFrames = ref->readOneBatch(buffer);
            }
            catch(std::exception &e)
            {
                numberOfFrames = -1;
                std::cout<<"Error on read 1";

            }
            catch(...)
            {
                numberOfFrames = -1;
                std::cout<<"Error on read 2";
            }


            if(numberOfFrames == -1)
            {
                ref->_deviceConnected = false;
            }
            //std::cout<<numberOfFrames<<"-";
            for(int i=0;i<numberOfFrames;i++)
            {
                //we copy here head position since we dont want to cut the frame
                //in half in reading thread. (thread race problem)
                int indexOfHead=ref->mainHead;

                for(int j=0;j<ref->numberOfChannels();j++)
                {
                    ref->mainCircularBuffer[indexOfHead++] = buffer[i*ref->numberOfChannels()+j];

                    if(indexOfHead>=ref->maxSamples)
                    {
                        indexOfHead = 0;
                    }
                }
                //update head position after writting walue
                //so that we always have whole frame when reading
                ref->mainHead = indexOfHead;
                int tempMainHead2 = ref->mainHead;
                if(tempMainHead2>mainTail)
                {
                    tempHeadAndTailDifference =(tempMainHead2-mainTail)/ref->numberOfChannels();
                }
                else
                {
                    tempHeadAndTailDifference = ((ref->numberOfChannels()*SIZE_OF_MAIN_CIRCULAR_BUFFER-mainTail)+tempMainHead2)/ref->numberOfChannels();
                }
            }
        }
        //realy disconnect from device here
        if(!_deviceConnected)
        {
            ref->stopDevice();
            try {
                hid_close(ref->handle);
            }
            catch(std::exception &e)
            {
                std::cout<<"Error while closing device";
                // hid_free_enumeration(devs);
            }
            catch(...)
            {
                std::cout<<"Error while closing devices";
                //hid_free_enumeration(devs);
            }

            ref->handle = NULL;
            currentAddOnBoard = 0;
        }
        numberOfFrames = 0;
    }



    //
    // Read one batch of data from HID usb
    //
    int HIDUsbManager::readOneBatch(int32_t * obuffer)
    {
        unsigned char buffer[256];

        int writeInteger = 0;
        int obufferIndex = 0;
        int numberOfFrames = 0;
        int size = -1;


        try {
            size = hid_read(handle, buffer, sizeof(buffer));
        }
        catch(std::exception &e)
        {
            size = -1;
            std::cout<<"Error on read 3";

        }
        catch(...)
        {
            size = -1;
            std::cout<<"Error on read 4";
        }


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


        for(unsigned int i=2;i<sizeOfPackage+2;i++)
        {

            if(weAreInsideEscapeSequence)
            {
                messagesBuffer[messageBufferIndex] = buffer[i];
                messageBufferIndex++;
            }
            else
            {
                circularBuffer[cBufHead++] = buffer[i];

                if(cBufHead>=SIZE_OF_CIRC_BUFFER)
                {
                    cBufHead = 0;
                }
            }
            testEscapeSequence((unsigned int) buffer[i],  ((i-2)/2)/_numberOfChannels);
        }

        unsigned int LSB;
        unsigned int MSB;

        bool haveData = true;
        while (haveData)
        {

            MSB  = ((unsigned int)(circularBuffer[cBufTail])) & 0xFF;
            if(MSB > 127)//if we are at the begining of frame
            {
                if(checkIfHaveWholeFrame() && obufferIndex<1000)
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
                        obuffer[obufferIndex++] = (writeInteger-512)*62;

                        if(areWeAtTheEndOfFrame() || obufferIndex>1000)
                        {
                         //   std::cout<<"We brake at areWeAtTheEndOfFrame!!!!\n";
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
    int HIDUsbManager::readDevice(int32_t * obuffer)
    {
        int frames;

        int tempMainHead = mainHead;//keep head position because input thread will move it.

       if(mainTail>tempMainHead)
       {
          // std::cout<<"Head: "<<tempMainHead<<" tail "<<mainTail<<"\n";
           memcpy ( obuffer, &mainCircularBuffer[mainTail], sizeof(int32_t)*(maxSamples-mainTail));
           memcpy ( &obuffer[maxSamples-mainTail], mainCircularBuffer, sizeof(int32_t)*(tempMainHead));
           frames = ((maxSamples-mainTail)+tempMainHead)/_numberOfChannels;

       }
       else
       {
           memcpy ( obuffer, &mainCircularBuffer[mainTail], sizeof(int32_t)*(tempMainHead-mainTail));
           frames = (tempMainHead-mainTail)/_numberOfChannels;
       }

        mainTail = tempMainHead;

        if(restartDevice)
        {
            restartDevice = false;
            _manager->reloadHID();
        }

        return frames;
    }


    //
    // Get list of all HID devices attached to computer
    //
    void HIDUsbManager::getAllDevicesList()
    {
        try
        {
            if((!_deviceConnected) )
            {
               // std::cout<<"Call HID exit... \n";
                hid_exit();
            }
            list.clear();
            struct hid_device_info *devs, *cur_dev;
            //std::cout<<"Scan for HID devices... \n";
            devs = hid_enumerate(0x0, 0x0);
           // std::cout<<"HID After scan \n";
            cur_dev = devs;
            while (cur_dev) {
                //check VID and PID
                //std::cout<<"Check VID, check PID \n";
                if((cur_dev->vendor_id == BYB_VID) && (cur_dev->product_id == BYB_PID))
                {
                //     std::cout<<"HID while \n";
                     std::string nameOfHID((char *) cur_dev->product_string);
                   //  std::cout<<"Name took \n";

                     list.push_back(nameOfHID);
                   //  std::cout<<"HID name added to list \n";
                  //   std::cout<<"HID device: "<<cur_dev->vendor_id<<", "<<cur_dev->product_string<<"\n";



                }
               //  std::cout<<"Next device \n";
                cur_dev = cur_dev->next;
            }
           //  std::cout<<"Free enumeration \n";
            hid_free_enumeration(devs);
        }
        catch(std::exception &e)
        {
            std::cout<<"Error while scanning VID/PID of devices 2: "<<e.what();
           // hid_free_enumeration(devs);
        }
        catch(...)
        {
            std::cout<<"Error while scanning VID/PID of devices";
            //hid_free_enumeration(devs);
        }
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
            _powerRailState = -1;
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
    // Ask microcontroller for state of power
    // rail. ON or OFF
    //
    void HIDUsbManager::askForStateOfPowerRail()
    {
        std::stringstream sstm;
        sstm << "V:"<<";\n";
        writeToDevice((unsigned char*)(sstm.str().c_str()),sstm.str().length());
    }

    void HIDUsbManager::askForMaximumRatings()
    {
        std::stringstream sstm;
        sstm << "max:"<<";\n";
        writeToDevice((unsigned char*)(sstm.str().c_str()),sstm.str().length());
    }

    //
    // Ask if we have some add on board connected
    //
    void HIDUsbManager::askForBoard()
    {
        std::stringstream sstm;
        sstm << "board:"<<";\n";
        writeToDevice((unsigned char*)(sstm.str().c_str()),sstm.str().length());
    }

    //
    // Ask if Reaction timer is repeating
    //
    void HIDUsbManager::askForRTRepeat()
    {
        std::stringstream sstm;
        sstm << "rtrepeat:"<<";\n";
        writeToDevice((unsigned char*)(sstm.str().c_str()),sstm.str().length());
    }


    void HIDUsbManager::swapRTRepeat()
    {
        _rtReapeating = !_rtReapeating;
        std::stringstream sstm;
        sstm << "srtrepeat:"<<";\n";
        writeToDevice((unsigned char*)(sstm.str().c_str()),sstm.str().length());
    }

    //
    // Reaction timer repeating
    //
    bool HIDUsbManager::isRTRepeating()
    {
        return _rtReapeating;
    }

    //
    // Put microcontroller in update firmware mode.
    // This works only on windows
    //
    void HIDUsbManager::putInFirmwareUpdateMode()
    {
        #if defined(_WIN32)
            std::cout<<"Put MSP into firmware update\n";
             std::stringstream sstm;
            sstm << "update:;\n";
            writeToDevice((unsigned char*)(sstm.str().c_str()),sstm.str().length());
        #endif
    }

    //
    // Sends command to set number of channels and sampling rate on micro.
    //
    void HIDUsbManager::setNumberOfChannelsAndSamplingRate(int numberOfChannels, int samplingRate)
    {
        _numberOfChannels = numberOfChannels;
        _samplingRate = samplingRate;
       /* std::cout<<"HID - Set number of channels:"<<numberOfChannels<<" and sampling rate: "<<samplingRate<<"\n";
        std::stringstream sstm;
        sstm << "conf s:" << samplingRate<<";c:"<<numberOfChannels<<";\n";
        writeToDevice((unsigned char*)(sstm.str().c_str()),sstm.str().length());*/
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
        return _samplingRate;
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
