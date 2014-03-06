#ifndef BACKYARDBRAINS_RECORDINGBAR_H
#define BACKYARDBRAINS_RECORDINGBAR_H

#include "widgets/Widget.h"

namespace BackyardBrains {

class RecordingBar : public Widgets::Widget {
public:
	RecordingBar(Widget *parent = NULL);
	void setActive(bool active);
	bool active() const;
private:
	bool _active;
	unsigned int _startTime;

	void paintEvent();
	void advance();
};

}

#endif
