#ifndef BACKYARDBRAINS_SAMPLEBUFFER_H
#define BACKYARDBRAINS_SAMPLEBUFFER_H

#include <vector>
#include <cstring>
#include <cassert>
#include <stdint.h>
#include <iostream>
//==============================================================================
// - This class has circular buffer "_buffer" with raw data for one channel
//   and 21 (SIZE_LOG2) envelopes
// - Each envelope contains subsampled data at different resolution.
// - Each envelope is half the length of previous envelope, namely contains signal at
//   half of the resolution
// - Each envelope contains two arrays. First array contains maximal values of signal and
//   second array contains minimal value of the signal
//
// Values in envelopes are added gradualy as we receive more and more data.
// When circular buffer "_buffer" starts from begining (rewinds) envelopes also start rewriting data
// from the begining
//
// WHole point is to have minimum and maximum on some interval that is phisicaly one pixel
// on the screen so that we can draw vertical line to indicate to user amplitude span of the signal
//
//
//        Look at "AudioView::drawData" it draws vertical lines for each "sample" (subsample)
//        glVertex3i(xc, -data[j].first*_channels[channel].gain*scale+y, 0);
//        glVertex3i(xc, -data[j].second*_channels[channel].gain*scale+y, 0);
//==============================================================================
namespace BackyardBrains {

class SampleBuffer
{
public:
	static const int64_t SIZE = 44100*60*1;
	static const int SIZE_LOG2 = 21;

    //
    //Set initial values of parameters and create envelopes
    //create SIZE_LOG2 (21) envelope arrays. Every envelope is half the length of previous
    //each envelope has two arrays:
    //First array holds maximum values of signal (just subsampled)
    //Second array holds minimum values of signal (just subsampled)
    //
	SampleBuffer(int64_t pos = 0) : _pos(pos), _head(0), _buffer(new int16_t[SIZE]), _notEmpty(false)
	{
		memset(_buffer, 0, sizeof(int16_t[SIZE]));
		int size = SIZE/2;
        //create SIZE_LOG2 (21) envelope arrays.
		for (int i = 0; i < SIZE_LOG2; i++, size/=2)
		{
			_envelopes[i].assign(size+1, std::pair<int16_t, int16_t>(0, 0));
		}
	}
    
    
    //
    // Copy envelopes
    //
	SampleBuffer(const SampleBuffer &other) : _pos(other._pos), _head(other._head), _buffer(new int16_t[SIZE]), _notEmpty(false)
	{
		memcpy(_buffer, other._buffer, sizeof(int16_t[SIZE]));
		for (int i = 0; i < static_cast<int>(SIZE_LOG2); i++)
		{
			_envelopes[i] = other._envelopes[i];
		}
	}
    
    //
    // Destructor
    //
	~SampleBuffer()
	{
		delete [] _buffer;
	}
    
    
    //
    // Assign (copy) envelope
    //
	SampleBuffer & operator=(const SampleBuffer &other)
	{
		_pos = other._pos;
		_head = other._head;
		memcpy(_buffer, other._buffer, sizeof(int16_t[SIZE]));
		for (int i = 0; i < SIZE_LOG2; i++)
		{
			_envelopes[i] = other._envelopes[i];
		}
		_notEmpty = other._notEmpty;
		return *this;
	}
	
    //
    // Look at the explanation at the begining of this file
    //
    // Add raw data from src to circular buffer _buffer and
    // Make envelopes. Check for every sample and every envelope
    // do we have new maximum or minimum.
    //
    // Parameters:
    // src - data from one channel (deinterleaved)
    // len - length of data in samples
    //	
	void addData(const int16_t *src, int64_t len)
	{
		if (len > 0)
			_notEmpty = true;
		for (int i = 0; i < len; i++)
		{
			for (int j = 1; j <= SIZE_LOG2; j++)
			{
				const int skipCount = (1 << j);//this is 2,4,8,....,2^21 = 2097152
				const int envelopeIndex = (j-1);
				
                //This envelopeSampleIndex has same value for skipCount consecutive samples.
                //So for every level of envelope resolution (envelopeIndex) we find max and min sample
                //on interval of skipCount consecutive samples and store as one value of envelope
                //at envelopeSampleIndex index				
				const unsigned int envelopeSampleIndex = (_head / skipCount);//ROUNDING on division!!!!
				
                
                
                if (envelopeSampleIndex >= _envelopes[envelopeIndex].size())
				{
                    //this is basicaly error situation, should not ever happen
					continue;
				}
                
                
                //check if we have new min/max values with this new sample
				std::pair<int16_t, int16_t> &dst = _envelopes[envelopeIndex][envelopeSampleIndex];
				if (_head % skipCount == 0)
				{
				    //if it is first in skipCount consecutive samples
                    //take this to compare with others
					dst = std::pair<int16_t, int16_t>(*src, *src);
				}
				else
				{
				    //if it is not first in skipCount consecutive samples
                    // compare and keep max and min
					dst = std::pair<int16_t, int16_t>(std::min(dst.first, *src), std::max(dst.second, *src));
				}
			}
            
            //add raw data to simple circular buffer
			_buffer[_head++] = *src++;
			if (_head == SIZE)
				_head = 0;
		}
		_pos += len;//add to cumulative number of samples (number of samples since begining of the time)
	}
	
    //
    // Just copy raw data for one channel from interleaved "src" buffer to
    // non-interleaved circular buffer "_buffer"
    // for "stride" channel
    // (just raw data, ignore envelopes)
    // Move the reading head (tail) also by "len" samples
    //
    // Parameters:
    // src - source buffer
    // len - number of frames (or number of samples for single channel)
    // stride - number of channels in one frame
    //
    void simpleAddData(const int16_t *src, int64_t len, int16_t stride)
    {
        if (len > 0)
            _notEmpty = true;
        for (int i = 0; i < len; i++)
        {
            //copy data
            _buffer[_head++] = *src;
            
            //jump to next sample from the same channel
            src = src+stride;
            
            //wrap around circular buffer
            if (_head == SIZE)
                _head = 0;
        }
       
        //move reading head (tail) also
        _pos += len;
    }	
	
	//
    // Parameters:
    //
    //      dst - destination buffer
    //      offset - offset in number of samples since begining of the time
    //      len - number of samples to get
    //
    //   Gets raw data from circular bufer using index "offset" that is given
    //   in number of the samples since begining of the time. Since "_pos" represents
    //   cumulative number of the samples since begining of the time (this buffer has received)
    //   offset must be smaller value (we can't fetch into future)
	void getData(int16_t *dst, int64_t offset, int64_t len) const
	{
		int64_t j = 0;
		for (int64_t i = offset - _pos; i < (offset - _pos + len); i++, j++)
		{
            //(i < -SIZE) - we already owervrite vales
            //(i >= 0) - asking for future values
			if (i < -SIZE || (i >= 0))
				dst[j] = 0;
			else
				dst[j] = _buffer[(_head + i + SIZE)%SIZE];
		}
	}
    
    
    
	void getDataEnvelope(std::pair<int16_t, int16_t> *dst, int64_t offset, int64_t len, int skip) const
	{
		// qDebug() << "SampleBuffer: CALLING getDataEnvelope(<dst>," << offset << "," << len << "," << skip << ") w/ force =" << force;
		const int64_t lllleft  = (offset - _pos);//(negative value)
		const int64_t rrrright = (offset - _pos + len); //(usualy negative value if we don't ask for future)
		int j = 0;
		for (int64_t i = lllleft; i < rrrright; j++)
		{
			std::pair<int16_t, int16_t> bounding(0, 0);
            
            //if (i >= -SIZE) we still have that data in circular buffer
            // (i + skip <= 0) we are not asking for future
			if (i >= -SIZE && i + skip <= 0)
			{
				// qDebug() << "Whole thing...";
				// we can process the whole thing
				uint64_t index = (_head + i + SIZE)%SIZE;// transform index "i" into circular buffer reference frame
				unsigned int remaining = skip;
				bounding = std::pair<int16_t, int16_t>(_buffer[index], _buffer[index]);
				while (remaining > 0)
				{
					// qDebug() << "index =" << index;
					int levels = -1;
					uint64_t multiplier = 1;
					while ((index%(multiplier*2) == 0) && (multiplier*2) <= remaining)
					{
						multiplier *= 2;
						levels++;
					}
					// qDebug() << "levels =" << levels << " multiplier =" << multiplier;
					if (levels >= 0 && levels < SIZE_LOG2 && (index/multiplier) < _envelopes[levels].size())
					{
						// qDebug() << "A";
						// qDebug() << "dst[" << j << "] examines Examining _envelopes[" << (levels-1) << "][" << (index/multiplier) << "]" << _envelopes[levels].size();
						const std::pair<int16_t, int16_t> val = _envelopes[levels][index/multiplier];
						// qDebug() << "OK";
						if (val.first < bounding.first)
							bounding.first = val.first;
						if (val.second > bounding.second)
							bounding.second = val.second;
						index = (index + multiplier)%SIZE;
						remaining -= multiplier;
					}
					else
					{
						// qDebug() << "B";
						const int16_t val = _buffer[index];
						if (val > bounding.second)
							bounding.second = val;
						if (val < bounding.first)
							bounding.first = val;
						index = (index+1)%SIZE;
						remaining--;
					}
					// qDebug() << "OK2";
				}
				// qDebug() << "OK3";
				i += skip;
			}
			else if ((i < -SIZE && i + skip <= -SIZE) || i >= 0)
			{
				// qDebug() << "None...";
				// none of it
				i += skip;
			}
			else
			{
				// qDebug() << "Some...";
				// TODO some of it...
				i += skip;
			}
			//if (j > 1897) qDebug() << "dst[" << j << "] =" << bounding << (len / skip);
			dst[j] = bounding;
			// qDebug() << "zZz";
		}
		// qDebug() << "SampleBuffer: RETURNING";
	}
    
    
    
    
	std::vector<int16_t> getData(int64_t offset, int64_t len) const
	{
		std::vector<int16_t> result(len);
		getData(result.data(), offset, len);
		return result;
	}
    
    //
    // Parameters:
    //    offset - offset in samples from begining of the time
    //    len - number of samples to get
    //    skip - get every "skip" sample (skip "skip"-1 sample after each sample)
    //
    //
    //     returns len/skip data samples
    //
	std::vector< std::pair<int16_t, int16_t> > getDataEnvelope(int64_t offset, int64_t len, int skip) const
	{
		std::vector< std::pair<int16_t, int16_t> > result(len/skip);
		getDataEnvelope(result.data(), offset, len, skip);
		return result;
	}
    
    
	int16_t at(int64_t pos) const
	{
		if (pos <= _pos - SIZE || pos >= _pos)
			return 0;
		return _buffer[(_head + pos - _pos + SIZE)%SIZE];
	}
    
	int64_t pos() const {return _pos;}
    
	void setPos(int64_t pos)
	{
		// qDebug() << "SampleBuffer: SETPOS CALLED";
		_pos = pos;
	}

	int head() const { return _head; }

	void setHead(int head) {
		_head = head%SIZE;
	}

	void reset() {
		_pos = 0;
		_head = 0;
		if(_notEmpty) {
			_notEmpty = false;
			memset(_buffer, 0, SIZE*sizeof(int16_t));

			for (int i = 0, size = SIZE/2; i < SIZE_LOG2; i++, size/=2)
				_envelopes[i].assign(size+1, std::pair<int16_t, int16_t>(0, 0));

		}
	}
	bool empty() const {return !_notEmpty;}
private:
    
    //number of samples since begining of the time (we have to have that since
    //other parts of the application calculate samples from begining of the time
    //and this class has circular buffer that rewinds all the time)
	int64_t _pos;
    
    //position of head in "_buffer" circular buffer
	int _head;
    
    //Circular buffer with raw data. Size is SIZE = 44100*60*1 samples
	int16_t * const _buffer;
    
    //There are SIZE_LOG2 (21) envelope arrays. Every envelope is half the length of previous
    //each envelope has two arrays:
    //First array holds maximum values of signal (just subsampled)
    //Second array holds minimum values of signal (just subsampled)
	std::vector<std::pair<int16_t, int16_t> > _envelopes[SIZE_LOG2];
    
	bool _notEmpty;
};

} // namespace BackyardBrains

#endif
