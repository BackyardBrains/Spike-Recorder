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

void Player::start(int chans) {
	_output = BASS_StreamCreate(44100, chans, 0, STREAMPROC_PUSH, 0);
	BASS_StreamPutData(_output, NULL, 10*44100);
	_pos = 0;
}

bool Player::paused() const {
	return BASS_ChannelIsActive(_output) != BASS_ACTIVE_PLAYING;
}

void Player::setPaused(bool npaused) {
	if(npaused) {
		if(!paused() && !BASS_ChannelPause(_output))
			std::cerr << "Bass Error: pausing channel failed1: " << BASS_ErrorGetCode() << "\n";
	} else {
		if(paused() && !BASS_ChannelPlay(_output, FALSE))
			std::cerr << "Bass Error: resuming channel playback failed: " << BASS_ErrorGetCode() << "\n";
	}
}

void Player::push(void *data, uint32_t size) {
	if(BASS_StreamPutData(_output, data, size) == (DWORD)-1)
		std::cerr << "Bass Error: putting stream data failed: " << BASS_ErrorGetCode() << "\n";
	_pos += size;
}

void Player::stop() {
	BASS_StreamFree(_output);
}

}
