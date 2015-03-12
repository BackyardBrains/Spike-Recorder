#ifndef BACKYARDBRAINS_CONFIGVIEW_H
#define BACKYARDBRAINS_CONFIGVIEW_H

#include "widgets/Widget.h"
#include "widgets/PushButton.h"

namespace BackyardBrains {

class RecordingManager;
class AudioView;
class ColorDropDownList;

class ConfigView : public Widgets::Widget {
public:
	ConfigView(RecordingManager &mngr, AudioView &audioView, Widget *parent = NULL);
private:
	class SignalCatcher : public sigslot::has_slots<> {
	public:
		SignalCatcher(int virtualDevice, ConfigView *parent) : _virtualDevice(virtualDevice), _parent(parent) {}
		void catchColor(int coloridx) {
			_parent->colorChanged(_virtualDevice, coloridx);
		}
	private:
		int _virtualDevice;
		ConfigView *_parent;
	};

	Widgets::PushButton *_muteCKBox;

	std::vector<SignalCatcher> _catchers;
	std::vector<ColorDropDownList *> _clrs;

	void colorChanged(int virtualDevice, int coloridx);

	RecordingManager &_manager;
	AudioView &_audioView;
	void paintEvent();

	void closePressed();
	void mutePressed();

};

}
#endif
