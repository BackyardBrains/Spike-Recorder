#include "FileDialogMac.h"
#include "FileDialog.h"

namespace BackyardBrains {
namespace Widgets {
namespace FileDialogMac {


- (FileDialog::DialogState)openFileDialog: (std::string *) res {
	NSOpenPanel *panel = [NSOpenPanel openPanel];
	NSInteger res = [zOpenPanel runModal];
	if (res == NSFileHandlingPanelCancelButton)
		return 1;

	NSURL *url = [panel URL];
	NSString *str = [url absoluteString];
	res->assign([str UTF8String]);
	return 1;
}

}
}
}
