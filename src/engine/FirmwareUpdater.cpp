#include "FirmwareUpdater.h"
#include "tinyxml2.h"
using namespace tinyxml2;

namespace BackyardBrains
{

        FirmwareUpdater::FirmwareUpdater()
        {
            LoadXMLFile();
        }

        void FirmwareUpdater::LoadXMLFile()
        {
            XMLDocument doc;
            doc.SaveFile("compatibility.xml");
            XMLNode *rootnode = doc.FirstChild();
            XMLNode *buildsNode = rootnode ->FirstChild();
            while(buildsNode!=NULL)
            {
               /* if(std::string(buildsNode->Value()).compare("firmwares"))
                {
                    break;
                }
                */
                buildsNode->NextSibling();
            }
            if(buildsNode==NULL)
            {
                return;
            }
        }
}
