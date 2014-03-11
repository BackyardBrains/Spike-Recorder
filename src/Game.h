#ifndef BACKYARDBRAINS_WIDGETS_GAME_H
#define BACKYARDBRAINS_WIDGETS_GAME_H

#include "widgets/Application.h"
#include "engine/RecordingManager.h"
#include "engine/FileRecorder.h"

namespace BackyardBrains {

class AudioView;
class RecordingBar;
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
	Widgets::PushButton *_configButton;
	Widgets::ScrollBar *_seekBar;
	Widgets::Widget *_threshavgGroup;
	AudioView *_audioView;
	RecordingBar *_recBar;

	Widgets::Widget *makeThreshavgGroup();

	void loadResources();
	RecordingManager _manager;
	FileRecorder _fileRec;

	void advance();

	/* slots: */
	void pausePressed();
	void backwardPressed();
	void forwardPressed();

	void threshPressed();
	void filePressed();
	void configPressed();
	void recordPressed();
};

} // namespace BackyardBrains

#endif
