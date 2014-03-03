#ifndef BACKYARDBRAINS_WIDGETS_FILEDIALOG_H
#define BACKYARDBRAINS_WIDGETS_FILEDIALOG_H

#include <sigslot.h>
#include <string>

/*
 * Cross platform wrapper for file dialogs. For native implementations see
 * src/widgets/native
 */

namespace BackyardBrains {
namespace Widgets {

class FileDialog {
public:
	enum DialogType {
		OpenFile,
		SaveFile
	};

	FileDialog(DialogType type) : _opened(0), _type(type) {}
	int open();

	enum DialogState {
		SUCCESS,
		CANCELED,
		FAILURE,
		RUNNING = -1
	};

	int isOpen() {
		return _opened;
	}

	DialogState getResultState() {
		if(_opened)
			return RUNNING;
		return _state;
	}

	std::string getResultFilename() {
		return _fileName;
	}

private:
	volatile bool _opened;
	DialogState _state;
	DialogType _type;

	std::string _fileName;

	static void *startThread(void *obj);
	static unsigned long _startThread(void *obj); // Windows
	void openDialog();
};

}

}
#endif
