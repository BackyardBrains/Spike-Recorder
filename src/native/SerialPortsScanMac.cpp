//
//  SerialPortsScan.cpp
//  SpikeRecorder
//
//  Created by Stanislav Mircic on 9/6/17.
//  Copyright © 2017 BackyardBrains. All rights reserved.
//

#include "SerialPortsScan.h"
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOBSD.h>

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

#include "Log.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOBSD.h>


#define LOG_USB 1

// Arduino VIDs:
//0x2341 - Arduino
//0x2A03 - Arduino
//0x1B4F - lilipad
//0x239A - caterina adafruit
//0x0403 - FTDI
//0x1A86 - chinese clones - CH340 chip
//0x4348 - CH341 chip
//0x10C4 - CP210x chip Silicon Labs
//0x067B - PL2303 Prolific Tech.
//0x04D8 - MCP2221 Microchip
//0x04B4 - CY7C65213 Cypress Semiconductor


//M0 pro  VID: 0x03eb - Atmel corp VID actualy (pid 2111 = Xplained Pro board debugger and programmer)

//0x2E73 - BYB VID
#define NUMBER_OF_VIDS 6
UInt16 enabledVIDs[NUMBER_OF_VIDS] = {0x2341, 0x2A03, 0x0403, 0x1A86, 0x2E73, 0x10C4};//conservative list: Arduino, FTDI, China and BYB
//int enabledVIDs[] = {0x2341, 0x2A03, 0x1B4F, 0x239A, 0x0403, 0x1A86, 0x4348, 0x10C4, 0x067B, 0x04D8, 0x04B4};//All VIDs



namespace BackyardBrains {
    int getListOfSerialPorts( std::list<std::string>& listOfPorts)
    {
        listOfPorts.clear();
        CFMutableDictionaryRef matchingDict;
        io_iterator_t iter;
        kern_return_t kr;
        io_service_t device;
        
        /* set up a matching dictionary for the class */
        matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
        if (matchingDict == NULL)
        {
            return -1; // fail
        }
        
        /* Now we have a dictionary, get an iterator.*/
        kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iter);
        if (kr != KERN_SUCCESS)
        {
            return -1;
        }
        
        /* iterate */
        while ((device = IOIteratorNext(iter)))
        {
            /* do something with device, eg. check properties */
            /* ... */
            /* And free the reference taken before continuing to the next item */
            
            
            
            /***Display the device names ***/
            io_name_t	deviceName;
            kr = IORegistryEntryGetName(device, deviceName);
            if (KERN_SUCCESS != kr)
            {
                deviceName[0] = '\0';
            }
            #ifdef LOG_USB
            Log::msg("USB Device found: %s", deviceName);
            #endif
            
            if (strcmp(deviceName, "IOUSBHostDevice") == 0)
            {
                Log::msg("Interesting device");
            }
            else if(strcmp(deviceName, "Arduino Leonardo") == 0)
            {
                Log::msg("Interesting board");
            }
            else if(strcmp(deviceName, "Arduino Uno") == 0)
            {
                Log::msg("Interesting board");
            }
            else if(strcmp(deviceName, "CP2102N USB to UART Bridge Controller")==0)
            {
                Log::msg("Interesting board");
            }
            else if(strcmp(deviceName, "FT230X Basic UART")==0)
            {
                Log::msg("Interesting board");
            }
            //32-bit MFi Development Kit
            else if(strcmp(deviceName, "32-bit MFi Development Kit")==0)
            {
                Log::msg("Interesting board");
            }
            else
            {
                #ifdef LOG_USB
                Log::msg("Skipping");
                #endif
                continue;
            }
            //printf("deviceName:%s\n",deviceName);
            
            IOCFPlugInInterface         **plugInInterface = NULL;
            IOUSBDeviceInterface        **dev = NULL;
            HRESULT                     result;
            
            SInt32                      score;
            UInt16                      vendor;
          //  UInt16                      product;
            
            //Create an intermediate plug-in
            usleep(500000);
            kr = IOCreatePlugInInterfaceForService(device,
                                                   kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID,
                                                   &plugInInterface, &score);
            //Don’t need the device object after intermediate plug-in is created
            //kr = IOObjectRelease(device);
            if ((kIOReturnSuccess != kr) || !plugInInterface)
            {
                Log::msg("Unable to create a plug-in");
                continue;
            }
            //Now create the device interface
            result = (*plugInInterface)->QueryInterface(plugInInterface,
                                                        CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID),
                                                        (LPVOID *)&dev);
            //Don’t need the intermediate plug-in after device interface
            //is created
            //(*plugInInterface)->Release(plugInInterface);
            if (result || !dev)
            {
                Log::msg("Couldn’t create a device interface");
                continue;
            }
            
            //Check these values for confirmation
            
            kr = (*dev)->GetDeviceVendor(dev, &vendor);
           // kr = (*dev)->GetDeviceProduct(dev, &product);
           
            #ifdef LOG_USB
            Log::msg("Vendor ID: %x", (int)vendor);
            #endif
            bool isItEnabledVID = false;
            for (int i=0;i<NUMBER_OF_VIDS;i++)
            {
                if(vendor == enabledVIDs[i])
                {
                    isItEnabledVID = true;
                    break;
                }
            }
            //printf("\nVendor ID:%04x", vendor);
            //printf("\nProduct ID:%04x", product);
           
            
            if(isItEnabledVID)
            {
                    CFTypeRef nameCFstring;
                    char s[MAXPATHLEN];
             
                
                    CFStringRef deviceBSDName_cf = ( CFStringRef ) IORegistryEntrySearchCFProperty (device,
                                                                                                kIOServicePlane,
                                                                                                CFSTR (kIOCalloutDeviceKey ),
                                                                                                kCFAllocatorDefault,
                                                                                                kIORegistryIterateRecursively );
                    if (deviceBSDName_cf)
                    {
                        //const char *cs = CFStringGetCStringPtr( deviceBSDName_cf, kCFStringEncodingMacRoman ) ;
                        CFStringGetCString((const __CFString *)deviceBSDName_cf,s, sizeof(s), kCFStringEncodingASCII);
                                           
                        //printf("Path: %s\n", cs);
                        #ifdef LOG_USB
                        Log::msg("Path: %s", s);
                        #endif
                        listOfPorts.push_back(s);
                    }
            }
            
            (*plugInInterface)->Release(plugInInterface);
            kr = IOObjectRelease(device);
            kr = IOObjectRelease(device);
        }
        
        /* Done, release the iterator */
        IOObjectRelease(iter);
        return 0;
    }
}
