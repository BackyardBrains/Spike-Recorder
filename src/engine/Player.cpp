#include "Player.h"
#include "BASSErrors.h"
#include "Log.h"

namespace BackyardBrains {

Player::Player() {
}

Player::~Player() {
	stop();
}

void Player::start(int sampleRate) {
	_output = BASS_StreamCreate(sampleRate, 1, 0, STREAMPROC_PUSH, 0);
	setVolume(0);
	//setPaused(false);
	BASS_ChannelPlay(_output, TRUE);
}

uint64_t Player::pos(uint16_t bytesPerSample, uint16_t numOfChannels)
{

    return (BASS_ChannelGetPosition(_output, BASS_POS_BYTE)/(bytesPerSample*numOfChannels));
}

void Player::setPos(uint64_t npos, uint16_t bytesPerSample, uint16_t numOfChannels)
{
    BASS_ChannelSetPosition(_output, bytesPerSample*npos*numOfChannels, BASS_POS_BYTE);
}


int Player::stateOfBuffer()
{
    return BASS_ChannelGetData(_output,NULL,BASS_DATA_AVAILABLE);
}

int Player::volume() const {
	float val;
	BASS_ChannelGetAttribute(_output, BASS_ATTRIB_VOL, &val);
	return val*100.f+0.5f;
}

void Player::setVolume(int volume) {
	
    if(!BASS_ChannelSetAttribute(_output, BASS_ATTRIB_VOL, volume*0.01f))
        Log::error("Bass Error: volume %s", GetBassStrError());
}

bool Player::paused() const {
	return BASS_ChannelIsActive(_output) != BASS_ACTIVE_PLAYING;
}

void Player::setPaused(bool npaused) {
	if(npaused == paused())
		return;

	if(npaused) {
		if(!BASS_ChannelPause(_output))
			Log::error("Bass Error: pausing channel failed: %s", GetBassStrError());
	} else {
		if(!BASS_ChannelPlay(_output, FALSE))
			Log::error("Bass Error: resuming channel playback failed: %s", GetBassStrError());
	}
}

int Player::sampleRate() const {
	float freq;
	BASS_ChannelGetAttribute(_output, BASS_ATTRIB_FREQ, &freq);
	return freq;
}

void Player::setSampleRate(int sampleRate) {
	if(BASS_ChannelSetAttribute(_output, BASS_ATTRIB_FREQ, sampleRate))
    {
        Log::msg("Player::setSampleRate - Sample rate set OK");
    }
    else
    {
        Log::error("Bass Error: setting sample rate", GetBassStrError());
    }
}

void Player::push(void *data, uint32_t size) {
    int buffered = BASS_ChannelGetData(_output, NULL, BASS_DATA_AVAILABLE);
    //Log::msg("In buffer: %d - New: %d", buffered, size);
    
	if(BASS_StreamPutData(_output, data, size) == (DWORD)-1)
		Log::error("Bass Error: putting stream data failed: %s", GetBassStrError());
}

void Player::stop() {
	BASS_StreamFree(_output);
}

}
