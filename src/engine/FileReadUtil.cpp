#include "FileReadUtil.h"
#include "Log.h"
#include "BASSErrors.h"
#include <bass.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <algorithm>
#include "miniz.h"
#include "Paths.h"
namespace BackyardBrains {



bool openAnyFile(const char *file, HSTREAM &handle, int &nchan, int &samplerate, int &bytespersample)
{
    std::string filenameAndPath(file);
    std::string lowerCasePath;

      // Allocate the destination space
    lowerCasePath.resize(filenameAndPath.size());

      // Convert the source string to lower case
      // storing the result in destination string
    std::transform(filenameAndPath.begin(),
                     filenameAndPath.end(),
                     lowerCasePath.begin(),
                     ::tolower);
    if(lowerCasePath.find(".wav") != std::string::npos)
    {
        return OpenWAVFile(file, handle, nchan, samplerate, bytespersample);
    }
    if(lowerCasePath.find(".byb") != std::string::npos)
    {
        return OpenBYBFile(file, handle, nchan, samplerate, bytespersample);
       
    }
    return true;
}

bool OpenBYBFile(const char *file, HSTREAM &handle, int &nchan, int &samplerate, int &bytespersample)
{
    mz_zip_archive zip_archive;
    mz_bool status;
    size_t uncomp_size;
    void *p;

    //open byb/zip file
    memset(&zip_archive, 0, sizeof(zip_archive));
    status = mz_zip_reader_init_file(&zip_archive, file, 0);
    if (!status)
    {
        printf("mz_zip_reader_init_file() failed!\n");
        return EXIT_FAILURE;
    }

    std::string appWorkingDir = getRecordingPath();
    
    
    //Extract .wav file
    p = mz_zip_reader_extract_file_to_heap(&zip_archive, "signal.wav", &uncomp_size, 0);
    if (!p)
    {
      printf("mz_zip_reader_extract_file_to_heap() failed!\n");
      mz_zip_reader_end(&zip_archive);
      return EXIT_FAILURE;
    }

    FILE *wavfile;
    //std::string zipFileName = std::string(file);
    //size_t dotpos = zipFileName.find_last_of('.');
    std::string wavfilename = appWorkingDir + "/signal.wav";
    wavfile = fopen(wavfilename.c_str(), "wb");
    if(wavfile == 0) {
        return false;
    }
    fwrite(p, uncomp_size, 1, wavfile);
    fclose(wavfile);
    mz_free(p);
    
    
    
    
    //Extract .wav file
    p = mz_zip_reader_extract_file_to_heap(&zip_archive, "signal-events.txt", &uncomp_size, 0);
    if (p)
    {
        FILE *eventsfile;
        std::string eventsfilename = appWorkingDir+ "/signal-events.txt";
        eventsfile = fopen(eventsfilename.c_str(), "wb");
        if(eventsfile == 0) {
            return false;
        }
        fwrite(p, uncomp_size, 1, eventsfile);
        fclose(eventsfile);
        mz_free(p);
    }


    // Close the archive, freeing any resources it was using
    mz_zip_reader_end(&zip_archive);
    
    
    file = wavfilename.c_str();
    OpenWAVFile(file, handle, nchan, samplerate, bytespersample);
    
    return true;
}


bool OpenWAVFile(const char *file, HSTREAM &handle, int &nchan, int &samplerate, int &bytespersample) {
	handle = BASS_StreamCreateFile(false, file, 0, 0, BASS_STREAM_DECODE);
	if(handle == 0) {
		Log::error("Bass Error: Failed to load file '%s': %s", file, GetBassStrError());
		return false;
	}

	BASS_CHANNELINFO info;
	BASS_ChannelGetInfo(handle, &info);

	bytespersample = info.origres/8;
	if(bytespersample == 0) {
		Log::error("Bass Error: Failed to load file '%s': strange bytes per sample", file);
        bytespersample = 2;
		//return false;
	}
    
    if(bytespersample >= 3)//probably float sample
    {
       
        handle = BASS_StreamCreateFile(false, file, 0, 0, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE);
        if(handle == 0) {
            Log::error("Bass Error: Failed to load float file '%s': %s", file, GetBassStrError());
            return false;
        }

        BASS_ChannelGetInfo(handle, &info);
		bytespersample = 4; // bass converts everything it doesn’t support.
    }

	nchan = info.chans;
	samplerate = info.freq;

	return true;
}

const char * readEventsAndSpikesForAnyFile(HSTREAM handle)
{
    if(handle)
    {
        return readEventsAndSpikesForWav(handle);
    }
    return 0;
}



const char * readEventsAndSpikesForWav(HSTREAM handle)
{
    return BASS_ChannelGetTags(handle, BASS_TAG_RIFF_INFO);
}

bool readAnyFile(std::vector<std::vector<int16_t> > &channels, int len, HSTREAM handle, int nchan, int bytespersample)
{
    if(handle)
    {
        return ReadWAVFile(channels, len, handle, nchan, bytespersample);
    }
    return 0;
}



bool ReadWAVFile(std::vector<std::vector<int16_t> > &channels, int len, HSTREAM handle, int nchan, int bytespersample) {
	channels.resize(nchan);
	uint8_t *buffer = new uint8_t[len];
	DWORD samplesRead = BASS_ChannelGetData(handle, buffer, len);
   // long positionInfile = BASS_ChannelGetPosition(handle, BASS_POS_BYTE)/bytespersample;
   // std::cout<<"Current position: "<<positionInfile<<"\n";
	if(samplesRead == (DWORD)-1) {
		Log::error("Bass Error: getting channel data failed: %s", GetBassStrError());
		delete[] buffer;
		return false;
	}

	samplesRead /= bytespersample;

	for(int chan = 0; chan < nchan; chan++)
		channels[chan].resize(samplesRead/nchan);

	// de-interleave the channels
	for(DWORD i = 0; i < samplesRead/nchan; i++) {
		for(int chan = 0; chan < nchan; chan++) {
			if(bytespersample == 1) { // unsigned 8 bit format
				channels[chan][i] = (buffer[i*nchan + chan]-128)<<7;
			} else { // else assume signedness and take the 2 most significant bytes
                if(bytespersample==4)
                {//if it is 4 byte we assume that it is float and multiply it by 32000 to convert from [-1,1] to [-32000,32000]
                    float tempBuffer;
                   memcpy(&tempBuffer, buffer+(i*nchan + chan)*bytespersample, 4);
                    channels[chan][i] = tempBuffer*32766;
                }
                else
                {
                    memcpy(&channels[chan][i], buffer+(i*nchan + chan)*bytespersample, 2);
                }
    
			}
		}
	}
	delete[] buffer;
	return true;
}
	

int64_t anyFilesLength(HSTREAM handle, int bytespersample, int channels)
{
    if(handle)
    {
        return  BASS_ChannelGetLength(handle, BASS_POS_BYTE)/bytespersample/channels;
    }
    return 0;
}


}//namespace BYB

