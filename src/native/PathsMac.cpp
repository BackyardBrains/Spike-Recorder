#include "../Paths.h"
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

namespace BackyardBrains {

std::string getRecordingPath() {
	struct passwd *p = getpwuid(getuid());
	std::string res;

	if(p == NULL)
		res = std::string(getenv("HOME"));
	else
		res = std::string(p->pw_dir);

    res += "/Music/BYB";
	mkdir(res.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	return res;
}

std::string getLoggingPath() {
	return "";
}

}
