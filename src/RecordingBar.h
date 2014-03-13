#ifndef BACKYARDBRAINS_RECORDINGBAR_H
#define BACKYARDBRAINS_RECORDINGBAR_H

#include "widgets/Widget.h"

namespace BackyardBrains {

class FileRecorder;
class RecordingBar : public Widgets::Widget {
public:
	RecordingBar(FileRecorder &rec, Widget *parent = NULL);
	void setActive(bool active);
	bool active() const;
private:
	bool _active;
	unsigned int _startTime;
	FileRecorder &_rec;
	
	void paintEvent();
	void advance();
};

}

#endif
