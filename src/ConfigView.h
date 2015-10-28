#ifndef BACKYARDBRAINS_CONFIGVIEW_H
#define BACKYARDBRAINS_CONFIGVIEW_H

#include "widgets/Widget.h"
#include "widgets/PushButton.h"
#include "DropDownList.h"
#if defined(_WIN32)
    #include "BYBFirmwareVO.h"
#endif

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
		void catchPort(int portidx) {
            _parent->serialPortChanged(_virtualDevice, portidx);
        }
        #if defined(_WIN32)
            void catchFirmwareSelection(int firmwareid) {
                _parent->firmwareSelectionChanged(firmwareid);
            }
        #endif
        void setNumOfChannelsHandler(int selectionNum) {
            _parent->setSerialNumberOfChannels(selectionNum+1);
        }
	private:
		int _virtualDevice;
		ConfigView *_parent;
	};

	Widgets::PushButton *_muteCKBox;

	std::vector<SignalCatcher> _catchers;
	std::vector<ColorDropDownList *> _clrs;

	void colorChanged(int virtualDevice, int coloridx);
    void serialPortChanged(int virtualDevice, int portidx);
    void setSerialNumberOfChannels(int numberOfChannels);

	RecordingManager &_manager;
	AudioView &_audioView;
    DropDownList *serialPortWidget;
    DropDownList * numberOfChannelsWidget;
    Widgets::PushButton *_connectButton;


    //HID usb connection and firmware update
    //Works only on Windows
    #if defined(_WIN32)
        Widgets::PushButton *_hidButton;
        DropDownList * firmwaresWidget;
        void firmwareSelectionChanged(int firmwareid);
        Widgets::PushButton *_updateButton;
        void firmwareUpdatePressed();
        BYBFirmwareVO * selectedFirmware;
        typedef std::list<BYBFirmwareVO> listBYBFirmwareVO;
        void hidConnectPressed();
    #endif


	void paintEvent();
    void connectPressed();

	void closePressed();
	void mutePressed();


};

}
#endif
