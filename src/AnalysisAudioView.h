#ifndef BACKYARDBRAINS_ANALYSISAUDIOVIEW_H
#define BACKYARDBRAINS_ANALYSISAUDIOVIEW_H

#include "AudioView.h"
#include "engine/SpikeSorter.h"

namespace BackyardBrains {

class SpikeSorter;

/* This is a messy extension of AudioView. It adds a spike display and is thought
 * for paused one channel non-thresh-mode non-live-mode playback. But it doesn’t
 * prevent everything elsefrom happening and that’s why it’s messy.
 */

class AnalysisAudioView : public AudioView {
public:
	AnalysisAudioView(RecordingManager &mngr, SpikeSorter &spikes, Widget *parent = NULL);

	int upperThresh() const;
	int lowerThresh() const;

private:
	SpikeSorter &_spikes;

	int _clickedThresh;
	int _clickOffset;
	float _threshPos[2]; // relative position

	int screenWidth() const;

	int relPosToAmp(float rpos) const;
	void mousePressEvent(Widgets::MouseEvent *event);
	void mouseMotionEvent(Widgets::MouseEvent *event);
	void mouseReleaseEvent(Widgets::MouseEvent *event);

	void drawTargetMarkers();
	void paintEvent();
};

}
#endif
