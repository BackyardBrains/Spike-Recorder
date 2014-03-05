#ifndef BACKYARDBRAINS_WIDGETS_FILEDIALOGMAC_H
#define BACKYARDBRAINS_WIDGETS_FILEDIALOGMAC_H

#include "../FileDialog.h"

namespace BackyardBrains {
namespace Widgets {
namespace FileDialogMac {

FileDialog::DialogState openFileDialog(std::string *str, FileDialog::DialogType type);

}
}
}

#endif
