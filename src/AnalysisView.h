#ifndef BACKYARDBRAINS_ANALYSISVIEW_H
#define BACKYARDBRAINS_ANALYSISVIEW_H

#include "widgets/Widget.h"

namespace BackyardBrains {

class RecordingManager;
class AudioView;

class AnalysisView : public Widgets::Widget {
public:
	AnalysisView(RecordingManager &mngr, Widget *parent = NULL);
private:
	AudioView *_audioView;
	RecordingManager &_manager;

	void paintEvent();
};

}
#endif
