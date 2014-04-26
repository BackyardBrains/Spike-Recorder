#ifndef BACKYARDBRAINS_ANALYSISVIEW_H
#define BACKYARDBRAINS_ANALYSISVIEW_H

#include "widgets/Widget.h"
#include "engine/SpikeSorter.h"

namespace BackyardBrains {

class RecordingManager;
class AnalysisAudioView;

class AnalysisView : public Widgets::Widget {
public:
	AnalysisView(RecordingManager &mngr, Widget *parent = NULL);
private:
	AnalysisAudioView *_audioView;
	RecordingManager &_manager;

	SpikeSorter _spikes;
	bool _wasThreshMode;
	
	void paintEvent();
	void closePressed();
	void savePressed();
};

}
#endif
