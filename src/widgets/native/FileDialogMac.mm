#include "FileDialogMac.h"
#include "../FileDialog.h"
#import <Cocoa/Cocoa.h>

namespace BackyardBrains {
namespace Widgets {
namespace FileDialogMac {


FileDialog::DialogState openFileDialog(std::string *fn) {
	NSOpenPanel *panel = [NSOpenPanel openPanel];
	NSInteger res = [panel runModal];
	if (res == NSFileHandlingPanelCancelButton)
		return FileDialog::CANCELED;

	NSURL *url = [panel URL];
	NSString *str = [url absoluteString];
	fn->assign([str UTF8String]);
	return FileDialog::SUCCESS;
}

}
}
}
