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
#include <IOKit/IOCFSerialize.h>

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
#define NUMBER_OF_VIDS 7
UInt16 enabledVIDs[NUMBER_OF_VIDS] = {0x2341, 0x2A03, 0x0403, 0x1A86, 0x2E73, 0x10C4, 1155};//conservative list: Arduino, FTDI, China and BYB
//int enabledVIDs[] = {0x2341, 0x2A03, 0x1B4F, 0x239A, 0x0403, 0x1A86, 0x4348, 0x10C4, 0x067B, 0x04D8, 0x04B4};//All VIDs



namespace BackyardBrains {
    
    
    static mach_port_t masterPort;
    
    static void indent(Boolean node, int depth, UInt64 stackOfBits);
    static void properties(io_registry_entry_t service,
                           int depth,
                           UInt64 stackOfBits);
    static void traverse(unsigned int options,
                         io_name_t plane, io_iterator_t services,
                         io_registry_entry_t first,
                         int depth, UInt64 stackOfBits,io_name_t nameOfDeviceToInspect);
    static bool searchNowForPath = false;
    static bool foundThePath = false;
    static char actualPathThatWeFound[MAXPATHLEN];
    static io_name_t nameOfDeviceToInspectGlobal;
    static bool searchNowForPathInOnlyOneDevice = false;
    enum {
        kDoPropsOption = 1,
        kDoRootOption  = 2
    };
    
    int test(io_name_t nameOfDeviceToInspect)
    {
        io_registry_entry_t    root;

        unsigned int    options;
        kern_return_t     status;   ///na
        int            arg;
        
        // Parse args
        
       
        options = kDoPropsOption;
      
        
        for( int i=0;i<128;i++)
        {
            nameOfDeviceToInspectGlobal[i] = nameOfDeviceToInspect[i];
        }
        
        // Obtain the I/O Kit communication handle.
        
        //    status = IOGetMasterPort(&masterPort);
        status = IOMasterPort(bootstrap_port, &masterPort);
        assert(status == KERN_SUCCESS);
        
        // Obtain the registry root entry.
        
        root = IORegistryGetRootEntry(masterPort);
        assert(root);
        
        // Traverse below the root in the plane.
        
        traverse(options, kIOServicePlane, 0, root, 0, 0, nameOfDeviceToInspect);
        
        // Quit.
        
        //exit(0);
        return 0;
    }
    
    void traverse(unsigned int options,
                  io_name_t plane, io_iterator_t services,
                  io_registry_entry_t serviceUpNext,
                  int depth,
                  UInt64 stackOfBits,
                  io_name_t nameOfDeviceToInspect)
    {
        io_registry_entry_t service;                                ///ok
        Boolean        doProps;
        
        // We loop for every service in the list of services provided.
        
        while ( (service = serviceUpNext) )
        {
            io_iterator_t        children;
            Boolean            hasChildren;
            io_name_t            name;
            kern_return_t        status;
            io_registry_entry_t    child;
            int            busy;
            
            // Obtain the next service entry, if any.
            
            serviceUpNext = IOIteratorNext(services);
            
            // Obtain the current service entry's children, if any.
            
            status = IORegistryEntryGetChildIterator(service,
                                                     plane,
                                                     &children);
            assert(status == KERN_SUCCESS);
            
            child = IOIteratorNext(children); ///ok
            hasChildren = child ? true : false;
            
            // Save has-more-siblings state into stackOfBits for this depth.
            
            if (serviceUpNext)
                stackOfBits |=  (1 << depth);
            else
                stackOfBits &= ~(1 << depth);
            
            // Save has-children state into stackOfBits for this depth.
            
            if (hasChildren)
                stackOfBits |=  (2 << depth);
            else
                stackOfBits &= ~(2 << depth);
            
            //indent(true, depth, stackOfBits);
            
            // Print out the name of the service.
            
            status = IORegistryEntryGetName(service, name);
            assert(status == KERN_SUCCESS);
            
            
            //printf("%s depth: %d", name, depth);
            bool searchingForPath = false;
            if (strcmp(nameOfDeviceToInspect, name)== 0)
            {
                searchingForPath = true;
                searchNowForPath = true;
                printf("Found name of the device in reg\n");
            }
            if (strcmp("Root", name))
                doProps = (options & kDoPropsOption) != 0;
            else
                doProps = (options & kDoRootOption) != 0;
            
            // Print out the class of the service.
            
            status = IOObjectGetClass(service, name);
            assert(status == KERN_SUCCESS);
            //printf("  <class %s\n", name);
            
            /*status = IOServiceGetBusyState(service, &busy);
            if(status == KERN_SUCCESS)
                printf(", busy %d", busy);
            // Print out the retain count of the service.
            
            printf(", retain count %d>\n", IOObjectGetRetainCount(service));
            */
            // Print out the properties of the service.
            
            if (doProps)
                properties(service, depth, stackOfBits);
            
            // Recurse down.
            
            traverse(options, plane, children, child, depth + 1, stackOfBits, nameOfDeviceToInspect);
            searchNowForPathInOnlyOneDevice = false;
            if(searchingForPath)
            {
                searchingForPath = false;
                searchNowForPath = false;
                //printf("\n----------- END OF SEACH------- depth: %d\n", depth);
            }
            // Release resources.
            
            IOObjectRelease(children); children = 0;
            IOObjectRelease(service);  service  = 0;
        }
    }
    
    struct indent_ctxt {
        int depth;
        UInt64 stackOfBits;
    };
    
    static void printCFString(CFStringRef string)
    {
        CFIndex    len;
        char *    buffer;
        
        len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(string),
                                                CFStringGetSystemEncoding()) + sizeof('\0');
        buffer = (char *)malloc(len);
        if (buffer && CFStringGetCString(string, buffer, len,
                                         CFStringGetSystemEncoding()) )
            printf(buffer);
        
        if (buffer)
            free(buffer);
    }
    
    static void printEntry(const void *key, const void *value, void *context)
    {
        struct indent_ctxt * ctxt = (indent_ctxt *)context;
        

        // IOKit pretty
        CFDataRef    data;
        bool foundTheCallout = false;
        //indent(false, ctxt->depth, ctxt->stackOfBits);
        //printf("  ");
        if(searchNowForPath || searchNowForPathInOnlyOneDevice)
        {
            if(CFStringCompare((CFStringRef)key,CFSTR("IOCalloutDevice"),0)==kCFCompareEqualTo)
            {
                foundTheCallout = true;
            }
        }
        //printCFString( (CFStringRef)key );
        //printf(" = ");
        
        //print value
        data = IOCFSerialize((CFStringRef)value, kNilOptions);
        if( data)
        {
            if( 10000 > CFDataGetLength(data))
            {
                //printf((char*)CFDataGetBytePtr(data));
                char * tempTextPointer = (char*)CFDataGetBytePtr(data);
                char* tempResult = strstr(tempTextPointer, (char *) nameOfDeviceToInspectGlobal );
                if(tempResult!=nullptr)
                {
                    searchNowForPathInOnlyOneDevice = true;
                    //searchNowForPath = true;
                }
            }
            else
            {
                //printf("<is BIG>");
            }
            CFRelease(data);
        }
        else
        {
            //printf("<IOCFSerialize failed>");
        }
        //printf("\n");
        
        //if we found IOCalloutDevice key extract path from value
        if(foundTheCallout)
        {
                data = IOCFSerialize((CFStringRef)value, kNilOptions);
                if( data)
                {
                   
                    if( 10000 > CFDataGetLength(data))
                    {
                        foundThePath = true;
                        int i=0;
                        bool inside = false;
                        int pathi = 0;
                        
                        //parse something like this path: <string>/dev/cu.usbmodem143101</string>
                        while(((char*)CFDataGetBytePtr(data))[i]!=0  && i<MAXPATHLEN)
                        {
                            if(((char*)CFDataGetBytePtr(data))[i]=='<')
                            {
                                if(inside)
                                {
                                    i++;
                                    break;
                                }
                            }
                            if(inside)
                            {
                                actualPathThatWeFound[pathi] = ((char*)CFDataGetBytePtr(data))[i];
                                pathi++;
                            }
                            if(((char*)CFDataGetBytePtr(data))[i]=='>')
                            {
                                inside = true;
                            }
                            i++;
                        }
                        actualPathThatWeFound[pathi] = 0;
                        //printf((char*)CFDataGetBytePtr(data));
                    }
                    else
                    {
                        //printf("<is BIG>");
                    }
                    
                    CFRelease(data);
                } else
                {
                    //printf("<IOCFSerialize failed>");
                }
        }
        //printf("\n");
        

    }
    
    static void properties(io_registry_entry_t service,
                           int depth,
                           UInt64 stackOfBits)
    {
        CFMutableDictionaryRef     dictionary; ///ok
        kern_return_t    status;     ///na
        struct indent_ctxt    context;
        
        context.depth = depth;
        context.stackOfBits = stackOfBits;
        
        // Prepare to print out the service's properties.
        //indent(false, context.depth, context.stackOfBits);
        //printf("{\n");
        
        // Obtain the service's properties.
        
        status = IORegistryEntryCreateCFProperties(service,
                                                    &dictionary,
                                                   kCFAllocatorDefault, kNilOptions);
        assert( KERN_SUCCESS == status );
        assert( CFDictionaryGetTypeID() == CFGetTypeID(dictionary));
        
        CFDictionaryApplyFunction(dictionary,
                                  (CFDictionaryApplierFunction) printEntry, &context);
        
        CFRelease(dictionary);
        
        //indent(false, context.depth, context.stackOfBits);
        //printf("}\n");
        //indent(false, context.depth, context.stackOfBits);
        //printf("\n");
        
    }
    
    void indent(Boolean node, int depth, UInt64 stackOfBits)
    {
        int i;
        
        // stackOfBits representation, given current depth is n:
        //   bit n+1             = does depth n have children?       1=yes, 0=no
        //   bit [n, .. i .., 0] = does depth i have more siblings?  1=yes, 0=no
        
        if (node)
        {
            for (i = 0; i < depth; i++)
                printf( (stackOfBits & (1 << i)) ? "| " : "  " );
            
            printf("+-o ");
        }
        else // if (!node)
        {
            for (i = 0; i <= depth + 1; i++)
                printf( (stackOfBits & (1 << i)) ? "| " : "  " );
        }
    }
    
    
    
    
    
    
    int getListOfSerialPorts( std::list<std::string>& listOfPorts, std::string& portForBootloader)
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
            bool foundBootloader = false;
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
                 #ifdef LOG_USB
                Log::msg("Interesting device IOUSBHostDevice");
                #endif
            }
            else if (strcmp(deviceName, "STM32L4_Boot") == 0)
            {
                 #ifdef LOG_USB
                Log::msg("Found our bootloader for STM32");
                #endif
                foundBootloader = true;
            }
            else if (strcmp(deviceName, "Muscle SpikerBox Pro") == 0)
            {
                //usbmodem
                ////swap Musscle SpikerBox Pro with IOSerialBSDClient
                //sprintf(deviceName, "IOSerialBSDClient");

                 #ifdef LOG_USB
                Log::msg("Interesting device Muscle SpikerBox Pro");
                #endif
            }
            else if(strcmp(deviceName, "Arduino Leonardo") == 0)
            {
                 #ifdef LOG_USB
                Log::msg("Interesting board Arduino Leonardo");
                #endif
            }
            else if(strcmp(deviceName, "Arduino Uno") == 0)
            {
                 #ifdef LOG_USB
                Log::msg("Interesting board Arduino Uno");
                #endif
            }
            else if(strcmp(deviceName, "CP2102N USB to UART Bridge Controller")==0)
            {
                 #ifdef LOG_USB
                Log::msg("Interesting board CP2102N USB to UART Bridge Controller");
                #endif
            }
            else if(strcmp(deviceName, "FT230X Basic UART")==0)
            {
                 #ifdef LOG_USB
                Log::msg("Interesting board FT230X Basic UART");
                #endif
            }
            else if(strcmp(deviceName, "FT231X USB UART")==0)
            {
                 #ifdef LOG_USB
                Log::msg("Interesting board FT231X USB UART");
                #endif
            }
            else if(strcmp(deviceName, "32-bit MFi Development Kit")==0)
            {
                 #ifdef LOG_USB
                 Log::msg("Interesting board MFi");
                #endif
            }
            else if(strcmp(deviceName, "Arduino Due Prog. Port")==0)
            {
                 #ifdef LOG_USB
                Log::msg("Interesting board Arduino Due Prog. Port");
                #endif
            }
            else if(strcmp(deviceName, "USB2.0-Serial")==0)
            {
                 #ifdef LOG_USB
                Log::msg("Interesting board USB2.0-Serial");
                #endif
            }
            else if(strcmp(deviceName, "Arduino Due")==0)
            {
                 #ifdef LOG_USB
                Log::msg("Interesting board Arduino Due");
                #endif
            }
            else if(strcmp(deviceName,"H&B SpikerBox Pro")==0)
            {
                #ifdef LOG_USB
                Log::msg("Interesting board H&B SpikerBox Pro");
                #endif
            }
            else if(strcmp(deviceName,"STM32 Virtual ComPort")==0)
            {
                #ifdef LOG_USB
                Log::msg("Interesting board STM32 Virtual ComPort");
                #endif
            }
            else if (strcmp(deviceName,"Nano 33 BLE")==0)
            {
                #ifdef LOG_USB
                Log::msg("Interesting board MFi board SpikerBox");
                #endif
            }
            else if(strcmp(deviceName,"SpikerBox")==0)
            {
                #ifdef LOG_USB
                Log::msg("Interesting board MFi board SpikerBox");
                #endif
            }
            else if(strcmp(deviceName,"HHI 1v0")==0)
            {
                #ifdef LOG_USB
                Log::msg("Interesting board HHI 1v0");
                #endif
            }
            else if(strcmp(deviceName,"HHI 1v1")==0)
            {
                #ifdef LOG_USB
                Log::msg("Interesting board HHI 1v1");
                #endif
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
                                                        CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID650),
                                                        (LPVOID *)&dev);
            //Don’t need the intermediate plug-in after device interface
            //is created
            //(*plugInInterface)->Release(plugInInterface);
            if (result || !dev)
            {
                Log::msg("Couldn’t create a device interface (%08x)\n",
                         (int) result);
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
                                                                                                kIORegistryIterateRecursively | kIORegistryIterateParents);
                
                    if (deviceBSDName_cf)
                    {
                         //const char *cs = CFStringGetCStringPtr( deviceBSDName_cf, kCFStringEncodingMacRoman ) ;
                         CFStringGetCString((const __CFString *)deviceBSDName_cf,s, sizeof(s), kCFStringEncodingASCII);
                                            
                         //printf("Path: %s\n", cs);
                         #ifdef LOG_USB
                         Log::msg("Path: %s", s);
                         #endif
                        if(foundBootloader)
                        {
                            portForBootloader = portForBootloader+s;
                        }
                        else
                        {
                            listOfPorts.push_back(s);
                        }
                    }
                    else
                    {
                
                        foundThePath = false;
                        test(deviceName);
                        if(foundThePath)
                        {
                            
                            
                            int i = 0;
                            while(actualPathThatWeFound[i]!=0 && i<MAXPATHLEN)
                            {
                                s[i] =actualPathThatWeFound[i];
                                i++;
                            }
                            s[i]=0;
                            #ifdef LOG_USB
                            Log::msg("Path: %s", s);
                            #endif
                           
                            if(foundBootloader)
                            {
                                portForBootloader = portForBootloader+s;
                            }
                            else
                            {
                                listOfPorts.push_back(s);
                            }
                        }
                    }
              
                   /* if (deviceBSDName_cf)
                    {
                        //const char *cs = CFStringGetCStringPtr( deviceBSDName_cf, kCFStringEncodingMacRoman ) ;
                        CFStringGetCString((const __CFString *)deviceBSDName_cf,s, sizeof(s), kCFStringEncodingASCII);
                                           
                        //printf("Path: %s\n", cs);
                        #ifdef LOG_USB
                        Log::msg("Path: %s", s);
                        #endif
                        listOfPorts.push_back(s);
                    }*/
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
