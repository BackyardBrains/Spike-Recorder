#ifndef BYBBOOTLOADER_H
#define BYBBOOTLOADER_H
#include <string>
namespace BackyardBrains {
class BYBBootloaderVO
{
    public:
        BYBBootloaderVO();
        int id;
        std::string port;
        std::string name;
    protected:
    private:
};
}
#endif // BYBBOOTLOADER_H
