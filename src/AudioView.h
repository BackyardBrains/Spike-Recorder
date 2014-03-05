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


	void toggleThreshMode();
	// signals:
	sigslot::signal1<int> relOffsetChanged;
private:
	struct ViewData {
		float gain;
		float thresh;
		float pos;
	};

	static const int MOVEPIN_SIZE = 30;
	static const int RESOLUTION = 800;

	int clickedGain;
	float prevGain;
	
	int clickedSlider;
	int clickedPixelOffset;

	bool clickedThresh;

	int channelOffset;

	int selectedChannel; // in thresh mode
	bool threshMode;

	RecordingManager *manager;
	std::vector<ViewData> views;

	float timeScale;
	static const float ampScale;

	float scaleLenFactor();

	void mousePressEvent(Widgets::MouseEvent *event);
	void mouseMotionEvent(Widgets::MouseEvent *event);
	void mouseReleaseEvent(Widgets::MouseEvent *event);
	void resizeEvent(Widgets::ResizeEvent *e);

	int determineSliderHover(int x, int y, int *yoffset);
	int determineThreshHover(int x, int y, int *yoffset);

	float thresholdPos();
	float scaleWidth(); //pixels per second of audio
	float screenWidth();
	float sampleCount(float screenw, float scalew);

	void paintEvent();

	void drawThreshold(int screenw);
	void drawScale();
	void drawData(int channel, int samples, float x, float y, float width);
};


} // namespace BackyardBrains

#endif
