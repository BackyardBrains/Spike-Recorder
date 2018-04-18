#include "SpikeSorter.h"
#include "Log.h"
#include "BASSErrors.h"
#include <cstring>
#include <bass.h>
#include <cmath>
#include <algorithm>

namespace BackyardBrains {

const std::vector<std::pair<int64_t, int16_t> > &SpikeSorter::spikes(int channel) const {
	return _allSpikeTrains[channel];
}

static int16_t convert_bytedepth(int8_t *pos, int bytes) {
	int16_t res;
	if(bytes == 1) { // unsigned 8 bit format
		res = (*pos-128)<<7;
	} else { // else assume signedness and take the 2 most significant bytes
		memcpy(&res, pos, 2);
	}

	return res;
}

double SpikeSorter::calcRMS(int8_t *buffer, int size, int chan, int channels, int bytedepth, double *mean)
{
	double sum = 0;
	*mean = 0;
	const int nsamples = size/channels/bytedepth;

	for(int i = 0; i < nsamples; i++) {
		const int16_t val = convert_bytedepth(&buffer[(i*channels+chan)*bytedepth], bytedepth);
		*mean += val;
	}

	*mean /= nsamples;

	for(int i = 0; i < nsamples; i++) {
		const int16_t val = convert_bytedepth(&buffer[(i*channels+chan)*bytedepth], bytedepth)-*mean;
		sum += val*val;
	}

	return sqrt(sum/nsamples);
}

int SpikeSorter::findThreshold(int handle, int channel, int channels, int bytedepth, double *meanValue) {
	int64_t left = BASS_ChannelGetLength(handle, BASS_POS_BYTE);
	int8_t buffer[BUFSIZE];

	std::vector<double> rms(left/channels/bytedepth/BUFSIZE);
	unsigned int i = 0;
	while(left > 0) {
		DWORD bytesread = BASS_ChannelGetData(handle, buffer, std::min(left,(int64_t)BUFSIZE));
		if(bytesread == (DWORD)-1) {
			Log::error("Bass Error: getting channel data failed: %s", GetBassStrError());
			break;
		}
        
		double r = calcRMS(buffer, bytesread, channel, channels, bytedepth, meanValue);
		left -= bytesread;

		if(i == rms.size())
			rms.resize(2*rms.size());
		rms[i++] = r;
	}

	rms.resize(i+1);
	std::sort(rms.begin(),rms.end());
	BASS_ChannelSetPosition(handle, 0, BASS_POS_BYTE);
	return rms[rms.size()*4/10];
}

void SpikeSorter::findAllSpikes(const std::string &filename, int holdoff)
{
    HSTREAM handle = BASS_StreamCreateFile(false, filename.c_str(), 0, 0, BASS_STREAM_DECODE);
    if(handle == 0) {
        Log::error("Bass Error: Failed to load file '%s': %s", filename.c_str(), GetBassStrError());
        return;
    }

    
    BASS_CHANNELINFO info;
    BASS_ChannelGetInfo(handle, &info);
    
    BASS_StreamFree(handle);
    
    for(int i=0;i<info.chans;i++)
    {
        _allSpikeTrains.push_back(std::vector<std::pair<int64_t, int16_t> >(0));
    }
    for(int i=0;i<info.chans;i++)
    {
        findSpikes(filename, i, holdoff);
    }

}
    
    
    
      
    
void SpikeSorter::findSpikes(const std::string &filename, int channel, int holdoff) {
	HSTREAM handle = BASS_StreamCreateFile(false, filename.c_str(), 0, 0, BASS_STREAM_DECODE);
	if(handle == 0) {
		Log::error("Bass Error: Failed to load file '%s': %s", filename.c_str(), GetBassStrError());
		return;
	}

	int8_t buffer[BUFSIZE];

	BASS_CHANNELINFO info;
	BASS_ChannelGetInfo(handle, &info);
	int bytespersample = info.origres/8;
    if(bytespersample == 0)
    {
        bytespersample = 2;
    }
	_spikes.reserve(256);

    double meanValue = 0;
	int threshold = findThreshold(handle, channel, info.chans, bytespersample, &meanValue);

	int64_t left = BASS_ChannelGetLength(handle, BASS_POS_BYTE);
	int64_t pos = 0;
    int triggerPositive = 0;
    int triggerNegative = 0;
    int16_t peakvalPositive = 0;
    int16_t peakvalNegative = 0;
    int64_t peakposPositive = 0;
    int64_t peakposNegative = 0;
    
    std::vector<std::pair<int64_t, int16_t> > posspikes, negspikes;
    long long testMaxSize = posspikes.max_size();
	while(left > 0)
    {
		DWORD bytesread = BASS_ChannelGetData(handle, buffer, std::min(left,(int64_t)BUFSIZE));
		if(bytesread == (DWORD)-1) {
			Log::error("Bass Error: getting channel data failed: %s", GetBassStrError());
			break;
    }

		

        const int nsamples = bytesread/info.chans/bytespersample;
        
        //calculate mean value for this part of a signal
        //this can be potential issue with signal that has floating mean value
        long long sumForMean = 0;
        for(int i = 0; i < nsamples; i++) {
            sumForMean += convert_bytedepth(&buffer[(i*info.chans+channel)*bytespersample], bytespersample);
        }
        meanValue = sumForMean/nsamples;
        
        
        int tempDifference = 0;
        // looking for positive peaks
        for(int i = 0; i < nsamples; i++) {
            const int16_t val = convert_bytedepth(&buffer[(i*info.chans+channel)*bytespersample], bytespersample) - meanValue;
            
            if(triggerPositive) {
                if(val < 0) {
                     triggerPositive = 0;
                    
                    //if signal crosses back to zerro
                    //(actualy it crosses mean value since "val" has zero mean value)
                    if(posspikes.size()>0)
                    {
                        //check if there is another peak already detected that is too close to this one
                        tempDifference = peakposPositive - posspikes[posspikes.size()-1].first;
                        if(tempDifference<holdoff)//check if peaks are closser in time than "holdoff"
                        {
                            //if new peak has greater amplitude keep new one and delete last one
                            if(posspikes[posspikes.size()-1].second<(peakvalPositive+meanValue))
                            {
                                posspikes.pop_back();
                                posspikes.push_back(std::make_pair(peakposPositive, peakvalPositive+meanValue));
                            }
                            
                        }
                        else
                        {
                            //if new peak is not too close to last one just add new one
                            posspikes.push_back(std::make_pair(peakposPositive, peakvalPositive+meanValue));
                        }
                    }
                    else
                    {
                        //if this is first detected peak just add the peak to array
                        posspikes.push_back(std::make_pair(peakposPositive, peakvalPositive+meanValue));
                    }
                    
                } else if(val > peakvalPositive) {
                    //if we found sample that has greater value than last potential peak
                    //save the amplitude and position
                    peakvalPositive = val;
                    peakposPositive = pos+i;
                }
            } else if(val > threshold) {
                //if we crossed sensitivity (STD) threshold
                //start searching for positive peak
                triggerPositive = 1;
                peakvalPositive = val;
                peakposPositive = pos+i;
            }
        }
        
       
        
        // looking for negative peaks
        for(int i = 0; i < nsamples; i++) {
            const int16_t val = convert_bytedepth(&buffer[(i*info.chans+channel)*bytespersample], bytespersample)-meanValue;
            if(triggerNegative) {
                if(val > 0) {
                    triggerNegative = 0;
                    
                    if(negspikes.size()>0)
                        
                    {
                        tempDifference = peakposNegative - negspikes[negspikes.size()-1].first;
                        if(tempDifference<holdoff)
                        {
                            if(negspikes[negspikes.size()-1].second>(peakvalNegative+meanValue))
                            {
                                negspikes.pop_back();
                                negspikes.push_back(std::make_pair(peakposNegative, peakvalNegative+meanValue));
                            }
                        }
                        else
                        {
                            negspikes.push_back(std::make_pair(peakposNegative, peakvalNegative+meanValue));
                        }
                        
                    }
                    else
                    {
                        negspikes.push_back(std::make_pair(peakposNegative, peakvalNegative+meanValue));
                    }
                    
                } else if(val < peakvalNegative) {
                    peakvalNegative = val;
                    peakposNegative = i+pos;
                }
            } else if(val < -(threshold)) {
                triggerNegative = 1;
                peakvalNegative = val;
                peakposNegative = i+pos;
            }
        }
        
		pos += bytesread/info.chans/bytespersample;
		left -= bytesread;
	}
    
    
    
    //Spike sorting that:
    //first detect spikes
    //than sort the spikes according to amplitude
    //than eliminate all spikes that are in holdoff interval
    //
    // It is better alghorithm but it is commented because it is slow!
    
    /*
    
    unsigned long tempSizeBigSpikes = posspikes.size()-1;
    unsigned long tempSizeSmallSpikes = posspikes.size();
    int indexOfTempValue;
    std::sort(posspikes.begin(),posspikes.end(), sortPositive);
    for(int indexOfGreatValue = 0;indexOfGreatValue<tempSizeBigSpikes;indexOfGreatValue++)
    {
        
        for(indexOfTempValue = indexOfGreatValue+1;indexOfTempValue<tempSizeSmallSpikes;indexOfTempValue++)
        {
            
            if(std::abs((float)posspikes[indexOfGreatValue].first - posspikes[indexOfTempValue].first)<holdoff)
            {
                posspikes.erase(posspikes.begin()+indexOfTempValue);
                indexOfTempValue--;
                tempSizeBigSpikes = posspikes.size()-1;
                tempSizeSmallSpikes = posspikes.size();
            }
        }
    }
    std::sort(posspikes.begin(),posspikes.end(), sortSpikesBack);
    
    
    
    tempSizeBigSpikes = negspikes.size()-1;
    tempSizeSmallSpikes = negspikes.size();
    
    std::sort(negspikes.begin(),negspikes.end(), sortNegative);
    for(int indexOfGreatValue = 0;indexOfGreatValue<tempSizeBigSpikes;indexOfGreatValue++)
    {
        
        for(indexOfTempValue = indexOfGreatValue+1;indexOfTempValue<tempSizeSmallSpikes;indexOfTempValue++)
        {
            if(std::abs((float)negspikes[indexOfGreatValue].first - negspikes[indexOfTempValue].first)<holdoff)
            {
                negspikes.erase(negspikes.begin()+indexOfTempValue);
                indexOfTempValue--;
                tempSizeBigSpikes = negspikes.size()-1;
                tempSizeSmallSpikes = negspikes.size();
            }
        }
    }
    std::sort(negspikes.begin(),negspikes.end(), sortSpikesBack);
   
    */
    
    
    //Combine negative and positive spikes in one array "_allSpikeTrains"
    
    std::vector<std::pair<int64_t, int16_t> >::iterator itp = posspikes.begin(), itn = negspikes.begin();

    while(itp != posspikes.end() || itn != negspikes.end()) {
        int next = 1;
        if(itp == posspikes.end())
            next = 0;
        else if(itn != negspikes.end())//flip flop between negative and positive peaks
            next = itp->first < itn->first;
        
        
        if(next) {//check next positive
           
            _allSpikeTrains[channel].push_back(*itp);
            itp++;
        } else {//check next negative
            _allSpikeTrains[channel].push_back(*itn);
            itn++;
        }
    }
    
    
  

	BASS_StreamFree(handle);
}

    
bool SpikeSorter::sortPositive(std::pair<int64_t, int16_t> firstSpike, std::pair<int64_t, int16_t> secondSpike) { return  firstSpike.second  > secondSpike.second; }

bool SpikeSorter::sortSpikesBack(std::pair<int64_t, int16_t> firstSpike, std::pair<int64_t, int16_t> secondSpike) { return  firstSpike.first  < secondSpike.first; }
    
bool SpikeSorter::sortNegative(std::pair<int64_t, int16_t> firstSpike, std::pair<int64_t, int16_t> secondSpike) { return  firstSpike.second  < secondSpike.second; }
    
void SpikeSorter::freeSpikes() {
	_spikes.clear();
    _allSpikeTrains.clear();
}

}
