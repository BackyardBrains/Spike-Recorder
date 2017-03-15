#ifndef BYBFIRMWAREVO_H
#define BYBFIRMWAREVO_H
#include <string>
#define REMOTE_FIRMWARE 0
#define LOCAL_FIRMWARE 1
namespace BackyardBrains {
class BYBFirmwareVO
{
    public:
        BYBFirmwareVO();
        int id;
        std::string description;
        std::string version;
        std::string type;
        std::string URL;
        std::string filepath;
        std::string hardware;
        int location;
    protected:
    private:
};
}
#endif // BYBFIRMWAREVO_H
