#ifndef BACKYARDBRAINS_WIDGETS_GAME_H
#define BACKYARDBRAINS_WIDGETS_GAME_H

#include "widgets/Application.h"
#include "engine/RecordingManager.h"
#include "engine/FileRecorder.h"
#include "engine/ArduinoSerial.h"

namespace BackyardBrains {

class Game : public Widgets::Application
{
public:
	Game();
	~Game();
private:
	void loadResources();
	RecordingManager _manager;
	FileRecorder _fileRec;
    ArduinoSerial _arduinoSerial;
	void advance();
};

} // namespace BackyardBrains

#endif
