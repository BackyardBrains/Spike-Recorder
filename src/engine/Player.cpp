#include "Player.h"
#include <iostream>

namespace BackyardBrains {

Player::Player() : _pos(0) {
}

Player::~Player() {
	stop();
}

int64_t Player::pos() const {
	return _pos;
}

void Player::setPos(int64_t pos) {
	BASS_ChannelSetPosition(_output, BASS_POS_BYTE, 0);
	_pos = pos;
}

void Player::start(int sampleRate) {
	_output = BASS_StreamCreate(sampleRate, 1, 0, STREAMPROC_PUSH, 0);
	BASS_ChannelSetAttribute(_output, BASS_ATTRIB_VOL, 0.f);
	setPaused(false);
	_pos = 0;
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
			std::cerr << "Bass Error: pausing channel failed1: " << BASS_ErrorGetCode() << "\n";
	} else {
		if(!BASS_ChannelPlay(_output, FALSE))
			std::cerr << "Bass Error: resuming channel playback failed: " << BASS_ErrorGetCode() << "\n";
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
		std::cerr << "Bass Error: putting stream data failed: " << BASS_ErrorGetCode() << "\n";

	_pos += size/sizeof(int16_t);
}

void Player::stop() {
	BASS_StreamFree(_output);
}

}
