#ifndef BACKYARDBRAINS_ANALYSISVIEW_H
#define BACKYARDBRAINS_ANALYSISVIEW_H

#include "widgets/Widget.h"
#include "engine/SpikeSorter.h"
#include "engine/RecordingManager.h"

namespace BackyardBrains {

namespace Widgets {
class PushButton;
class Label;
}
class RecordingManager;
class AnalysisManager;
class AnalysisAudioView;
class AnalysisTrainList;
class AnalysisPlots;

class AnalysisView : public Widgets::Widget {
public:
	AnalysisView(RecordingManager &mngr, AnalysisManager &anaman, Widget *parent = NULL);
private:
	AnalysisAudioView *_audioView;
	RecordingManager &_manager;

	Widgets::PushButton *_plotButton;
	Widgets::Label *_crossLabel;
	AnalysisTrainList *_trainList;
	AnalysisPlots *_plots;
	std::vector<SpikeTrain> &_spikeTrains;

	SpikeSorter _spikeSorter;
	bool _wasThreshMode;

	int _colorCounter;
	
	void setPlotData();
	void paintEvent();
	void closePressed();
	void savePressed();
	void addPressed();
	void plotsPressed();

    void selectionChanged(int idx);
	void threshChanged();
	void trainDeleted(int idx);
	void plotModeChanged(int mode);
    int numberOfSpikeTrainsForCurrentChannel();
};

}
#endif
