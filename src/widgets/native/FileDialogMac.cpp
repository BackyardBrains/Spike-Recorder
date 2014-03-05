#include "../FileDialog.h"
#include "FileDialogMac.h"
#include <stdio.h>

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
	_state = (DialogState)FileDialogMac::openFileDialog(&_fileName, _type);

	return 1;
}


}

}
