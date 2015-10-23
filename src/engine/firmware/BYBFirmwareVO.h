#ifndef BYBFIRMWAREVO_H
#define BYBFIRMWAREVO_H
#include <string>
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
    protected:
    private:
};
}
#endif // BYBFIRMWAREVO_H
