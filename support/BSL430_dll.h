/*
 * MSP430 USB Firmware Upgrade Example
 *
 * An example software to field firmware upgrade a MSP430 USB based device
 *
 * Copyright (C) {2010} Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/
#ifdef __cplusplus
extern "C" {
#endif

#include "BSL_Definitions.h"
#include <windows.h>

#define FAMILY_5438  1
#define FAMILY_ROM   2
#define FAMILY_FLASH 3

#define COM_UART 1
#define COM_USB  2

// BSL operation was successful
#define BSL_SUCCESS           0x00
// BSL Acknowledged
#define BSL_ACK               0x00
// BSL failed
#define BSL_FAILED            0x01
// BSL could not find any devices
#define BSL_DEVICE_NOT_FOUND  0x01
// BSL failed to register USB HID with Windows Handle
#define BSL_USB_HANDLE_ERROR  0x06

// CRC Error Codes
// CRC passes
#define CRC_SUCCESS           0x00
// CRC failed
#define CRC_FAILED            -1

/* #defines for TI I/O File Operation */
// Successful in opening file
#define OPERATION_SUCCESSFUL  0
// File is at EOF 
#define TXT_EOF               -1
// Error found during opening
#define ERROR_OPENING_FILE    -2
// No data being read yet
#define NO_DATA_READ          -3

/*******************************************************************************
*Function:    BSL_TX_BSL_Version_String
*Valid Mode:  5438, Flash
*Description: Queries the BSL for version string
*Parameters: 
*             none
*Returns:
*             unsigned char*   detailed description of current BSL or
*                              "ERROR" if failed
*******************************************************************************/
extern unsigned char* BSL_TX_BSL_Version_String();

/*******************************************************************************
*Function:    BSL_TX_BufferSize
*Valid Mode:  5438, Flash
*Description: Queries the BSL for it’s buffer size, adjusts all communication
*             appropriately
*Parameters: 
*             none
*Returns:
*             signed int        BSL Buffer size
*                               -1 = wrong family
*                               -2 = BSL did not acknowledge
*******************************************************************************/
extern signed int BSL_TX_BufferSize();

/*******************************************************************************
*Function:    BSL_TX_TXT_File
*Valid Mode:  ROM, 5438, Flash
*Description: Reads BSL from targeted device to a file
*Parameters: 
*             fileName          Filename to be created and written
*             addr              Address from where to start reading
*             length            Length of data to be read
*Returns:
*             unsigned int      BSL_ACK           - Success
*******************************************************************************/
extern unsigned int BSL_TX_TXT_File( char* fileName, unsigned int addr, unsigned int length );

/*******************************************************************************
*Function:    BSL_RX_TXT_File
*Valid Mode:  ROM, 5438, Flash
*Description: Transmit BSL to targeted device from a file
*Parameters: 
*             fileName          Filename to be opened and read
*             fast              0 - Slow flash download w BSL ack on every 
*                                   packet
*                               1 - fast flash download w/o BSL ack on every
*                                   packet
*Returns:
*             unsigned int      BSL_ACK           - Success
*******************************************************************************/
extern unsigned int BSL_RX_TXT_File( char* fileName, unsigned char fast);

/*******************************************************************************
*Function:    BSL_RX_TXT
*Valid Mode:  ROM, 5438, Flash
*Description: Transmit BSL to targeted device using an array of characters
*Parameters: 
*             dataArray         Character array of data to be transmitted
*             fast              0 - Slow flash download w BSL ack on every 
*                                   packet
*                               1 - fast flash download w/o BSL ack on every
*                                   packet
*Returns:
*             unsigned int      BSL_ACK           - Success
*******************************************************************************/
extern unsigned int BSL_RX_TXT( char * dataArray, unsigned char fast);

/*******************************************************************************
*Function:    BSL_setFamily
*Valid Mode:  ROM, 5438, Flash
*Description: This command also tells the PC side which communication
*             protocol should be used
*Parameters: 
*             family            FAMILY_5438   - Specific to F543x devices only
*                               FAMILY_ROM    - Specific to ROM based BSL
*                               FAMILY_FLASH  - Specific to Flash based BSL
*Returns:
*             none
*******************************************************************************/
extern void BSL_setFamily( unsigned int family );

/*******************************************************************************
*Function:    BSL_setCom
*Valid Mode:  ROM, 5438, Flash
*Description: Initializes the PC port for communication with the BSL, does not 
*             invoke the BSL.
*Parameters: 
*             com               COM_UART      - UART based communication
*                               COM_USB       - USB HID based communication
*Returns:
*             none
*******************************************************************************/
extern void BSL_setCom( unsigned int com );

/*******************************************************************************
*Function:    BSL_close_BSL
*Valid Mode:  USB
*Description: This command closes the USB BSL port.
*Parameters: 
*             none
*Returns:
*             unsigned int      BSL_SUCCESS          - Success
*                               BSL_DEVICE_NOT_FOUND - Device not found
*******************************************************************************/
extern unsigned char BSL_close_BSL( void );

/*******************************************************************************
*Function:    BSL_initialize_BSL
*Valid Mode:  ROM, 5438, Flash
*Description: This command invokes the BSL. For UART, it initializes based on
*             COM port and the device into BSL mode. For USB, it
*             initializes the HID based on a VID or PID. If no VID and PID
*             supplied, a default TI VID and PID is used
*Parameters: 
*             com               UART - BSL_initialize_BSL("COMx");
*                               USB  - BSL_initialize_BSL("");  // Uses default TI VID and PID
*                                      BSL_initialize_BSL("VID=0x2056,PID=0x201,DEVICEINDEX=0"); // Uses explicit VID and PID
*Returns:
*             unsigned int      BSL_SUCCESS          - Success
*                               BSL_DEVICE_NOT_FOUND - Device not found
*******************************************************************************/
extern unsigned char BSL_initialize_BSL( unsigned char* com);

/*******************************************************************************
*Function:    BSL_RegisterUSBDeviceNotification
*Valid Mode:  USB
*Description: Registers USB BSL devices for automatic event notification when a
*             device is removed or inserted.
*Parameters: 
*             HWND hWnd         Current GUI Windows handle
*Returns:
*             unsigned char     BSL_SUCCESS           - Success
*                               BSL_USB_HANDLE_ERROR  - HID device handle error
*******************************************************************************/
extern unsigned char BSL_RegisterUSBDeviceNotification(HWND hWnd);

/*******************************************************************************
*Function:    BSL_UnRegisterUSBDeviceNotification
*Valid Mode:  USB
*Description: Unregisters USB device for Windows notification
*Parameters: 
*             none
*Returns:
*             unsigned char     BSL_SUCCESS           - Success
*                               BSL_USB_HANDLE_ERROR  - HID device handle error
*******************************************************************************/
extern unsigned char BSL_UnRegisterUSBDeviceNotification(void);

/*******************************************************************************
*Function:    BSL_NumOfUSBDevices
*Valid Mode:  USB
*Description: Search for number of USB devices based on a VID and PID
*Parameters: 
*             VID               USB Vendor ID 
*             PID               USB Product ID
*Returns:
*             unsigned long     Number of USB devices
*******************************************************************************/
extern unsigned long BSL_NumOfUSBDevices(unsigned short VID, unsigned short PID);

/*******************************************************************************
*Function:    BSL_get_RX_Buffer
*Valid Mode:  ROM, 5438, Flash
*Description: Reads the current state of dataBuffer
*Parameters: 
*             none
*Returns:
*             dataBuffer        returns current state of RX buffer
*******************************************************************************/
extern dataBuffer BSL_get_RX_Buffer(void);

/*******************************************************************************
*Function:    BSL_SetVerbose
*Valid Mode:  ROM, 5438, Flash
*Description: Sets Verbose mode to on or off for debugging
*Parameters: 
*             verb              1 == verbose mode on
*                               0 == verbose mode off
*Returns:
*             none
*******************************************************************************/
extern void BSL_SetVerbose( unsigned int verb );

/*******************************************************************************
*Function:    BSL_eraseCheck
*Valid Mode:  ROM
*Description: Checks to see there are all 0xFF values starting at {ADDR} for
*             {LENGTH} bytes
*Parameters: 
*             addr              Address of the first byte to check for proper
*                               erasure
*             length            The number of bytes to check
*Returns:
*             unsigned char     BSL_ACK           - Success
*******************************************************************************/
extern unsigned char BSL_eraseCheck( unsigned long int addr, unsigned long int length );

/*******************************************************************************
*Function:    BSL_LoadPC
*Valid Mode:  ROM, 5438, Flash
*Description: Set an address to MSP430’s Program Counter.
*Parameters: 
*             addr              An address to which the MSP430’s Program Counter
*                               will be set and begin program execution
*Returns:
*             unsigned char     BSL_ACK           - Success
*******************************************************************************/
extern unsigned char BSL_LoadPC( unsigned long int addr );

/*******************************************************************************
*Function:    BSL_setMemOffset
*Valid Mode:  ROM
*Description: Sets the memory offset for CPU X devices
*Parameters: 
*             addr              A value that will be appended above the 16 bits
*                               of any subsequent 16 bit addresses. Acceptable
*                               values:
*                                       1
*                                       0
*Returns:
*             unsigned char     BSL_ACK           - Success
*******************************************************************************/
extern unsigned char BSL_setMemOffset( unsigned long int addr );

/*******************************************************************************
*Function:    BSL_CRC_Check
*Valid Mode:  5438, Flash
*Description: Performs a CRC check starting at the given Address over length
*             number of bytes
*Parameters: 
*             none
*Returns:
*             unsigned char     BSL_ACK - dataBuffer is filled correctly
*******************************************************************************/
extern unsigned char BSL_CRC_Check( unsigned long int addr, unsigned long int length );

/*******************************************************************************
*Function:    BSL_massErase
*Valid Mode:  ROM, 5438, Flash
*Description: Causes the BSL to perform a mass erase
*Parameters: 
*             none
*Returns:
*             unsigned char     BSL_ACK - Memory erased successfully
*******************************************************************************/
extern unsigned char BSL_massErase(void);

/*******************************************************************************
*Function:    BSL_toggleInfo
*Valid Mode:  5438, Flash
*Description: Toggles the MSP430 info memory lock
*Parameters: 
*             none
*Returns:
*             unsigned char     BSL_ACK - Success
*******************************************************************************/
extern unsigned char BSL_toggleInfo(void);

/*******************************************************************************
*Function:    BSL_eraseSegment
*Valid Mode:  ROM, 5438, Flash
*Description: Erases a memory segment in Flash where each segment is 512-bytes
*             in size.
*Parameters: 
*             addr              Address of memory segment to be erased
*Returns:
*             unsigned char     BSL_ACK - Success
*******************************************************************************/
extern unsigned char BSL_eraseSegment( unsigned long int addr );

/*******************************************************************************
*Function:    BSL_eraseMainOrInfo
*Valid Mode:  ROM
*Description: Causes the BSL to erase either the main memory, or info memory,
*             depending on the address supplied
*Parameters: 
*             addr              Address of memory segment to be erased
*Returns:
*             unsigned char     BSL_ACK - Success
*******************************************************************************/
extern unsigned char BSL_eraseMainOrInfo( unsigned long int addr );

/*******************************************************************************
*Function:    BSL_RX_Password
*Valid Mode:  ROM, 5438, Flash
*Description: Transmits BSL password to unlock the device. If the password is 
*             wrong, the device automatically triggers a memory mass erase.
*Parameters:
*             data              DataBlock structure containing the password
*Returns:
*             unsigned char     BSL_ACK - Success
*******************************************************************************/
extern unsigned char BSL_RX_Password( DataBlock data );

/*******************************************************************************
*Function:    BSL_changeBaudRate
*Valid Mode:  5438, Flash
*Description: Changes UART baud rate. Default baud rate is 9600bps.
*Parameters: 
*             rate              4800 bps
*                               9600 bps
*                               19200 bps
*                               38400 bps
*                               57600 bps
*                               115200 bps
*Returns:
*             unsigned char     BSL_ACK - Successfully changed
*******************************************************************************/
extern unsigned char BSL_changeBaudRate( unsigned int rate );

/*******************************************************************************
*Function:    BSL_readTI_TextFile
*Description: Requires BSL_openTI_TextForRead() function before using this
*             function. Reads a specified amount of bytes from the TI Text File.
*Parameters: 
*             bytesToRead       Number of bytes to read
*Returns:
*             DataBlock         Reads a specificed amount of bytes and pack
*                               data in a DataBlock structure
*******************************************************************************/
extern DataBlock BSL_readTI_TextFile(int bytesToRead);

/*******************************************************************************
*Function:    BSL_moreDataToRead_TextFile
*Description: Requires BSL_openTI_TextForRead() function before using this
*             function. Checks if more data is available inside a text file.
*Parameters: 
*             none
*Returns:
*             1                 If an EOF has not been hit
*             0                 If an EOF has been hit
*******************************************************************************/
extern int BSL_moreDataToRead_TextFile(void);

/*******************************************************************************
*Function:    BSL_closeTI_Text
*Description: Requires BSL_openTI_TextForRead() function before using this
*             function. Closes an opened TI Text File.
*Parameters: 
*             none
*Returns:
*             none
*
*******************************************************************************/
extern void BSL_closeTI_Text( void );

/*******************************************************************************
*Function:    BSL_openTI_TextForRead
*Description: Opens a TI Text file for reading.
*Parameters: 
*             filename          Filename to be read
*Returns:
*             int               OPERATION_SUCCESSFUL  - Successful to open file
*                               ERROR_OPENING_FILE    - Failed to open file
*******************************************************************************/
extern int BSL_openTI_TextForRead( char *filename );

/*******************************************************************************
*Function:    BSL_readTI_Text
*Description: Creates a DataBlock structure based on an array of TI text 
*             character array. This is mainly used to create Password DataBlock
*Parameters: 
*             dataArray         TI text character array
*             bytesToRead       Number of bytes to read
*Returns:
*             DataBlock         Creates a DataBlock structure based on number of
*                               bytes to read.
*******************************************************************************/
extern DataBlock BSL_readTI_Text(char * dataArray , int bytesToRead);

/*******************************************************************************
*Function:    BSL_getDefaultPass
*Description: Returns a default BSL password
*Parameters: 
*             none
*Returns:
*             DataBlock         Creates a default password DataBlock structure.
*******************************************************************************/
extern DataBlock BSL_getDefaultPass( void );

#ifdef __cplusplus
}
#endif