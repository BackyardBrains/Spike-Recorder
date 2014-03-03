#ifndef BACKYARDBRAINS_AUDIOVIEW_H
#define BACKYARDBRAINS_AUDIOVIEW_H

#include "widgets/Widget.h"
#include "widgets/ScrollBar.h"
#include "engine/RecordingManager.h"
#include "widgets/Color.h"

namespace BackyardBrains {

class AudioView : public Widgets::Widget
{
public:
	AudioView(Widgets::Widget *parent, RecordingManager *mngr);

	// public slots:
	void updateView(int nchan);
	void setOffset(int offset);
	void setRelOffset(int reloffset); // in per mille
	int offset();

	// signals:
	sigslot::signal1<int> relOffsetChanged;
private:
	struct ViewData {
		float gain;
		float pos;
	};

	static const int MOVEPIN_SIZE = 30;
	static const int RESOLUTION = 800;

	int clickedSlider;
	int clickedPixelOffset;

	int channelOffset;

	RecordingManager *manager;
	std::vector<ViewData> views;

	float displayScale;

	float scaleLenFactor();

	void mousePressEvent(Widgets::MouseEvent *event);
	void mouseMotionEvent(Widgets::MouseEvent *event);
	void mouseReleaseEvent(Widgets::MouseEvent *event);
	void resizeEvent(Widgets::ResizeEvent *e);

	int determineSliderHover(int x, int y);

	float scaleWidth(); //pixels per second of audio
	float screenWidth();
	float sampleCount(float screenw, float scalew);

	void paintEvent();
};


} // namespace BackyardBrains

#endif
