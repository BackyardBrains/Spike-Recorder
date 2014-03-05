#include "../FileDialog.h"
#include "FileDialogMac.h"
#include <pthread.h>
#include <stdio.h>
#include <sys/wait.h>

namespace BackyardBrains {
namespace Widgets {

/* OS X implementation does not support running in parallel.
 * But that isn’t used at the moment anyway. */

// void *FileDialog::startThread(void *arg) {
// 	reinterpret_cast<FileDialog *>(arg)->openDialog();
// 	return 0;
// }
//
// void FileDialog::openDialog() {
// 	_state = (DialogState)FileDialogMac::openFileDialog(&_fileName);
// 	_opened = false;
// }

int FileDialog::open() {
	pthread_t thread;

	_state = (DialogState)FileDialogMac::openFileDialog(&_fileName, _type);

	return 1;
}


}

}
