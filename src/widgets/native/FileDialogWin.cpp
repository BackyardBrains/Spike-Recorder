#include "../FileDialog.h"
#include <windows.h>
#include <stdio.h>

namespace BackyardBrains {
namespace Widgets {

unsigned long FileDialog::_startThread(void *arg) {
	reinterpret_cast<FileDialog *>(arg)->openDialog();
	return 0;
}

void FileDialog::openDialog() {
	OPENFILENAMEW ofn;
	wchar_t buf[512];
	ZeroMemory(buf, sizeof(buf));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = buf;
	buf[0] = 0;
	ofn.nMaxFile = sizeof(buf);
	ofn.Flags = OFN_NOCHANGEDIR;

	int rc;
	if(_type == OpenFile) {
		ofn.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		rc = GetOpenFileNameW(&ofn);
	} else {
		rc = GetSaveFileNameW(&ofn);
	}

	if(rc) {
		char chbuf[1024];
		WideCharToMultiByte(CP_OEMCP, 0, buf, -1, chbuf, sizeof(chbuf), NULL, NULL);
		_state = SUCCESS;
		_fileName = chbuf;
	} else {
		_state = CANCELED;
	}

	_opened = false;
}

int FileDialog::open() {
	if(_opened)
		return -1;

	_opened = true;
	HANDLE h = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)_startThread,this,0,NULL);

	return h != 0;
}


}

}
