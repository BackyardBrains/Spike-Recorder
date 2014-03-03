#ifndef BACKYARDBRAINS_CONFIGVIEW_H
#define BACKYARDBRAINS_CONFIGVIEW_H

#include "widgets/Widget.h"

namespace BackyardBrains {

class RecordingManager;

class ConfigView : public Widgets::Widget {
public:
	ConfigView(RecordingManager *mngr, Widget *parent = NULL);
private:
	RecordingManager *_manager;
	void paintEvent();

	void closePressed();
};

}
#endif
