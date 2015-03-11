#ifndef BACKYARDBRAINS_ANALYSISVIEW_H
#define BACKYARDBRAINS_ANALYSISVIEW_H

#include "widgets/Widget.h"
#include "engine/SpikeSorter.h"

namespace BackyardBrains {

namespace Widgets {
class Plot;
}
class RecordingManager;
class AnalysisAudioView;

class AnalysisView : public Widgets::Widget {
public:
	AnalysisView(RecordingManager &mngr, Widget *parent = NULL);
private:
	AnalysisAudioView *_audioView;
	RecordingManager &_manager;
	Widgets::Plot *_plot;

	SpikeSorter _spikes;
	bool _wasThreshMode;
	
	void setPlotData();
	void paintEvent();
	void closePressed();
	void savePressed();
};

}
#endif
