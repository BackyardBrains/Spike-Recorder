#ifndef BACKYARDBRAINS_AUDIOVIEW_H
#define BACKYARDBRAINS_AUDIOVIEW_H

#include "widgets/Widget.h"
#include "widgets/ScrollBar.h"
#include "engine/RecordingManager.h"
#include "widgets/Color.h"

namespace BackyardBrains {

struct MetadataChunk;

class AudioView : public Widgets::Widget
{
public:
	struct Channel
	{
		Channel() : virtualDevice(RecordingManager::INVALID_VIRTUAL_DEVICE_INDEX), colorIdx(1), gain(1.f), pos(0.5f) {}

		int virtualDevice;
		int colorIdx;

		float gain;
		float pos;
	};


	static const Widgets::Color COLORS[];
	static const int COLOR_NUM;

	void constructMetadata(MetadataChunk *mdata) const;
	void applyMetadata(const MetadataChunk &mdata);

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
	int offset() const;

	sigslot::signal1<int> relOffsetChanged;
private:
	static const int MOVEPIN_SIZE = 30;
	static const Widgets::Color MARKER_COLORS[];
	static const int MARKER_COLOR_NUM;

	int _clickedGain;
	float _prevGain;
	
	int _clickedSlider;
	int _clickedPixelOffset;

	bool _clickedThresh;

	bool _rulerClicked;
	int _rulerStart;
	int _rulerEnd;

	int64_t _channelOffset;

	RecordingManager &_manager;
	std::vector<Channel> _channels;

	float _timeScale;
	static const float ampScale;

	void clearChannels();

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

	void drawMarkers();
	void drawThreshold(int screenw);
	void drawScale();
	void drawData(std::vector<std::pair<int16_t, int16_t> > &data, int channel, int samples, int x, int y, int width);
};


} // namespace BackyardBrains

#endif
