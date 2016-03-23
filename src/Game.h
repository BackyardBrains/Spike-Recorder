#ifndef BACKYARDBRAINS_WIDGETS_GAME_H
#define BACKYARDBRAINS_WIDGETS_GAME_H

#include "widgets/Application.h"
#include "engine/RecordingManager.h"
#include "engine/AnalysisManager.h"
#include "engine/FileRecorder.h"

namespace BackyardBrains {

class Game : public Widgets::Application
{
public:
	Game();
	~Game();
private:
	void loadResources();
	RecordingManager _manager;
	AnalysisManager _anaman;
	FileRecorder _fileRec;
	void advance();
	float roundingDifference;
};

} // namespace BackyardBrains

#endif
