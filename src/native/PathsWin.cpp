#include "Paths.h"
#include <windows.h>
#include <shlobj.h>
#include <direct.h>

namespace BackyardBrains {

std::string getRecordingPath() {
	char path[MAX_PATH];

	SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);
	std::string res = std::string(path) + "/BYB";
	mkdir(res.c_str());

	return res;
}

std::string getLoggingPath() {
	return "";
}

}
