#ifndef BACKYARDBRAINS_AUDIOVIEW_H
#define BACKYARDBRAINS_AUDIOVIEW_H

#include "widgets/Widget.h"
#include "widgets/ScrollBar.h"
#include "engine/RecordingManager.h"
#include "engine/AnalysisManager.h"
#include "widgets/Color.h"

namespace BackyardBrains {

struct MetadataChunk;

class AudioView : public Widgets::Widget
{
public:
	struct Channel
	{
		Channel() : virtualDevice(RecordingManager::INVALID_VIRTUAL_DEVICE_INDEX), colorIdx(1), gain(0.5f) {
			pos = rand()/(float)RAND_MAX;
		}
		void setGain(float ngain);

		int virtualDevice;
		int colorIdx;

		float gain;//channel zoom level (0,1] interval
		float pos;//vertical position on the screen ([0,1] interval)
	};


	static const Widgets::Color COLORS[];
	static const Widgets::Color ARDUINO_COLORS[];
	static const int COLOR_NUM;
	static const Widgets::Color MARKER_COLORS[];
	static const int MARKER_COLOR_NUM;

	void constructMetadata(MetadataChunk *mdata) const;
	void applyMetadata(const MetadataChunk &mdata);

	AudioView(Widgets::Widget *parent, RecordingManager &mngr, AnalysisManager &anaman);
	virtual ~AudioView();

	void setChannelColor(int channel, int coloridx);
	int channelColor(int channel) const;
	int channelCount() const;
	int channelVirtualDevice(int channel) const;
	int virtualDeviceChannel(int virtualDevice) const;
    void navigateFilePosition(bool navigateForward);
    void navigateCurrentRecordingPosition(bool navigateForward);
	int selectedChannel() const;

	float scaleWidth() const; //pixels per second of audio
	int screenWidth() const;
	int sampleCount(int screenw, float scalew) const;

    float calculateCalibrationCoeficient(float timePeriod, float voltagePtoP);

	void standardSettings();

	void setOffset(int64_t offset);
	void setRelOffset(int reloffset); // in per mille
	int offset() const;

	int channelOffset() const;

	sigslot::signal1<int> relOffsetChanged;

	static const int MOVEPIN_SIZE = 30;
	static const int DATA_XOFF = MOVEPIN_SIZE*1.48f;

    void updateChannels();

protected:

	static const int GAINCONTROL_XOFF = MOVEPIN_SIZE*6/5;
	static const int GAINCONTROL_YOFF = MOVEPIN_SIZE*3/5;
	static const int GAINCONTROL_RAD = MOVEPIN_SIZE/4;

	static const float ampScale; // pixels per amplitude unit
	RecordingManager &_manager;
	AnalysisManager &_anaman;
	std::vector<Channel> _channels;

	void drawAudio();
	void drawTimeScale();
	void drawAmplitudeScale();
	void drawSpikeTrain();
	void drawRulerBox();
	void drawRulerTime();
	void drawMarkers(bool fromAnalysisView);
    void drawZeroLine();
	void drawThreshold(int screenw);
	void drawGainControls();
	void drawData(std::vector<std::pair<int16_t, int16_t> > &data, int channel, int samples, int x, int y, int width, int numberOfSamplesToAvoid);
	void paintEvent();
	void drawSpikeTrainStatistics();

	void mousePressEvent(Widgets::MouseEvent *event);
	void mouseMotionEvent(Widgets::MouseEvent *event);
	void mouseReleaseEvent(Widgets::MouseEvent *event);
    void twoFingersPinchEvent(const  SDL_Event &event, int pinchDirection);


	int determineThreshHover(int x, int y, int thresholdPos, int *yoffset);
	int determineGainControlHover(int x, int y);
    int determineGainControlHoverFromAnalysisView(int x, int y);
    void setTimeScale(float newTimesScale);
    void mousePressEventFromAnalysisView(Widgets::MouseEvent *event);

private:
	int _clickedGain;
	float _prevGain;
	float _prevDragX;
	int64_t _prevDragOffset;

	int _clickedSlider;
	int _clickedPixelOffset;

	bool _clickedThresh;

	uint32_t _gainCtrlHoldTime;
	int8_t _gainCtrlDir;

	bool _rulerClicked;
	float _rulerStart;//position of ruler [0,1] interval
	float _rulerEnd;//position of ruler [0,1] interval

    //offset of channel in number of samples (negative value) when
    //return in time to browse past values of samples
	int64_t _channelOffset;

	float _timeScale;

	void clearChannels();

	float scaleLenFactor();

	void resizeEvent(Widgets::ResizeEvent *e);

	int determineSliderHover(int x, int y, int *yoffset);
	float thresholdPos();

	void advance();


    void loadAndApplyAudioInputConfig();//load setings for zoom and timescale
    bool analysisViewIsActive = false;


};


} // namespace BackyardBrains

#endif
