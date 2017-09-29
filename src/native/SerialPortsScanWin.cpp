//
//  SerialPortsScan.cpp
//  SpikeRecorder
//
//  Created by Stanislav Mircic on 9/6/17.
//  Copyright © 2017 BackyardBrains. All rights reserved.
//

#include "SerialPortsScan.h"
#include "Windows.h"
#include "Log.h"
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
#define NUMBER_OF_VIDS 5
uint16_t enabledVIDs[NUMBER_OF_VIDS] = {0x2341, 0x2A03, 0x0403, 0x1A86, 0x2E73};//conservative list: Arduino, FTDI, China and BYB
//int enabledVIDs[] = {0x2341, 0x2A03, 0x1B4F, 0x239A, 0x0403, 0x1A86, 0x4348, 0x10C4, 0x067B, 0x04D8, 0x04B4};//All VIDs
namespace BackyardBrains {


        void EnumerateValues(HKEY hKey, DWORD numValues)
        {
                 DWORD dwIndex = 0;
                    LPSTR valueName = new CHAR[64];
                 DWORD valNameLen;
                 DWORD dataType;
                 DWORD data;
                 DWORD dataSize;

                    for (int i = 0; i < numValues; i++)
                 {
                   DWORD valNameLen = 64;
                  RegEnumValue(hKey,
                     dwIndex,
                     valueName,
                     &valNameLen,
                     NULL,
                     &dataType,
                     (BYTE*)&data,
                     &dataSize);

                  dwIndex++;

                        printf("Code: 0x%08X\n", data);
                 }
        }


        void EnumerateSubKeys(HKEY RootKey, char* subKey, unsigned int tabs = 0)
        {
                 HKEY hKey;
                    DWORD cSubKeys;        //Used to store the number of Subkeys
                    DWORD maxSubkeyLen;    //Longest Subkey name length
                    DWORD cValues;        //Used to store the number of Subkeys
                    DWORD maxValueLen;    //Longest Subkey name length
                    DWORD retCode;        //Return values of calls

                 RegOpenKeyEx(RootKey, subKey, 0, KEY_ALL_ACCESS, &hKey);

                    RegQueryInfoKey(hKey,            // key handle
                                    NULL,            // buffer for class name
                                    NULL,            // size of class string
                                    NULL,            // reserved
                                    &cSubKeys,        // number of subkeys
                                    &maxSubkeyLen,    // longest subkey length
                                    NULL,            // longest class string
                                    &cValues,        // number of values for this key
                                    &maxValueLen,    // longest value name
                                    NULL,            // longest value data
                                    NULL,            // security descriptor
                                    NULL);            // last write time

                    if(cSubKeys>0)
                 {
                        char currentSubkey[MAX_PATH];

                        for(int i=0;i < cSubKeys;i++){
                   DWORD currentSubLen=MAX_PATH;

                            retCode=RegEnumKeyEx(hKey,    // Handle to an open/predefined key
                            i,                // Index of the subkey to retrieve.
                            currentSubkey,            // buffer to receives the name of the subkey
                            &currentSubLen,            // size of that buffer
                            NULL,                // Reserved
                            NULL,                // buffer for class string
                            NULL,                // size of that buffer
                            NULL);                // last write time

                            if(retCode==ERROR_SUCCESS)
                   {
                                for (int i = 0; i < tabs; i++)
                                    printf("\t");
                                printf("(%d) %s\n", i+1, currentSubkey);

                                char* subKeyPath = new char[currentSubLen + strlen(subKey)];
                                sprintf(subKeyPath, "%s\\%s", subKey, currentSubkey);
                    EnumerateSubKeys(RootKey, subKeyPath, (tabs + 1));
                   }
                  }
                 }
                    else
                 {
                  EnumerateValues(hKey, cValues);
                 }

                 RegCloseKey(hKey);
        }






    int getListOfSerialPorts( std::list<std::string>& listOfPorts)
    {

           // EnumerateSubKeys(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Enum\\USB");


          /*  HKEY hkCommMap;

            if (ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Enum\\USB"), 0, KEY_QUERY_VALUE, &hkCommMap))
            {
              return 0;
            }

             DWORD dwIndex = 0;
             LPSTR valueName = new CHAR[64];
             DWORD dataType;
             DWORD data;
             DWORD dataSize;

             for (int i = 0; i < numValues; i++)
             {
                  DWORD valNameLen = 64; //added this line to match valueName buffer size
                  RegEnumValue(hkCommMap,
                 dwIndex,
                 valueName,
                 &valNameLen,
                 NULL,
                 &dataType,
                 (BYTE*)&data,
                 &dataSize);

                 dwIndex++;

                  printf("Code: 0x%08X\n", data);
                  printf("Code: %s\n", valueName);
             }











                void* pValNameBuff = 0;
                void* pValueBuff = 0;

               DWORD dwValCount, dwMaxCharValNameLen, dwMaxByteValueSize;

               if (ERROR_SUCCESS != ::RegQueryInfoKey(hkCommMap, NULL, NULL, NULL, NULL, NULL, NULL,
                     &dwValCount, &dwMaxCharValNameLen, &dwMaxByteValueSize, NULL, NULL))
               {// regkey enum failed !!
                    ::RegCloseKey(hkCommMap);

                    return 0;  // ERROR EXIT
               }

               // The max value name size is returned in TCHARs not including the terminating null character.
                dwMaxCharValNameLen++;
               pValNameBuff = new TCHAR[dwMaxCharValNameLen];
               if (!pValNameBuff) {// no memory !!
                    ::RegCloseKey(hkCommMap);
                   return 0; // ERROR EXIT
               }
               // The max needed data size is returned in bytes
               dwMaxByteValueSize+=sizeof(TCHAR);
               DWORD dwMaxCharValueLen = (dwMaxByteValueSize/2) * sizeof(TCHAR); // num of TCHARS
               pValueBuff = new TCHAR[dwMaxCharValueLen];
               if (!pValueBuff) { // no memory !!
                    ::RegCloseKey(hkCommMap); delete pValNameBuff;
                   return 0; // ERROR EXIT
               }

               for (DWORD dwIndex = 0; dwIndex < dwValCount; ++dwIndex)
               {
                      DWORD dwCharValNameSize = dwMaxCharValNameLen;
                      DWORD dwByteValueSize   = dwMaxCharValueLen*sizeof(TCHAR);;
                      DWORD dwType;
                      LONG nRes = ::RegEnumValue(hkCommMap,
                                     dwIndex, (LPTSTR)pValNameBuff,&dwCharValNameSize, NULL,
                                     &dwType, (LPBYTE)pValueBuff, &dwByteValueSize);
                      if (nRes != ERROR_SUCCESS) {
                         break; // no more
                      }
                      std::string short_name((LPCTSTR)pValueBuff);
                      if (dwType != REG_SZ) {
                         continue; // not expected type - try next
                      }
                      // now we have name and value in the buffers (TCHAR)

                        Log::msg("---------  %s\n\n",short_name.c_str());
                      //m_pPorts[port_items] = new wxPortDescr();
                      //m_pPorts[port_items]->Init(port_items, short_name, 0, short_name, true,  true,  true, false, false, false, false);


               }//for

               // clean up
                  delete pValNameBuff;
               delete pValueBuff;
                ::RegCloseKey(hkCommMap);






*/





                /*if (ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_QUERY_VALUE, &hkCommMap))
               {
                  return 0;
                }

                void* pValNameBuff = 0;
                void* pValueBuff = 0;

               DWORD dwValCount, dwMaxCharValNameLen, dwMaxByteValueSize;

               if (ERROR_SUCCESS != ::RegQueryInfoKey(hkCommMap, NULL, NULL, NULL, NULL, NULL, NULL,
                     &dwValCount, &dwMaxCharValNameLen, &dwMaxByteValueSize, NULL, NULL))
               {// regkey enum failed !!
                    ::RegCloseKey(hkCommMap);

                    return 0;  // ERROR EXIT
               }

               // The max value name size is returned in TCHARs not including the terminating null character.
                dwMaxCharValNameLen++;
               pValNameBuff = new TCHAR[dwMaxCharValNameLen];
               if (!pValNameBuff) {// no memory !!
                    ::RegCloseKey(hkCommMap);
                   return 0; // ERROR EXIT
               }
               // The max needed data size is returned in bytes
               dwMaxByteValueSize+=sizeof(TCHAR);
               DWORD dwMaxCharValueLen = (dwMaxByteValueSize/2) * sizeof(TCHAR); // num of TCHARS
               pValueBuff = new TCHAR[dwMaxCharValueLen];
               if (!pValueBuff) { // no memory !!
                    ::RegCloseKey(hkCommMap); delete pValNameBuff;
                   return 0; // ERROR EXIT
               }

               for (DWORD dwIndex = 0; dwIndex < dwValCount; ++dwIndex)
               {
                      DWORD dwCharValNameSize = dwMaxCharValNameLen;
                      DWORD dwByteValueSize   = dwMaxCharValueLen*sizeof(TCHAR);;
                      DWORD dwType;
                      LONG nRes = ::RegEnumValue(hkCommMap,
                                     dwIndex, (LPTSTR)pValNameBuff,&dwCharValNameSize, NULL,
                                     &dwType, (LPBYTE)pValueBuff, &dwByteValueSize);
                      if (nRes != ERROR_SUCCESS) {
                         break; // no more
                      }
                      if (dwType != REG_SZ) {
                         continue; // not expected type - try next
                      }
                      // now we have name and value in the buffers (TCHAR)
                      std::string short_name((LPCTSTR)pValueBuff);
                        Log::msg("---------  %s\n\n",short_name.c_str());
                      //m_pPorts[port_items] = new wxPortDescr();
                      //m_pPorts[port_items]->Init(port_items, short_name, 0, short_name, true,  true,  true, false, false, false, false);


               }//for

               // clean up
                  delete pValNameBuff;
               delete pValueBuff;
                ::RegCloseKey(hkCommMap);
               */



        return 0;
    }
}
