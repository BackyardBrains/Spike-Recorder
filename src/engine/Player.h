#ifndef BACKYARDBRAINS_PLAYER_H
#define BACKYARDBRAINS_PLAYER_H

#include <bass.h>

namespace BackyardBrains {

class Player {
public:
	Player();
	~Player();

	int64_t pos() const;
	void setPos(int64_t pos);
	void start(int chans);
	bool paused() const;
	void setPaused(bool paused);

	void push(void *data, uint32_t size);
	void stop();
private:
	int64_t _pos;
	HSTREAM _output;
};

}

#endif
