//
//  ArduinoSerial.cpp
//  SpikeRecorder
//
//  Created by Stanislav Mircic on 11/26/14.
//  Copyright (c) 2014 UNIT. All rights reserved.
//

#include "ArduinoSerial.h"
#include <sys/types.h>
#include "Log.h"
#include <unistd.h>
#include <sstream>
#include <stdint.h>


#define NUMBER_OF_TIMES_TO_SCAN_UNKNOWN_PORT 10

#if defined(__linux__)
    #include <sys/uio.h>
    #include <sys/types.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <sys/select.h>
    #include <termios.h>
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #include <sys/ioctl.h>
    #include <linux/serial.h>
    #include <cstring>
    #include <cstdio>

#elif defined(__APPLE__)
//added for port scan
    #include <sys/uio.h>
    #include <stdio.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <errno.h>
    #include <paths.h>
    #include <SerialPortsScan.h>
    #include <termios.h>
    #include <sysexits.h>
    #include <sys/param.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <time.h>
    //used for patch for nonstandard bauds
    #define IOSSIOSPEED _IOW('T', 2, speed_t)

#elif _WIN32
    #include <windows.h>
    #include "native/SerialPortsScan.h"
    #define win32_err(s) FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, \
                GetLastError(), 0, (s), sizeof(s), NULL)
    #define QUERYDOSDEVICE_BUFFER_SIZE 262144
    typedef unsigned int uint;
    #include <cstdio>
#endif


//#define LOG_SCANNING_OF_ARDUINO 1



namespace BackyardBrains {

    bool ArduinoSerial::openPortLock = false;

    ArduinoSerial::ArduinoSerial() : _portOpened(false) {

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
        //start thread that will periodicaly read HID

    }


//---------------------------------- Port scanning and opening ------------------------------
#pragma mark - Port scanning and opening


void ArduinoSerial::startScanningForArduinos(ArduinoSerial * refToWorkingArduinoSerial)
{

    scanningThread = std::thread(&ArduinoSerial::scanPortsThreadFunction, this, this, refToWorkingArduinoSerial);
    scanningThread.detach();
}


//
// Thread that scans if we have Arduino attached to computer
//
void ArduinoSerial::scanPortsThreadFunction(ArduinoSerial * selfRef, ArduinoSerial * workingArduinoRef)
{

    while(1)
    {
        #if defined(__APPLE__) || defined(__linux__)

                usleep(500000);
        #else
                Sleep(700);
        #endif
        #ifdef LOG_SCANNING_OF_ARDUINO
        Log::msg("New cycle in thread");
        #endif
        selfRef->checkAllPortsForArduino(workingArduinoRef);
    }
}




#if defined(__linux__)
    // All linux serial port device names.  Hopefully all of them anyway.  This
    // is a long list, but each entry takes only a few bytes and a quick strcmp()
    static const char *devnames[] = {
        "S",	// "normal" Serial Ports - MANY drivers using this
        "USB",	// USB to serial converters
        "ACM",	// USB serial modem, CDC class, Abstract Control Model
        "MI",	// MOXA Smartio/Industio family multiport serial... nice card, I have one :-)
        "MX",	// MOXA Intellio family multiport serial
        "C",	// Cyclades async multiport, no longer available, but I have an old ISA one! :-)
        "D",	// Digiboard (still in 2.6 but no longer supported), new Moschip MCS9901
        "P",	// Hayes ESP serial cards (obsolete)
        "M",	// PAM Software's multimodem & Multitech ISI-Cards
        "E",	// Stallion intelligent multiport (no longer made)
        "L",	// RISCom/8 multiport serial
        "W",	// specialix IO8+ multiport serial
        "X",	// Specialix SX series cards, also SI & XIO series
        "SR",	// Specialix RIO serial card 257+
        "n",	// Digi International Neo (yes lowercase 'n', drivers/serial/jsm/jsm_driver.c)
        "FB",	// serial port on the 21285 StrongArm-110 core logic chip
        "AM",	// ARM AMBA-type serial ports (no DTR/RTS)
        "AMA",	// ARM AMBA-type serial ports (no DTR/RTS)
        "AT",	// Atmel AT91 / AT32 Serial ports
        "BF",	// Blackfin 5xx serial ports (Analog Devices embedded DSP chips)
        "CL",	// CLPS711x serial ports (ARM processor)
        "A",	// ICOM Serial
        "SMX",	// Motorola IMX serial ports
        "SOIC",	// ioc3 serial
        "IOC",	// ioc4 serial
        "PSC",	// Freescale MPC52xx PSCs configured as UARTs
        "MM",	// MPSC (UART mode) on Marvell GT64240, GT64260, MV64340...
        "B",	// Mux console found in some PA-RISC servers
        "NX",	// NetX serial port
        "PZ",	// PowerMac Z85c30 based ESCC cell found in the "macio" ASIC
        "SAC",	// Samsung S3C24XX onboard UARTs
        "SA",	// SA11x0 serial ports
        "AM",	// KS8695 serial ports & Sharp LH7A40X embedded serial ports
        "TX",	// TX3927/TX4927/TX4925/TX4938 internal SIO controller
        "SC",	// Hitachi SuperH on-chip serial module
        "SG",	// C-Brick Serial Port (and console) SGI Altix machines
        "HV",	// SUN4V hypervisor console
        "UL",	// Xilinx uartlite serial controller
        "VR",	// NEC VR4100 series Serial Interface Unit
        "CPM",	// CPM (SCC/SMC) serial ports; core driver
        "Y",	// Amiga A2232 board
        "SL",	// Microgate SyncLink ISA and PCI high speed multiprotocol serial
        "SLG",	// Microgate SyncLink GT (might be sync HDLC only?)
        "SLM",	// Microgate SyncLink Multiport high speed multiprotocol serial
        "CH",	// Chase Research AT/PCI-Fast serial card
        "F",	// Computone IntelliPort serial card
        "H",	// Chase serial card
        "I",	// virtual modems
        "R",	// Comtrol RocketPort
        "SI",	// SmartIO serial card
        "T",	// Technology Concepts serial card
        "V"	// Comtrol VS-1000 serial controller
    };
#define NUM_DEVNAMES (sizeof(devnames) / sizeof(const char *))
#endif


#ifdef __APPLE__

    void ArduinoSerial::macos_ports(io_iterator_t  * PortIterator)
    {
        io_object_t modemService;
        CFTypeRef nameCFstring;
        char s[MAXPATHLEN];

        while ((modemService = IOIteratorNext(*PortIterator))) {
            nameCFstring = IORegistryEntryCreateCFProperty(modemService,
                                                           CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
            if (nameCFstring) {
                if (CFStringGetCString((const __CFString *)nameCFstring,
                                       s, sizeof(s), kCFStringEncodingASCII)) {
                    if(strstr(s,"Bluetooth")==NULL && strstr(s,"lpss")== NULL)
                    {
                        list.push_back(s);
                    }
                }
                CFRelease(nameCFstring);
            }
            IOObjectRelease(modemService);
        }
    }

#endif // __APPLE__


    // Return a list of all serial ports
    void ArduinoSerial::getAllPortsList()
    {



        list.clear();


#if defined(__linux__)
        // This is ugly guessing, but Linux doesn't seem to provide anything else.
        // If there really is an API to discover serial devices on Linux, please
        // email paul@pjrc.com with the info.  Please?
        // The really BAD aspect is all ports get DTR raised briefly, because linux
        // has no way to open the port without raising DTR, and there isn't any way
        // to tell if the device file really represents hardware without opening it.
        // maybe sysfs or udev provides a useful API??
        DIR *dir;
        struct dirent *f;
        struct stat st;
        unsigned int i, len[NUM_DEVNAMES];
        char s[512];
        int fd, bits;
        termios mytios;

        dir = opendir("/dev/");
        if (dir == NULL) return ;
        for (i=0; i<NUM_DEVNAMES; i++) len[i] = strlen(devnames[i]);
        // Read all the filenames from the /dev directory...
        while ((f = readdir(dir)) != NULL) {
            // ignore everything that doesn't begin with "tty"
            if (strncmp(f->d_name, "tty", 3)) continue;
            // ignore anything that's not a known serial device name
            for (i=0; i<NUM_DEVNAMES; i++) {
                if (!strncmp(f->d_name + 3, devnames[i], len[i])) break;
            }
            if (i >= NUM_DEVNAMES) continue;
            snprintf(s, sizeof(s), "/dev/%s", f->d_name);
            // check if it's a character type device (almost certainly is)
            if (stat(s, &st) != 0 || !(st.st_mode & S_IFCHR)) continue;
            // now see if we can open the file - if the device file is
            // populating /dev but doesn't actually represent a loaded
            // driver, this is where we will detect it.
            fd = open(s, O_RDONLY | O_NOCTTY | O_NONBLOCK);
            if (fd < 0) {
                // if permission denied, give benefit of the doubt
                // (otherwise the port will be invisible to the user
                // and we won't have a to alert them to the permssion
                // problem)
                if (errno == EACCES) list.push_back(s);
                // any other error, assume it's not a real device
                continue;
            }
            // does it respond to termios requests? (probably will since
            // the name began with tty).  Some devices where a single
            // driver exports multiple names will open but this is where
            // we can really tell if they work with real hardare.
            if (tcgetattr(fd, &mytios) != 0) {
                close(fd);
                continue;
            }
            // does it respond to reading the control signals?  If it's
            // some sort of non-serial terminal (eg, pseudo terminals)
            // this is where we will detect it's not really a serial port
            if (ioctl(fd, TIOCMGET, &bits) < 0) {
                close(fd);
                continue;
            }
            // it passed all the tests, it's a serial port, or some sort
            // of "terminal" that looks exactly like a real serial port!
            close(fd);
            // unfortunately, Linux always raises DTR when open is called.
            // not nice!  Every serial port is going to get DTR raised
            // and then lowered.  I wish there were a way to prevent this,
            // but it seems impossible.
            list.push_back(s);
        }
        closedir(dir);
#elif defined(__APPLE__)

        getListOfSerialPorts(list);

        // adapted from SerialPortSample.c, by Apple
        // http://developer.apple.com/samplecode/SerialPortSample/listing2.html
        // and also testserial.c, by Keyspan
        // http://www.keyspan.com/downloads-files/developer/macosx/KesypanTestSerial.c
        // www.rxtx.org, src/SerialImp.c seems to be based on Keyspan's testserial.c
        // neither keyspan nor rxtx properly release memory allocated.
        // more documentation at:
        // http://developer.apple.com/documentation/DeviceDrivers/Conceptual/WorkingWSerial/WWSerial_SerialDevs/chapter_2_section_6.html
       /* mach_port_t masterPort;
        CFMutableDictionaryRef classesToMatch;
        io_iterator_t serialPortIterator;
        if (IOMasterPort(NULL, &masterPort) != KERN_SUCCESS) return;
        // a usb-serial adaptor is usually considered a "modem",
        // especially when it implements the CDC class spec
        classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
        if (!classesToMatch) return;
        CFDictionarySetValue(classesToMatch, CFSTR(kIOSerialBSDTypeKey),
                             CFSTR(kIOSerialBSDModemType));
        if (IOServiceGetMatchingServices(masterPort, classesToMatch,
                                         &serialPortIterator) != KERN_SUCCESS) return;
        macos_ports(&serialPortIterator);
        IOObjectRelease(serialPortIterator);
        // but it might be considered a "rs232 port", so repeat this
        // search for rs232 ports
        classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
        if (!classesToMatch) return;
        CFDictionarySetValue(classesToMatch, CFSTR(kIOSerialBSDTypeKey),
                             CFSTR(kIOSerialBSDRS232Type));
        if (IOServiceGetMatchingServices(masterPort, classesToMatch,
                                         &serialPortIterator) != KERN_SUCCESS) return;
        macos_ports(&serialPortIterator);
        IOObjectRelease(serialPortIterator);*/

#elif defined(_WIN32)

        //getListOfSerialPorts(list);

        // http://msdn.microsoft.com/en-us/library/aa365461(VS.85).aspx
        // page with 7 ways - not all of them work!
        // http://www.naughter.com/enumser.html
        // may be possible to just query the windows registary
        // http://it.gps678.com/2/ca9c8631868fdd65.html
        // search in HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM
        // Vista has some special new way, vista-only
        // http://msdn2.microsoft.com/en-us/library/aa814070(VS.85).aspx
        char *buffer, *p;
        //DWORD size = QUERYDOSDEVICE_BUFFER_SIZE;
        DWORD ret;

        buffer = (char *)malloc(QUERYDOSDEVICE_BUFFER_SIZE);
        if (buffer == NULL) return;
        memset(buffer, 0, QUERYDOSDEVICE_BUFFER_SIZE);
        ret = QueryDosDeviceA(NULL, buffer, QUERYDOSDEVICE_BUFFER_SIZE);
        if (ret) {
            printf("Detect Serial using QueryDosDeviceA: ");
            for (p = buffer; *p; p += strlen(p) + 1) {
                //printf(":  %s\n", p);
                if (strncmp(p, "COM", 3)) continue;
                printf("\nFound port  %s\n", p);
                std::stringstream sstm;
                sstm << p << ":";
                list.push_back(sstm.str().c_str());
            }
        } else {
            char buf[1024];
            win32_err(buf);
            printf("QueryDosDeviceA failed, error \"%s\"\n", buf);
            printf("Detect Serial using brute force GetDefaultCommConfig probing: ");
            for (int i=1; i<=32; i++) {
                printf("try  %s", buf);
                COMMCONFIG cfg;
                DWORD len;
                snprintf(buf, sizeof(buf), "COM%d", i);
                if (GetDefaultCommConfig(buf, &cfg, &len)) {
                    //wxString name;
                    //name.Printf("COM%d:", i);
                    std::stringstream sstm;
                    sstm << "COM" << i;
                    list.push_back(sstm.str().c_str());

                    //list.Add(name);
                    printf(":  %s", buf);
                }
            }
        }
        free(buffer);


#endif // defined

        list.sort();
        refreshPortsDataList();

        return;
    }



    void ArduinoSerial::refreshPortsDataList()
    {

        std::list<std::string>::iterator list_it;
        std::list<SerialPort>::iterator portsIterator;

        //Add new to list of ports

        for(list_it = list.begin(); list_it!= list.end(); list_it++)
        {
            bool portAlreadyInPortList = false;
            for(portsIterator = ports.begin(); portsIterator!= ports.end(); portsIterator++)
            {
                std::size_t found=list_it->find(portsIterator->portName);
                if (found!=std::string::npos)
                {
                    portAlreadyInPortList = true;
                    break;
                }
            }

            if(portAlreadyInPortList == false)
            {
                SerialPort newPort;
                newPort.portName = *list_it;
                newPort.deviceType = ArduinoSerial::unknown;
                ports.push_back(newPort);
            }
        }

        //remove nonexisting from list of ports
        for(portsIterator = ports.begin(); portsIterator!= ports.end(); portsIterator++)
        {
            bool portFoundInList = false;
            for(list_it = list.begin(); list_it!= list.end(); list_it++)
            {

                std::size_t found=list_it->find(portsIterator->portName);
                if (found!=std::string::npos)
                {
                    portFoundInList = true;
                    break;
                }
            }

            if(portFoundInList == false)
            {
                portsIterator = ports.erase(portsIterator);
                portsIterator--;
            }
        }
    }


    void ArduinoSerial::setDeviceTypeToCurrentPort(SerialDevice deviceType)
    {

        std::list<SerialPort>::iterator portsIterator;

        for(portsIterator = ports.begin(); portsIterator!= ports.end(); portsIterator++)
        {
            std::size_t found=portsIterator->portName.find(_portName);
            if (found!=std::string::npos)
            {
                portsIterator->deviceType = deviceType;
                currentPort.deviceType = deviceType;
                currentPort.portName = portsIterator->portName;
                break;
            }
        }
    }



    int ArduinoSerial::openPort(const char *portName)
    {
        int portDescriptor;
#if defined(__APPLE__) || defined(__linux__)
        struct termios options;

        portDescriptor = open(portName, O_RDWR | O_NOCTTY | O_NDELAY);//O_SHLOCK
        std::cout<<"Sleep start\n";
        usleep(300000);
        std::cout<<"Sleep end\n";
        int bits;
#endif
#ifdef __APPLE__
        std::stringstream sstm;
        #ifdef LOG_SCANNING_OF_ARDUINO
        Log::msg("Open port - ");
        Log::msg(portName);
        #endif
        if (portDescriptor < 0) {
            sstm << "Unable to open " << portName << ", " << strerror(errno);
            errorString = sstm.str();

            #ifdef LOG_SCANNING_OF_ARDUINO
            std::cout<<"Unable to open "<<portName<<", "<<strerror(errno)<<"\n";
            Log::msg(errorString.c_str());
            #endif
            return -1;
        }
        if (ioctl(portDescriptor, TIOCEXCL) == -1) {
            close(portDescriptor);
            sstm << "Unable to get exclussive access to port " << portName;;
            errorString = sstm.str();
            #ifdef LOG_SCANNING_OF_ARDUINO
            Log::msg(errorString.c_str());
            std::cout<<"Unable to get exclussive access to port "<<portName<<"\n";
            #endif
            return -1;
        }
        if (ioctl(portDescriptor, TIOCMGET, &bits) < 0) {
            close(portDescriptor);
            sstm <<"Unable to query serial port signals on " << portName;
            errorString = sstm.str();
            #ifdef LOG_SCANNING_OF_ARDUINO
            Log::msg(errorString.c_str());
            std::cout<<"Unable to query serial port signals on "<<portName<<"\n";
            #endif
            return -1;
        }
        //bits &= ~(TIOCM_DTR | TIOCM_RTS);

//------------ Stanislav changed TIOCM_DTR to HIGH since Leonardo didnt work
        bits &= ~(TIOCM_RTS);
        if (ioctl(portDescriptor, TIOCMSET, &bits) < 0) {
            close(portDescriptor);
            sstm <<"Unable to control serial port signals on " << portName;
            errorString = sstm.str();
            #ifdef LOG_SCANNING_OF_ARDUINO
            Log::msg(errorString.c_str());
            std::cout<<"Unable to control serial port signals on "<<portName<<"\n";
            #endif
            return -1;
        }
        struct termios settings_orig;
        if (tcgetattr(portDescriptor, &settings_orig) < 0) {
            close(portDescriptor);
            sstm <<"Unable to access baud rate on port " << portName;
            errorString = sstm.str();
            #ifdef LOG_SCANNING_OF_ARDUINO
            Log::msg(errorString.c_str());
            std::cout<<"Unable to access baud rate on port "<<portName<<"\n";
            #endif
            return -1;
        }
#endif
#ifdef __linux__
        // struct serial_struct kernel_serial_settings;
        struct termios settings_orig;
        //struct termios settings;
        if (portDescriptor < 0)
        {
            if (errno == EACCES)
            {
                std::cout<<"Unable to access "<< portName<< ", insufficient permission";

            }
            else if (errno == EISDIR)
            {
                std::cout<< "Unable to open " << portName <<
                ", Object is a directory, not a serial port";
            }
            else if (errno == ENODEV || errno == ENXIO)
            {
                std::cout<< "Unable to open " << portName <<
                ", Serial port hardware not installed";
            }
            else if (errno == ENOENT)
            {
                std::cout<< "Unable to open " << portName <<
                ", Device name does not exist";
            }
            else
            {
                std::cout<< "Unable to open " << portName; //<<

            }
            return -1;
        }
        if (ioctl(portDescriptor, TIOCMGET, &bits) < 0)
        {
            close(portDescriptor);
            std::cout<< "Unable to query serial port signals";
            return -1;
        }
       // bits &= ~(TIOCM_DTR | TIOCM_RTS);
       bits &= ~( TIOCM_RTS);
        if (ioctl(portDescriptor, TIOCMSET, &bits) < 0)
        {
            close(portDescriptor);
            std::cout<< "Unable to control serial port signals";
            return -1;
        }
        if (tcgetattr(portDescriptor, &settings_orig) != 0)
        {
            close(portDescriptor);
            std::cout<< "Unable to query serial port settings (perhaps not a serial port)";
            return -1;
        }
        /*memset(&settings, 0, sizeof(settings));
         settings.c_iflag = IGNBRK | IGNPAR;
         settings.c_cflag = CS8 | CREAD | HUPCL | CLOCAL;
         Set_baud(baud_rate);
         if (ioctl(port_fd, TIOCGSERIAL, &kernel_serial_settings) == 0) {
         kernel_serial_settings.flags |= ASYNC_LOW_LATENCY;
         ioctl(port_fd, TIOCSSERIAL, &kernel_serial_settings);
         }
         tcflush(port_fd, TCIFLUSH);*/
#endif
#if defined(__APPLE__) || defined(__linux__)
        if (portDescriptor == -1)
        {
            #ifdef LOG_SCANNING_OF_ARDUINO
            std::cout<<"Can't open serial port\n";
            Log::msg("Can't open serial port\n");
            #endif
            return -1;
        }
        fcntl(portDescriptor, F_SETFL, 0);    // clear all flags on descriptor, enable direct I/O
        tcgetattr(portDescriptor, &options);   // read serial port options
        // enable receiver, set 8 bit data, ignore control lines
        options.c_cflag |= (CLOCAL | CREAD | CS8);
        // disable parity generation and 2 stop bits
        options.c_cflag &= ~(PARENB | CSTOPB);


        //traditional setup of baud rates
        //------------------------ traditional setup of baud rates for Mac and Linux --------------
        cfsetispeed(&options, B230400);
        cfsetospeed(&options, B230400);
        // set the new port options
        tcsetattr(portDescriptor, TCSANOW, &options);
        //------------------------ traditional setup of baud rates for Mac and Linux --------------

        //--------------------------- Patch for Mac for nonstandard bauds --------------------------
        /*speed_t speed = 2000000; // Set 2Mbaud
         if (ioctl(portDescriptor, IOSSIOSPEED, &speed) == -1) {
         std::cout<<"Error setting speed";
         }*/



        /*

         Explanation from blog post:
         http://stackoverflow.com/questions/9366249/boostasioserialport-alternative-that-supports-non-standard-baud-rates

         If you only want to use ioctl and termios you can do:

         #define IOSSIOSPEED _IOW('T', 2, speed_t)
         int new_baud = static_cast<int> (baudrate_);
         ioctl (portDescriptor_, IOSSIOSPEED, &new_baud, 1);
         And it will let you set the baud rate to any value in OS X, but that is OS specific. for Linux you need to do:

         struct serial_struct ser;
         ioctl (portDescriptor_, TIOCGSERIAL, &ser);
         // set custom divisor
         ser.custom_divisor = ser.baud_base / baudrate_;
         // update flags
         ser.flags &= ~ASYNC_SPD_MASK;
         ser.flags |= ASYNC_SPD_CUST;

         if (ioctl (portDescriptor_, TIOCSSERIAL, ser) < 0)
         {
         // error
         }
         For any other OS your gonna have to go read some man pages or it might not even work at all, its very OS dependent.

         */



        //--------------------------- Patch for Mac for nonstandard bauds --------------------------
#endif

#ifdef _WIN32

        COMMCONFIG cfg;
        COMMTIMEOUTS timeouts;
        int got_default_cfg=0, port_num;
        char buf[1024], name_createfile[64], name_commconfig[64], *p;
        DWORD len;

        snprintf(buf, sizeof(buf), "%s", _portName.c_str());
        p = strstr(buf, "COM");
        if (p && sscanf(p + 3, "%d", &port_num) == 1) {
            printf("port_num = %d\n", port_num);
            snprintf(name_createfile, sizeof(name_createfile), "\\\\.\\COM%d", port_num);
            snprintf(name_commconfig, sizeof(name_commconfig), "COM%d", port_num);
        } else {
            snprintf(name_createfile, sizeof(name_createfile), "%s", _portName.c_str());
            snprintf(name_commconfig, sizeof(name_commconfig), "%s", _portName.c_str());
        }

        len = sizeof(COMMCONFIG);
        //Stanislav commented this out since it was freezing app for 100ms and
        //it did some weard things with windows (refreshing file list in dev tool and disabling language selector in tray bar)
        /*if (GetDefaultCommConfig(name_commconfig, &cfg, &len)) {
         // this prevents unintentionally raising DTR when opening
         // might only work on COM1 to COM9
         got_default_cfg = 1;
         memcpy(&port_cfg_orig, &cfg, sizeof(COMMCONFIG));
         cfg.dcb.fDtrControl = DTR_CONTROL_DISABLE;
         cfg.dcb.fRtsControl = RTS_CONTROL_DISABLE;
         SetDefaultCommConfig(name_commconfig, &cfg, sizeof(COMMCONFIG));
         } else {
         printf("error with GetDefaultCommConfig\n");
         }
         */
        port_handle = CreateFile(name_createfile, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        //port_handle = CreateFile(name_createfile, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, NULL, NULL);
        if (port_handle == INVALID_HANDLE_VALUE) {
            win32_err(buf);
            //error_msg =  "Unable to open " + _portName + ", " + buf;
            return -1;
        }
        len = sizeof(COMMCONFIG);

        if (!GetCommConfig(port_handle, &port_cfg, &len)) {
            CloseHandle(port_handle);
            win32_err(buf);
            //error_msg = "Unable to read communication config on " + _portName + ", " + buf;
            return -1;
        }
        if (!got_default_cfg) {
            memcpy(&port_cfg_orig, &port_cfg, sizeof(COMMCONFIG));
        }

        // http://msdn2.microsoft.com/en-us/library/aa363188(VS.85).aspx
        port_cfg.dcb.BaudRate = 230400; //for high speed 2Mbit/s communication just change this number to 2000000

        port_cfg.dcb.fBinary = TRUE;
        port_cfg.dcb.fParity = FALSE;
        port_cfg.dcb.fOutxCtsFlow = FALSE;
        port_cfg.dcb.fOutxDsrFlow = FALSE;
       // port_cfg.dcb.fDtrControl = DTR_CONTROL_ENABLE;
        port_cfg.dcb.fDtrControl = DTR_CONTROL_DISABLE;
        port_cfg.dcb.fDsrSensitivity = FALSE;
        port_cfg.dcb.fTXContinueOnXoff = TRUE;	// ???
        port_cfg.dcb.fOutX = FALSE;
        port_cfg.dcb.fInX = FALSE;
        port_cfg.dcb.fErrorChar = FALSE;
        port_cfg.dcb.fNull = FALSE;
        port_cfg.dcb.fRtsControl = RTS_CONTROL_ENABLE;
        //port_cfg.dcb.fRtsControl = RTS_CONTROL_DISABLE;
        port_cfg.dcb.fAbortOnError = FALSE;
        port_cfg.dcb.ByteSize = 8;
        port_cfg.dcb.Parity = NOPARITY;
        port_cfg.dcb.StopBits = ONESTOPBIT;
        if (!SetCommConfig(port_handle, &port_cfg, sizeof(COMMCONFIG))) {
            CloseHandle(port_handle);
            win32_err(buf);
            //error_msg = "Unable to write communication config to " + name + ", " + buf;
            return -1;
        }
        if (!EscapeCommFunction(port_handle, CLRDTR | CLRRTS)) {
            CloseHandle(port_handle);
            win32_err(buf);
            //error_msg = "Unable to control serial port signals on " + name + ", " + buf;
            return -1;
        }
        // http://msdn2.microsoft.com/en-us/library/aa363190(VS.85).aspx
        // setting to all zeros means timeouts are not used
        timeouts.ReadIntervalTimeout		= MAXDWORD;
        timeouts.ReadTotalTimeoutMultiplier	= 0;
        timeouts.ReadTotalTimeoutConstant	= 0;
        timeouts.WriteTotalTimeoutMultiplier	= 0;
        timeouts.WriteTotalTimeoutConstant	= 0;
        if (!SetCommTimeouts(port_handle, &timeouts)) {
            CloseHandle(port_handle);
            win32_err(buf);
            //error_msg = "Unable to write timeout settings to " + name + ", " + buf;
            return -1;
        }
        return (int)port_handle;
#endif // _WIN32



        return portDescriptor;
    }




    int ArduinoSerial::openSerialDevice(const char *portName)
    {
        #ifdef LOG_SCANNING_OF_ARDUINO
        Log::msg("openSerialDevice before lock");
        Log::msg(portName);
        #endif
        int i;
        while(ArduinoSerial::openPortLock==true){
            #ifdef LOG_SCANNING_OF_ARDUINO
            Log::msg("openSerialDevice  lock----");
            #endif
        }
        ArduinoSerial::openPortLock = true;
        i=1+1;
        #ifdef LOG_SCANNING_OF_ARDUINO
        Log::msg("openSerialDevice after lock");
        #endif
        int fd  = openSerialDeviceWithoutLock(portName);

        //update current port

        std::list<SerialPort>::iterator portsIterator;

        for(portsIterator = ports.begin(); portsIterator!= ports.end(); portsIterator++)
        {
            std::size_t found=portsIterator->portName.find(portName);
            if (found!=std::string::npos)
            {
                currentPort.deviceType = portsIterator->deviceType;
                currentPort.portName = portsIterator->portName;
                break;
            }
        }

        //unlock port connection
        ArduinoSerial::openPortLock = false;
        return fd;
    }



    int ArduinoSerial::openSerialDeviceWithoutLock(const char *portName)
    {

        //Log::msg("openSerialDeviceWithoutLock before lock");
        _portName = std::string(portName);
        _portOpened = false;
        //Log::msg("openSerialDeviceWithoutLock after lock");

        fd = 0;
        _numberOfChannels = 1;

        fd = openPort(portName);
        if(fd==-1)
        {
            #ifdef LOG_SCANNING_OF_ARDUINO
            Log::msg("fail openSerialDeviceWithoutLock");
            #endif
            return -1;
        }



        circularBuffer[0] = '\n';

        cBufHead = 0;
        cBufTail = 0;

        serialCounter = 0;

        escapeSequenceDetectorIndex = 0;
        weAreInsideEscapeSequence = false;
        messageBufferIndex =0;

        _portOpened = true;

        setNumberOfChannelsAndSamplingRate(1, maxSamplingRate());
        //askForBoardType();

        return fd;
    }




    void ArduinoSerial::checkAllPortsForArduino(ArduinoSerial * workingArduinoRef)
    {

            #ifdef LOG_SCANNING_OF_ARDUINO
            std::cout<<"\nCheck for Arduino boards \n";
            Log::msg("checkAllPortsForArduino");
            #endif
            getAllPortsList();
            #ifdef LOG_SCANNING_OF_ARDUINO
            Log::msg("checkAllPortsForArduino Before lock");
            #endif
            int i = 0;
                while(ArduinoSerial::openPortLock==true){
                    #ifdef LOG_SCANNING_OF_ARDUINO
                    Log::msg("checkAllPortsForArduino lock --");
                    #endif
                }
                ArduinoSerial::openPortLock = true;
                i = i+1;
                #ifdef LOG_SCANNING_OF_ARDUINO
                Log::msg("checkAllPortsForArduino After lock");
                #endif
            std::list<SerialPort>::iterator list_it;
            for(list_it = ports.begin(); list_it!= ports.end(); list_it++)
            {
                #ifdef LOG_SCANNING_OF_ARDUINO
                std::cout<<"\nTry Port: "<<list_it->portName.c_str()<<"\n";
                Log::msg("Try port");
                #endif
                Log::msg(list_it->portName.c_str());


                std::size_t found=list_it->portName.find(workingArduinoRef->currentPortName());

                if (found!=std::string::npos && workingArduinoRef->portOpened())
                {

                    workingArduinoRef->currentPort.portName = workingArduinoRef->currentPortName();
                    workingArduinoRef->currentPort.deviceType = list_it->deviceType;
                    #ifdef LOG_SCANNING_OF_ARDUINO
                    std::cout<<"Port: "<<list_it->portName.c_str()<<" Already opened by another thread.\n";
                    Log::msg("checkAllPortsForArduino Already opened by another thread.");
                    #endif
                   // ArduinoSerial::openPortLock = false;
                    continue;
                }
                if (list_it->deviceType != SerialDevice::unknown)
                {
                    #ifdef LOG_SCANNING_OF_ARDUINO
                    std::cout<<"Port: "<<list_it->portName.c_str()<<" Already checked\n";
                    Log::msg("checkAllPortsForArduino Already checked");
                    #endif
                   // ArduinoSerial::openPortLock = false;
                    continue;
                }
                if(list_it->numOfTrials>NUMBER_OF_TIMES_TO_SCAN_UNKNOWN_PORT)
                {
                    #ifdef LOG_SCANNING_OF_ARDUINO
                    std::cout<<"Port: "<<list_it->portName.c_str()<<" checked 10 times\n";
                    Log::msg("checkAllPortsForArduino checked 10 times");
                    #endif
                   // ArduinoSerial::openPortLock = false;
                    continue;
                }
                list_it->numOfTrials++;
                #ifdef LOG_SCANNING_OF_ARDUINO
                Log::msg("checkAllPortsForArduino now open port");
                #endif

                if(openSerialDeviceWithoutLock(list_it->portName.c_str()) != -1)
                {
                    #ifdef LOG_SCANNING_OF_ARDUINO
                    Log::msg("checkAllPortsForArduino port opened sucess");
                    #endif
                    //check if it is our Arduino board
                    hardwareType.clear();
                    char buffer[33024];
                    #ifdef LOG_SCANNING_OF_ARDUINO
                    std::cout<<"Port: "<<list_it->portName.c_str()<<" Sucess\n";
                    #endif
                    time_t firstSeconds;

                    firstSeconds = time(NULL);

                    while(time(NULL)-firstSeconds<2)
                    {
                       // std::cout<<"Port: "<<list_it->c_str()<<" read\n";


                        //askForBoardType();
                        askForBoardType();
                        #if defined(__APPLE__) || defined(__linux__)

                                                usleep(100000);
                        #else
                                                Sleep(100);
                        #endif


                        int bytesRead = readPort(buffer);


                        for(int i=0;i<bytesRead;i++)
                        {
                            if(weAreInsideEscapeSequence)
                            {
                                messagesBuffer[messageBufferIndex] = buffer[i];
                                messageBufferIndex++;
                            }
                            testEscapeSequence(((unsigned int) buffer[i]) & 0xFF,  (i/2)/_numberOfChannels);
                        }

                        if(hardwareType.length()>0)
                        {
                            //found Arduino board that responded
                            #ifdef LOG_SCANNING_OF_ARDUINO
                            std::cout<<"Found Arduino !!!!!!!\n";
                            Log::msg("checkAllPortsForArduino found arduino");
                            #endif

                            break;
                        }
                    }
                    #ifdef LOG_SCANNING_OF_ARDUINO
                    std::cout<<"Close Port: "<<list_it->portName.c_str()<<"\n";
                    Log::msg("checkAllPortsForArduino close port");
                    #endif

                    closeSerial();
                }
                else
                {
                    #ifdef LOG_SCANNING_OF_ARDUINO
                    std::cout<<"\nPort: "<<list_it->portName.c_str()<<" Failed!\n";
                    Log::msg("checkAllPortsForArduino failed");
                    #endif
                    closeSerial();
                }
                #ifdef LOG_SCANNING_OF_ARDUINO
                Log::msg("checkAllPortsForArduino refresh working instance list");
                #endif
                workingArduinoRef->refreshPortList(ports);



            }
            #ifdef LOG_SCANNING_OF_ARDUINO
            Log::msg("checkAllPortsForArduino relaese lock");
            #endif
             ArduinoSerial::openPortLock = false;

    }


    // Close the port
    void ArduinoSerial::closeSerial(void)
    {
        currentPort.deviceType = ArduinoSerial::unknown;
        currentPort.portName = "";
        if(_portOpened)
        {
            setNumberOfChannelsAndSamplingRate(1, maxSamplingRate());
#if defined(__linux__) || defined(__APPLE__)
            close(fd);
#elif defined(_WIN32)

       //port_cfg.dcb.fRtsControl = RTS_CONTROL_ENABLE;
        port_cfg.dcb.fRtsControl = RTS_CONTROL_DISABLE;
        if (!SetCommConfig(port_handle, &port_cfg, sizeof(COMMCONFIG))) {
            CloseHandle(port_handle);
            //error_msg = "Unable to write communication config to " + name + ", " + buf;
        }




            CloseHandle(port_handle);
#endif
            _portOpened = false;

        }
    }

    //
    // Replace current ports list with new one
    //
    void ArduinoSerial::refreshPortList(std::list<SerialPort> newPorts)
    {
        ports.clear();

        if(newPorts.size()==0)
        {
            return;
        }

        std::list<SerialPort>::iterator portsIterator;

        //Add new to list of ports

        for(portsIterator = newPorts.begin(); portsIterator!= newPorts.end(); portsIterator++)
        {
                SerialPort newPort;
                newPort.portName = portsIterator->portName;
                newPort.deviceType = portsIterator->deviceType;
                ports.push_back(newPort);
        }






    }





//---------------------------------- Reading and processing ------------------------------
#pragma mark - Reading and processing


    int ArduinoSerial::readPort(char * buffer)
    {

        ssize_t size = -1;
#if defined(__APPLE__) || defined(__linux__)
        // Initialize file descriptor sets
        fd_set read_fds, write_fds, except_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&except_fds);
        FD_SET(fd, &read_fds);
        // Set timeout to 60 ms
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 60000;




        if (select(fd + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1)
        {
            size = read(fd, buffer, 32768 );
        }
        else
        {
            std::cout<<"Serial read error: Timeout\n";
        }
        if (size < 0)
        {
            if(errno == EAGAIN)
            {
                std::cout<<"Serial read error: 1\n";
            }
            if(errno == EINTR)
            {
                std::cout<<"Serial read error: 2\n";
            }
        }
#endif // defined
#if defined(__linux__)
        int bits;
        if (size == 0 && ioctl(fd, TIOCMGET, &bits) < 0)
        {
            std::cout<<"Serial read error: 3\n";
        }
#endif

#if defined(_WIN32)


        // first, we'll find out how many bytes have been received
        // and are currently waiting for us in the receive buffer.
        //   http://msdn.microsoft.com/en-us/library/ms885167.aspx
        //   http://msdn.microsoft.com/en-us/library/ms885173.aspx
        //   http://source.winehq.org/WineAPI/ClearCommError.html
        COMSTAT st;
        DWORD errmask=0, num_read, num_request;
        OVERLAPPED ov;
        int count = 32768 ;

        if (!ClearCommError(port_handle, &errmask, &st))
        {
            return -1;
        }
        //printf("Read, %d requested, %lu buffered\n", count, st.cbInQue);
        if (st.cbInQue <= 0)
        {
            return 0;
        }

        // now do a ReadFile, now that we know how much we can read
        // a blocking (non-overlapped) read would be simple, but win32
        // is all-or-nothing on async I/O and we must have it enabled
        // because it's the only way to get a timeout for WaitCommEvent


        num_request = ((DWORD)count < st.cbInQue) ? (DWORD)count : st.cbInQue;





        ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (ov.hEvent == NULL) return -1;
        ov.Internal = ov.InternalHigh = 0;
        ov.Offset = ov.OffsetHigh = 0;

        if (ReadFile(port_handle, buffer, num_request, &num_read, &ov)) {
            // this should usually be the result, since we asked for
            // data we knew was already buffered
            //printf("Read, immediate complete, num_read=%lu\n", num_read);
            size = num_read;

            // std::cout <<num_request<<" -- " <<num_request-num_read<<"\n";
        } else {
            if (GetLastError() == ERROR_IO_PENDING) {
                if (GetOverlappedResult(port_handle, &ov, &num_read, TRUE)) {
                    printf("Read, delayed, num_read=%lu\n", num_read);
                    //std::cout<<" -----------------" <<num_request<<" -- " <<num_request-num_read<<"\n";
                    size = num_read;
                } else {
                    printf("Read, delayed error\n");
                    //std::cout<<" Read, delayed error------------------\n";
                    size = -1;
                }
            } else {
                printf("Read, error\n");
                //std::cout<<"Read, error~~~~~~~~~~~~~~~~~~~~\n";
                size = -1;
            }
        }
        CloseHandle(ov.hEvent);

#endif // defined

        return (int)size;
    }


    //
    // Read port and process frames into samples
    //
    int ArduinoSerial::getNewSamples(int16_t * obuffer)
    {
        char buffer[33024];
        int bytesRead = readPort(buffer);
        int numberOfSamples =  processDataIntoSamples(buffer, bytesRead, obuffer);
        return numberOfSamples;
    }

    //
    // Process raw data from serial port
    // Extract frames and extract samples from frames
    //
    int ArduinoSerial::processDataIntoSamples(char * buffer, int size, int16_t * obuffer)
    {
        int numberOfFrames = 0;
        int obufferIndex = 0;
        int writeInteger = 0;
        // std::cout<<"------------------ Size: "<<size<<"\n";


        for(int i=0;i<size;i++)
        {
            if(weAreInsideEscapeSequence)
            {
                messagesBuffer[messageBufferIndex] = buffer[i];
                messageBufferIndex++;
            }
            else
            {
                circularBuffer[cBufHead++] = buffer[i];
                 //uint debugMSB  = ((uint)(buffer[i])) & 0xFF;
                 // std::cout<<"M: " << debugMSB<<"\n";

                if(cBufHead>=SIZE_OF_CIRC_BUFFER)
                {
                    cBufHead = 0;
                }
            }
             testEscapeSequence(((unsigned int) buffer[i]) & 0xFF,  (i/2)/_numberOfChannels);
        }
        if(size==-1)
        {
            return -1;
        }
        uint LSB;
        uint MSB;
        bool haveData = true;
        bool weAlreadyProcessedBeginingOfTheFrame;
        int numberOfParsedChannels;
        while (haveData)
        {

            MSB  = ((uint)(circularBuffer[cBufTail])) & 0xFF;

            if(MSB > 127)//if we are at the begining of frame
            {
                weAlreadyProcessedBeginingOfTheFrame = false;
                numberOfParsedChannels = 0;
                if(checkIfHaveWholeFrame())
                {
                    //std::cout<<"Inside serial "<< numberOfFrames<<"\n";
                    numberOfFrames++;
                    while (1)
                    {
                        //make sample value from two consecutive bytes
                        // std::cout<<"Tail: "<<cBufTail<<"\n";
                        //  MSB  = ((uint)(circularBuffer[cBufTail])) & 0xFF;
                        //std::cout<< cBufTail<<" -M "<<MSB<<"\n";


                        MSB  = ((uint)(circularBuffer[cBufTail])) & 0xFF;
                        if(weAlreadyProcessedBeginingOfTheFrame && MSB>127)
                        {
                            //we have begining of the frame inside frame
                            //something is wrong
                            numberOfFrames--;
                            break;//continue as if we have new frame
                        }
                        MSB  = ((uint)(circularBuffer[cBufTail])) & 0x7F;
                        weAlreadyProcessedBeginingOfTheFrame = true;

                        cBufTail++;
                        if(cBufTail>=SIZE_OF_CIRC_BUFFER)
                        {
                            cBufTail = 0;
                        }
                        LSB  = ((uint)(circularBuffer[cBufTail])) & 0xFF;
                        //if we have error in frame (lost data)
                        if(LSB>127)
                        {
                            numberOfFrames--;
                            break;//continue as if we have new frame
                        }
                        // std::cout<< cBufTail<<" -L "<<LSB<<"\n";
                        LSB  = ((uint)(circularBuffer[cBufTail])) & 0x7F;

                        MSB = MSB<<7;
                        writeInteger = LSB | MSB;
                        //  if(writeInteger>300)
                        //  {
                        //      logData = true;
                        //  }


                        numberOfParsedChannels++;
                        if(numberOfParsedChannels>numberOfChannels())
                        {
                            //we have more data in frame than we need
                            //something is wrong with this frame
                            numberOfFrames--;
                            break;//continue as if we have new frame
                        }
                        obuffer[obufferIndex++] = (writeInteger-512)*30;


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




    bool ArduinoSerial::checkIfNextByteExist()
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

    bool ArduinoSerial::checkIfHaveWholeFrame()
    {
        int tempTail = cBufTail + 1;
        if(tempTail>= SIZE_OF_CIRC_BUFFER)
        {
            tempTail = 0;
        }
        while(tempTail!=cBufHead)
        {
            uint nextByte  = ((uint)(circularBuffer[tempTail])) & 0xFF;
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

    bool ArduinoSerial::areWeAtTheEndOfFrame()
    {
        int tempTail = cBufTail + 1;
        if(tempTail>= SIZE_OF_CIRC_BUFFER)
        {
            tempTail = 0;
        }
        uint nextByte  = ((uint)(circularBuffer[tempTail])) & 0xFF;
        if(nextByte > 127)
        {
            return true;
        }
        return false;
    }


    //
    // Detect start-of-message escape sequence and end-of-message sequence
    // and set up weAreInsideEscapeSequence.
    // When we detect end-of-message sequence call executeContentOfMessageBuffer()
    //
    void ArduinoSerial::testEscapeSequence(unsigned int newByte, int offset)
    {



        if(weAreInsideEscapeSequence)
        {

            if(messageBufferIndex>=SIZE_OF_MESSAGES_BUFFER)
            {
                weAreInsideEscapeSequence = false; //end of escape sequence
                executeContentOfMessageBuffer(offset);
                escapeSequenceDetectorIndex = 0;//prepare for detecting begining of sequence
            }
            else if(endOfescapeSequence[escapeSequenceDetectorIndex] == newByte)
            {
                escapeSequenceDetectorIndex++;
                if(escapeSequenceDetectorIndex ==  ESCAPE_SEQUENCE_LENGTH)
                {
                    weAreInsideEscapeSequence = false; //end of escape sequence
                    executeContentOfMessageBuffer(offset);
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
    void ArduinoSerial::executeContentOfMessageBuffer(int offset)
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
            //std::cout<<"----- MB Arduino: "<< currentPositionInString<<"     :"<<messagesBuffer<<"\n";
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


    void ArduinoSerial::executeOneMessage(std::string typeOfMessage, std::string valueOfMessage, int offsetin)
    {
        std::cout<<"\nMESSAGE Arduino: "<<typeOfMessage<<" - "<<valueOfMessage<<"\n";
        Log::msg("Message");
        Log::msg(typeOfMessage.c_str());
        Log::msg(valueOfMessage.c_str());

        if(typeOfMessage == "HWT")
        {
            hardwareType = valueOfMessage;

            std::size_t found=hardwareType.find("PLANTSS");
            if (found!=std::string::npos)
            {
                setDeviceTypeToCurrentPort(ArduinoSerial::plant);
            }
            else
            {
                std::size_t found=hardwareType.find("MUSCLESS");
                if (found!=std::string::npos)
                {
                    setDeviceTypeToCurrentPort(ArduinoSerial::muscle);
                }
                else
                {
                    std::size_t found=hardwareType.find("HEARTSS");
                    if (found!=std::string::npos)
                    {
                        setDeviceTypeToCurrentPort(ArduinoSerial::heart);
                    }
                    else
                    {
                        std::size_t found=hardwareType.find("HBLEOSB");//leonardo heart and brain with one channel only
                        if (found!=std::string::npos)
                        {
                            setDeviceTypeToCurrentPort(ArduinoSerial::heartOneChannel);
                        }
                    }
                }
            }
        }

        /*if(typeOfMessage == "EVNT")
        {
            int mnum = (int)((unsigned int)valueOfMessage[0]-48);
            int64_t offset = 0;
            _manager->addMarker(std::string(1, mnum+'0'), offset+offsetin);

        }*/
    }



    //---------------------------------- Getters/parameters ------------------------------
#pragma mark - Getters/parameters
    const char * ArduinoSerial::currentPortName()
    {
        return _portName.c_str();
    }

    int ArduinoSerial::maxSamplingRate()
    {
        return 10000;
    }

    int ArduinoSerial::maxNumberOfChannels()
    {

        return 6;
    }

    int ArduinoSerial::numberOfChannels()
    {
        return _numberOfChannels;
    }


    bool ArduinoSerial::portOpened()
    {
        return _portOpened;
    }


//---------------------------------- Messages - Write ------------------------------------------------------
#pragma mark - Messages - Write

    void ArduinoSerial::setNumberOfChannelsAndSamplingRate(int numberOfChannels, int samplingRate)
    {
        _numberOfChannels = numberOfChannels;
        _samplingRate = samplingRate;
        //"conf s:%d;c:%d;"
        std::stringstream sstm;
        sstm << "conf s:" << samplingRate<<";c:"<<numberOfChannels<<";\n";
        writeToPort(sstm.str().c_str(),(int)(sstm.str().length()));
    }


    void ArduinoSerial::sendEventMessage(int eventType)
    {
        std::stringstream sstm;
        sstm << "p:" << eventType<<";\n";
        writeToPort(sstm.str().c_str(),(int)(sstm.str().length()));

    }

    void ArduinoSerial::sendPotentiometerMessage(uint8_t value)
    {
        std::stringstream sstm;
        sstm << "a:" << (uint8_t)value<<";\n";
       // writeToPort(sstm.str().c_str(),(int)(sstm.str().length()));

    }


    void ArduinoSerial::askForBoardType()
    {
        std::stringstream sstm;
        sstm << "b:;\n";
         writeToPort(sstm.str().c_str(),(int)(sstm.str().length()));

    }

    int ArduinoSerial::writeToPort(const void *ptr, int len)
    {

        #if defined(__APPLE__) || defined(__linux__)
        int n, written=0;
        fd_set wfds;
        struct timeval tv;

        while (written < len) {
           // std::cout<<written<<",";
            n = write(fd, (const char *)ptr + written, len - written);
            if (n < 0 && (errno == EAGAIN || errno == EINTR))
            {
               n = 0;
            }
            //printf("Write, n = %d\n", n);
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
                FD_SET(fd, &wfds);
                n = select(fd+1, NULL, &wfds, NULL, &tv);
                if (n < 0 && errno == EINTR) n = 1;
                if (n <= 0) return -1;
            }
        }
        //std::cout<<written<<"-"<<(char *)ptr<<"|";
        return written;



        #elif defined(_WIN32)
            DWORD num_written;
            OVERLAPPED ov;
            int r;
            ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (ov.hEvent == NULL) return -1;
            ov.Internal = ov.InternalHigh = 0;
            ov.Offset = ov.OffsetHigh = 0;
           if (WriteFile(port_handle, ptr, len, &num_written, &ov)) {

                //printf("Write, immediate complete, num_written=%lu\n", num_written);
                r = num_written;
              //  std::cout<<" 888888888 - " <<len<<" -- " <<num_written-len<<"\n";
            } else {
                if (GetLastError() == ERROR_IO_PENDING) {
                    if (GetOverlappedResult(port_handle, &ov, &num_written, TRUE)) {
                        //printf("Write, delayed, num_written=%lu\n", num_written);
                        r = num_written;
                      //  std::cout<<" 888888888 - " <<"Write, delayed, num_written" <<num_written-len<<"\n";
                    } else {
                        //printf("Write, delayed error\n");
                        std::cout<<" 888888888 - " <<"Write, delayed,error" <<num_written-len<<"\n";
                        r = -1;
                    }
                } else {
                    //printf("Write, error\n");
                  //  std::cout<<" 888888888 - " <<"Write,error" <<num_written-len<<"\n";
                    r = -1;
                }
            };
            CloseHandle(ov.hEvent);
            return r;
        #endif
    }


}
