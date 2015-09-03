#ifndef BACKYARDBRAINS_THRESHOLDPANEL_H
#define BACKYARDBRAINS_THRESHOLDPANEL_H

#include "widgets/Widget.h"
#include <bass.h>

namespace BackyardBrains {
namespace Widgets {
	class PushButton;
	class SwitchLayout;
	class ScrollBar;
}
class RecordingManager;


class EkgWidget : public Widgets::Widget {
public:
	EkgWidget(Widget *parent = NULL);
	void beat();
	void reset();
	bool sound() const;
	void setSound(bool sound);
private:
	float _frequency;
	unsigned int _lastTime;

	float _beatt;

	bool _sound;
	HSAMPLE _beepSample;

	void paintEvent();
	void advance();
};

class ThresholdPanel : public Widgets::Widget {
public:
	ThresholdPanel(RecordingManager &manager, Widget *parent = NULL);
    BOOL ekgOn();
private:
    RecordingManager* _manager;
    Widgets::PushButton * _triggerButton;
	Widgets::PushButton *_ekgButton;
	Widgets::PushButton *_speakerButton;
	Widgets::ScrollBar *_avg;
	Widgets::SwitchLayout *_switchLayout;
	EkgWidget *_ekgWidget;
    bool triggerOpened = false;

	void ekgPressed();
    void paintEvent();
	void speakerPressed();
    void triggerPressed();

};
}

#endif
