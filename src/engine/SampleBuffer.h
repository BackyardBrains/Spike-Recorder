#ifndef BACKYARDBRAINS_SAMPLEBUFFER_H
#define BACKYARDBRAINS_SAMPLEBUFFER_H

#include <vector>
#include <cstring>
#include <cassert>
#include <stdint.h>
#include <iostream>

namespace BackyardBrains {

class SampleBuffer
{
public:
	static const int64_t SIZE = 44100*60*1;
	static const int SIZE_LOG2 = 21;

	SampleBuffer(int64_t pos = 0) : _pos(pos), _head(0), _buffer(new int16_t[SIZE]), _notEmpty(false)
	{
		memset(_buffer, 0, sizeof(int16_t[SIZE]));
		int size = SIZE/2;
		for (int i = 0; i < SIZE_LOG2; i++, size/=2)
		{
			_envelopes[i].assign(size+1, std::pair<int16_t, int16_t>(0, 0));
		}
	}
	SampleBuffer(const SampleBuffer &other) : _pos(other._pos), _head(other._head), _buffer(new int16_t[SIZE]), _notEmpty(false)
	{
		memcpy(_buffer, other._buffer, sizeof(int16_t[SIZE]));
		for (int i = 0; i < static_cast<int>(SIZE_LOG2); i++)
		{
			_envelopes[i] = other._envelopes[i];
		}
	}
	~SampleBuffer()
	{
		delete [] _buffer;
	}
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
    // Copy data from src to _buffer and
    //For every level of _envelopes find min and max envelope
    //	
	void addData(const int16_t *src, int64_t len)
	{
		if (len > 0)
			_notEmpty = true;
		for (int i = 0; i < len; i++)
		{
			for (int j = 1; j <= SIZE_LOG2; j++)
			{
				const int skipCount = (1 << j);
				const int envelopeIndex = (j-1);
				
                //This envelopeSampleIndex has same value for skipCount consecutive samples.
                //So for every level of envelope resolution (envelopeIndex) we find max and min sample
                //on interval of skipCount consecutive samples and store as one value of envelope
                //at envelopeSampleIndex index				
				const unsigned int envelopeSampleIndex = (_head / skipCount);
				if (envelopeSampleIndex >= _envelopes[envelopeIndex].size())
				{
					// qDebug() << "By accessing _envelopes[" << envelopeIndex << "][" << envelopeSampleIndex << "] w/ size =" << _envelopes[envelopeIndex].size() << "we're out of bounds! _head =" << _head << "SIZE =" << SIZE;
					continue;
				}
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
			_buffer[_head++] = *src++;
			if (_head == SIZE)
				_head = 0;
		}
		_pos += len;
	}
	
    //
    // Just copy data from src interleaved buffer to non-interleaved _buffer buffer
    //for "stride" channel
    //
    void simpleAddData(const int16_t *src, int64_t len, int16_t stride)
    {
        if (len > 0)
            _notEmpty = true;
        for (int i = 0; i < len; i++)
        {
            _buffer[_head++] = *src;
            src = src+stride;
            if (_head == SIZE)
                _head = 0;
        }
       // std::cout<< "**Pos: "<< _pos <<" Len: "" \n";
        _pos += len;
    }	
	
	
	void getData(int16_t *dst, int64_t offset, int64_t len) const
	{
		int64_t j = 0;
		for (int64_t i = offset - _pos; i < (offset - _pos + len); i++, j++)
		{
			if (i < -SIZE || (i >= 0))
				dst[j] = 0;
			else
				dst[j] = _buffer[(_head + i + SIZE)%SIZE];
		}
	}
	void getDataEnvelope(std::pair<int16_t, int16_t> *dst, int64_t offset, int64_t len, int skip) const
	{
		// qDebug() << "SampleBuffer: CALLING getDataEnvelope(<dst>," << offset << "," << len << "," << skip << ") w/ force =" << force;
		const int64_t lllleft  = (offset - _pos);
		const int64_t rrrright = (offset - _pos + len);
		int j = 0;
		for (int64_t i = lllleft; i < rrrright; j++)
		{
			std::pair<int16_t, int16_t> bounding(0, 0);
			if (i >= -SIZE && i + skip <= 0)
			{
				// qDebug() << "Whole thing...";
				// we can process the whole thing
				uint64_t index = (_head + i + SIZE)%SIZE;
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
	int64_t _pos;
	int _head;
	int16_t * const _buffer;
	std::vector<std::pair<int16_t, int16_t> > _envelopes[SIZE_LOG2];
	bool _notEmpty;
};

} // namespace BackyardBrains

#endif
