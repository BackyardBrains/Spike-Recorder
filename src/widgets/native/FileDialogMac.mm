#include "FileDialogMac.h"
#include "../FileDialog.h"
#import <Cocoa/Cocoa.h>

namespace BackyardBrains {
namespace Widgets {
namespace FileDialogMac {


FileDialog::DialogState openFileDialog(std::string *fn, FileDialog::DialogType type) {
	NSPanel *panel;
	if(type == FileDialog::SaveFile) {
		panel = [NSSavePanel savePanel];
	} else {
		panel = [NSOpenPanel openPanel];
	}

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
