//
//  HDFReader.cpp
//  SpikeRecorder
//
//  Created by Stanislav on 12/05/2022.
//  Copyright © 2022 BackyardBrains. All rights reserved.
//

#include "HDFReader.h"
namespace BackyardBrains {
    HDFReader::HDFReader() {
       
        fileHandle = 0;
        
    }

    HDFReader::~HDFReader() {
       
    }

    bool HDFReader::openHDF5(const char *file, int &nchan, int &samplerate, int &bytespersample)
    {
        lastOffset = 0;
        // Open an existing file and dataset.
        _filename = "";
        _filename.append(file);//keep file name
        
        
        fileHandle = new H5File(file, H5F_ACC_RDWR);
       //test just if we can load audio
        DataSet dataset = fileHandle->openDataSet(DATASET_NAME_AUDIO);

        DataSpace currentSpace = dataset.getSpace();
        int rank = currentSpace.getSimpleExtentNdims();
        hsize_t currentSize[rank];
        currentSpace.getSimpleExtentDims(currentSize);
        nchan = (int)currentSize[0];
        //hsize_t lenghtOfExistingData = currentSize[1];
        bytespersample = 2;
        
        int sampleRateRead = 0;
        Attribute *attr = new Attribute(dataset.openAttribute(HDF_METADATA_SAMPLERATE));
        DataType  *type = new DataType(attr->getDataType());

        attr->read(*type, &sampleRateRead);
        samplerate =sampleRateRead;
        
        
        _fileSampleLenght = currentSize[1];
        _numberOfChannels = nchan;
        _fileSecondsLength = (float)_fileSampleLenght/(float)sampleRateRead;
        _sampleRate = sampleRateRead;
        _bytesPerSample = bytespersample;
        
      /*  DataSet infoDataset = tempFileHandle->openDataSet(DATASET_NAME_INFO);
        DataSpace infoDataspace = infoDataset.getSpace();
        rank = infoDataspace.getSimpleExtentNdims();
        hsize_t dims_out[rank];
        
        int ndims = infoDataspace.getSimpleExtentDims( dims_out, NULL);
        */
        
       


       /* hsize_t      offset[2];    // hyperslab offset in the file
        hsize_t      count[2];    // size of the hyperslab in the file
        
        count[0]  = 1;
        count[1]  = 1;
        
        offset[0] = 0;
        offset[1] = 0;
        infoDataspace.selectHyperslab( H5S_SELECT_SET, count, offset );

        
         // Define the memory dataspace.
         
        hsize_t     dimsm[2];
        dimsm[0] = 1;
        dimsm[1] = 1;
        DataSpace memspace( 2, dimsm );

        
         // Define memory hyperslab.
         
        hsize_t      offset_out[2];    // hyperslab offset in memory
        hsize_t      count_out[2];    // size of the hyperslab in memory
        offset_out[0] = 0;
        offset_out[1] = 0;
        count_out[0]  = 1;
        count_out[1]  = 1;
        memspace.selectHyperslab( H5S_SELECT_SET, count_out, offset_out );

        */
        /*CompType metaParamType(sizeof(MetaParameterStructure));
        H5::StrType stringtype(H5::PredType::C_S1, H5T_VARIABLE);
        metaParamType.insertMember("name", HOFFSET(MetaParameterStructure, name), stringtype);
        metaParamType.insertMember("value", HOFFSET(MetaParameterStructure, value), PredType::NATIVE_FLOAT);
        
  
        //MetaParameterStructure *tempInfoBuffer = new MetaParameterStructure[dims_out[0]*dims_out[1]];
        MetaParameterStructure tempInfoBuffer[10];
        dataset.read( tempInfoBuffer, metaParamType);
        
        */
        
        
        
      /*  DataSet infoDataset = fileHandle->openDataSet(DATASET_NAME_EVENTS);
        DataSpace infoDataspace = infoDataset.getSpace();
        rank = infoDataspace.getSimpleExtentNdims();
        hsize_t dims_out[rank];

        infoDataspace.getSimpleExtentDims( dims_out, NULL);
        
        CompType metaParamType(sizeof(EventStructure));
        H5::StrType stringtype(H5::PredType::C_S1, H5T_VARIABLE);
        
        stringtype.setCset(H5T_CSET_ASCII);
        metaParamType.insertMember("time", HOFFSET(EventStructure, time), PredType::NATIVE_FLOAT);
        metaParamType.insertMember("name", HOFFSET(EventStructure, name), stringtype);
        
  
        std::vector<MetaParameterStructure> data_vector;
        data_vector.resize(dims_out[0]*dims_out[1]);
        dataset.read(data_vector.data(), metaParamType);

      */
        
        
        
        
        
        
        
        
        
        return true;
    }
    

    bool HDFReader::readFile(std::vector<std::vector<int16_t> > &channels, int len)
    {
        
        channels.resize(_numberOfChannels);
        DataSet dataset = fileHandle->openDataSet(DATASET_NAME_AUDIO);
        DataSpace currentSpace = dataset.getSpace();
        
        int lenghtOfDataToRead = len;
        if((lenghtOfDataToRead+lastOffset)>_fileSampleLenght)
        {
            lenghtOfDataToRead =(int)(_fileSampleLenght-lastOffset);          //!!!!maybe overflow
        }
        if(lenghtOfDataToRead<=0)
        {
            return false;
        }

        short * buffer = new short[lenghtOfDataToRead];
         
         
        for(int channel=0;channel<_numberOfChannels;channel++)
        {
                 // Define the hyperslab for file based data.
                 hsize_t data_offset[2];
                 data_offset[0] = channel;
                
                 data_offset[1] = lastOffset;
                 hsize_t data_count[2];
                 data_count[0] = 1;
                 data_count[1] = lenghtOfDataToRead;
                
                
                // Memory dataspace.
                DataSpace mem_space (2, data_count);
                hsize_t mem_offset[2] = { 0, 0 };

                 currentSpace.selectHyperslab(H5S_SELECT_SET, data_count, data_offset);

               
                 mem_space.selectHyperslab(H5S_SELECT_SET, data_count, mem_offset);

                 for(int chan = 0; chan < _numberOfChannels; chan++)
                 {
                     channels[chan].resize(lenghtOfDataToRead);
                 }
                 // Read data to memory.
                 dataset.read(buffer, PredType::NATIVE_SHORT, mem_space, currentSpace);
                 
                
                 
                 for(int chan = 0; chan < _numberOfChannels; chan++) {
                     for(DWORD i = 0; i < len; i++) {
                         channels[chan][i] = buffer[i];
                     }
                 }
        
        }

        lastOffset = lastOffset + lenghtOfDataToRead;
        return true;
    }


    void HDFReader::readEvents()
    {
        
    }
    void HDFReader::readSpikes()
    {
        
    }


}
