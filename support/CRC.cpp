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
#include "CRC.h"

CRC::CRC(unsigned short int crcInitValue)
{
  crc = crcInitValue;
}

CRC::~CRC(void)
{
}

/*******************************************************************************
*Function:    CrcInit
*Description: initiailzes the CRC
*Parameters: 
*             unsigned short int init  The CRC init value
*Returns:
*             none
*******************************************************************************/
void CRC::CrcInit(unsigned short int crcInitValue)
{
  crc = crcInitValue;
}

/*******************************************************************************
*Function:    CrcInput
*Description: inputs one byte to the ongoing CRC
*Parameters: 
*             char data             A byte to add to the ongoing CRC
*Returns:
*             none
*******************************************************************************/
void CRC::CrcInput(unsigned char crcData)
{
  unsigned short int x;
  x = ((crc>>8) ^ crcData) & 0xff;
  x ^= x>>4;
  crc = (crc << 8) ^ (x << 12) ^ (x <<5) ^ x;
}

/*******************************************************************************
*Function:    GetCrcValue
*Description: Returns calculated CRC value
*Parameters: 
*             none             
*Returns:
*             unsigned short int Returns CRC value
*******************************************************************************/
unsigned short int CRC::GetCrcValue(void)
{
  return crc;
}