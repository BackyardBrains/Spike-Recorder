#include "FileDialogMac.h"
#include "../FileDialog.h"
#import <Cocoa/Cocoa.h>

namespace BackyardBrains {
namespace Widgets {
namespace FileDialogMac {


FileDialog::DialogState openFileDialog(std::string *fn, FileDialog::DialogType type) {
	NSURL *url;
	NSString *str;
	if(type == FileDialog::SaveFile) {
		NSPanel *panel = [NSSavePanel savePanel];

		NSInteger res = [panel runModal];
		if (res == NSFileHandlingPanelCancelButton)
			return FileDialog::CANCELED;

		url = [panel URL];
	} else {
		NSPanel *panel = [NSOpenPanel openPanel];

		NSInteger res = [panel runModal];
		if (res == NSFileHandlingPanelCancelButton)
			return FileDialog::CANCELED;

		url = [panel URL];
	}

	str = [url path];
	fn->assign([str UTF8String]);
	return FileDialog::SUCCESS;
}

}
}
}
