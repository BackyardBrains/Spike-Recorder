#ifndef BACKYARDBRAINS_ANALYSISVIEWTRAINLIST_H
#define BACKYARDBRAINS_ANALYSISVIEWTRAINLIST_H

#include "widgets/Widget.h"
#include "AnalysisView.h"
#include "RecordingManager.h"

namespace BackyardBrains {

class AnalysisTrainList : public Widgets::Widget {
public:
	AnalysisTrainList(RecordingManager &mngr, std::vector<SpikeTrain> &spikeTrains, Widgets::Widget *parent = NULL);
	int selectedTrain() const;
	void updateSize();
	sigslot::signal1<int> selectionChange;
	sigslot::signal1<int> trainDeleted;
    
    int selectedChannel();
    void setInitialSelection();
private:
    RecordingManager &_manager;
    
	static const int PADDING = 2;
	static const int FIELDW = 30;
	static const int FIELDH = 22;
	static const int REMOVERAD = 6;
	static const int REMOVEOFF = 14;
    static const int CHANNEL_HEADER_SIZE = 30;
	std::vector<SpikeTrain> &_spikeTrains;

	int _selectedTrain;
    int _selectedChannel;

	void paintEvent();
	void mousePressEvent(Widgets::MouseEvent *event);
};

}
#endif
