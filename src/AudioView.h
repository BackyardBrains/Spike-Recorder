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
	struct Channel
	{
		Channel() : virtualDevice(RecordingManager::INVALID_VIRTUAL_DEVICE_INDEX), coloridx(1), gain(1.f), thresh(0.1f), pos(0.5f) {}

		int virtualDevice;
		int coloridx;

		float gain;
		float thresh;
		float pos;
	};


	static const Widgets::Color COLORS[];
	static const int COLOR_NUM;

	AudioView(Widgets::Widget *parent, RecordingManager &mngr);

	int addChannel(int virtualDeviceIndex);
	void removeChannel(int virtualDeviceIndex);
	void setChannelColor(int channel, int coloridx);
	int channelColor(int channel) const;
	int channelCount() const;
	int channelVirtualDevice(int channel) const;
	int virtualDeviceChannel(int virtualDevice) const;

	void standardSettings();

	void setOffset(int64_t offset);
	void setRelOffset(int reloffset); // in per mille
	int offset();

	bool thresholdMode();
	void toggleThreshMode();

	sigslot::signal1<int> relOffsetChanged;
private:

	static const int MOVEPIN_SIZE = 30;
	static const int RESOLUTION = 800;

	int clickedGain;
	float prevGain;
	
	int clickedSlider;
	int clickedPixelOffset;

	bool clickedThresh;

	int64_t channelOffset;

	RecordingManager &manager;
	std::vector<Channel> channels;

	float timeScale;
	static const float ampScale;

	int selectedChannel() const;
	float scaleLenFactor();

	void mousePressEvent(Widgets::MouseEvent *event);
	void mouseMotionEvent(Widgets::MouseEvent *event);
	void mouseReleaseEvent(Widgets::MouseEvent *event);
	void resizeEvent(Widgets::ResizeEvent *e);

	int determineSliderHover(int x, int y, int *yoffset);
	int determineThreshHover(int x, int y, int *yoffset);

	float thresholdPos();
	float scaleWidth(); //pixels per second of audio
	int screenWidth();
	int sampleCount(int screenw, float scalew);

	void paintEvent();
	void advance();

	void drawThreshold(int screenw);
	void drawScale();
	void drawData(int channel, int samples, float x, float y, float width);
};


} // namespace BackyardBrains

#endif
