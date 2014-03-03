#include "../FileDialog.h"
#include <pthread.h>
#include <stdio.h>
#include <sys/wait.h>

namespace BackyardBrains {
namespace Widgets {

static FileDialog::DialogState open_dialog_proc(const char *cmd, std::string &fn) {
	FILE *p;
	char buf[256];
	p = popen(cmd, "r");
	FileDialog::DialogState state = FileDialog::SUCCESS;

	if(p == NULL)
		return FileDialog::FAILURE;

	int total = 0;
	size_t n;
	do {
		n = fread(buf, 1, sizeof(buf), p);

		if(n == 0) {
			if(ferror(p))
				state = FileDialog::FAILURE;
			break;
		} else {
			fn.insert(total, buf, n);
			total += n;
		}
	} while(n == sizeof(buf));

	fn[total-1] = 0; // strip the newline

	int rc = pclose(p);
	if(WIFEXITED(rc) && WEXITSTATUS(rc) >= 126) {
		return FileDialog::FAILURE;
	}

	if(total == 0)
		state = FileDialog::CANCELED;

	return state;
}

void *FileDialog::startThread(void *arg) {
	reinterpret_cast<FileDialog *>(arg)->openDialog();
	return 0;
}

void FileDialog::openDialog() {
	const char *opencmds[] = {
		"kdialog --getopenfilename ~ ''",
		"zenity --file-selection",
		"xmessage 'To use the file selection, you must either have 'zenity' or 'kdialog' installed!';exit 127"
	};

	const char *savecmds[] = {
		"kdialog --getsavefilename ~ ''",
		"zenity --file-selection --save --confirm-overwrite",
		"xmessage 'To use the file selection, you must either have 'zenity' or 'kdialog' installed!';exit 127"
	};

	const char **cmds = _type == OpenFile ? opencmds : savecmds;

	for(unsigned int i = 0; i < sizeof(opencmds)/sizeof(opencmds[0]); i++) {
		_state = open_dialog_proc(cmds[i], _fileName);
		if(_state != FAILURE) {
			_opened = false;
			return;
		}
	}

	_opened = false;
}

int FileDialog::open() {
	pthread_t thread;

	if(_opened)
		return -1;

	_opened = true;
	int rc = pthread_create(&thread, NULL, &FileDialog::startThread, this);

	return !rc;
}


}

}
