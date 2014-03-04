#include "../FileDialog.h"
#include "FileDialogMac.h"
#include <pthread.h>
#include <stdio.h>
#include <sys/wait.h>

namespace BackyardBrains {
namespace Widgets {


void *FileDialog::startThread(void *arg) {
	reinterpret_cast<FileDialog *>(arg)->openDialog();
	return 0;
}

void FileDialog::openDialog() {
	_state = (DialogState)FileDialogMac::openFileDialog(&_fileName);
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
