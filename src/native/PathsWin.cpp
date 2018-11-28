#include "Paths.h"
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
#include <string>
#include <iostream>
#include <sstream>
#include <string>

namespace BackyardBrains {

std::string ExePath() {
    char buffer[MAX_PATH];
    GetModuleFileName( NULL, buffer, MAX_PATH );
    std::string::size_type pos = std::string( buffer ).find_last_of( "\\/" );
    return std::string( buffer ).substr( 0, pos);
}

std::string getRecordingPath() {
	char path[MAX_PATH];

	SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, path);
	std::string res = std::string(path) + "/BYB";
	mkdir(res.c_str());

	return res;
}

std::string getLoggingPath() {

    std::stringstream ss;
    ss << ExePath() << "\\byb.log";
    //std::string s = ss.str();
    return ss.str().c_str();
	//return "%APPDATA%\\BackyardBrains\\SpikeRecorder\\byb.log";
}

std::string getConfigPath() {

    std::stringstream ss;
    ss<<getenv("APPDATA");
    ss <<"\\BackyardBrains\\SpikeRecorder\\config.xml";
    //std::string s = ss.str();
    return ss.str().c_str();
	//return "%APPDATA%\\BackyardBrains\\SpikeRecorder\\byb.log";
}


}
