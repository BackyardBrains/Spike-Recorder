//
//  SerialPortsScan.hpp
//  SpikeRecorder
//
//  Created by Stanislav Mircic on 9/6/17.
//  Copyright © 2017 BackyardBrains. All rights reserved.
//

#ifndef SerialPortsScan_h
#define SerialPortsScan_h

#include <stdio.h>

#include <list>
#include <string>
#include <stdint.h>
#include <cstdint>




namespace BackyardBrains
{
    int getListOfSerialPorts( std::list<std::string>& listOfPorts);
}


#endif /* SerialPortsScan_hpp */
