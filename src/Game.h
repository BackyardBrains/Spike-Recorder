#ifndef BACKYARDBRAINS_WIDGETS_GAME_H
#define BACKYARDBRAINS_WIDGETS_GAME_H

#include "widgets/Application.h"
#include "engine/RecordingManager.h"

namespace BackyardBrains {

class AudioView;
namespace Widgets {
	class PushButton;
	class ScrollBar;
}

class Game : public Widgets::Application
{
public:
	Game();
	~Game();
private:
	Widgets::PushButton *_pauseButton;
	Widgets::ScrollBar *_seekBar;
	AudioView *_audioView;

	void paintEvent();
	void loadResources();
	RecordingManager _manager;

	void advance();

	/* slots: */
	void pausePressed();
	void backwardPressed();
	void forwardPressed();

	void filePressed();
	void configPressed();
	void test(int state, std::string str);
};

} // namespace BackyardBrains

#endif
