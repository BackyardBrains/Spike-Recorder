#include "FileDialogMac.h"
#include "../FileDialog.h"
#import <Cocoa/Cocoa.h>

namespace BackyardBrains {
namespace Widgets {
namespace FileDialogMac {


FileDialog::DialogState openFileDialog(std::string *fn, FileDialog::DialogType type) {
    if(type == FileDialog::SaveFile) {
        NSSavePanel * panel = [NSSavePanel savePanel];
        NSInteger res = [panel runModal];
        if (res == NSFileHandlingPanelCancelButton)
            return FileDialog::CANCELED;
        
        NSURL *url = [panel URL];
        NSString *str = [url path];
        fn->assign([str UTF8String]);
        return FileDialog::SUCCESS;
    } else {
        NSOpenPanel * panel = [NSOpenPanel openPanel];
        NSInteger res = [panel runModal];
        if (res == NSFileHandlingPanelCancelButton)
            return FileDialog::CANCELED;
        
        NSURL *url = [panel URL];
        NSString *str = [url path];
        fn->assign([str UTF8String]);
        return FileDialog::SUCCESS;
    }
}

}
}
}
