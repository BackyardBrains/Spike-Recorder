#include "Paths.h"
#include <windows.h>
#include <shlobj.h>
#include <direct.h>

namespace BackyardBrains {

std::string getRecordingPath() {
	char path[MAX_PATH];

	SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, path);
	std::string res = std::string(path) + "/BYB";
	mkdir(res.c_str());

	return res;
}

std::string getLoggingPath() {
	return "%APPDATA%/BackyardBrains/SpikeRecorder/byb.log";
}

}
