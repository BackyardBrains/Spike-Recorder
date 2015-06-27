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
	setPaused(false);
}

int Player::volume() const {
	float val;
	BASS_ChannelGetAttribute(_output, BASS_ATTRIB_VOL, &val);
	return val*100.f+0.5f;
}

void Player::setVolume(int volume) {
	BASS_ChannelSetAttribute(_output, BASS_ATTRIB_VOL, volume*0.01f);
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
	BASS_ChannelSetAttribute(_output, BASS_ATTRIB_FREQ, sampleRate);
}

void Player::push(void *data, uint32_t size) {
	if(BASS_StreamPutData(_output, data, size) == (DWORD)-1)
		Log::error("Bass Error: putting stream data failed: %s", GetBassStrError());
}

void Player::stop() {
	BASS_StreamFree(_output);
}

}
