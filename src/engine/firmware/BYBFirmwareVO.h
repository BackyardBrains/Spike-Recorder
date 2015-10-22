#ifndef BYBFIRMWAREVO_H
#define BYBFIRMWAREVO_H

namespace BackyardBrains {
class BYBFirmwareVO
{
    public:
        BYBFirmwareVO();
        int id;
        const char * description;
        const char * version;
        const char * type;
        const char * URL;
        const char * filepath;
    protected:
    private:
};
}
#endif // BYBFIRMWAREVO_H
