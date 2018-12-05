#include "DefaultConfig.h"
#ifndef XMLCheckResult
	#define XMLCheckResult(a_eResult) if (a_eResult != XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif
using namespace tinyxml2;

namespace BackyardBrains
{
        DefaultConfig::DefaultConfig()
        {
            configVersion = 1;

            firstChannelButton = 3;
            secondChannelButton = 4;
            thirdChannelButton = 6;
        }

        int DefaultConfig::loadDefaults()
        {
           XMLDocument xmlDoc;
           XMLError eResult = xmlDoc.LoadFile(getConfigPath().c_str());
           if (eResult != XML_SUCCESS)
           {
               printf("Config not found %i\n", eResult);
               saveDefaults();
               return 1;
           }

           XMLNode * pRoot = xmlDoc.FirstChild();
           if (pRoot == nullptr) return -1;

           int iOutInt;

           XMLElement * pElement = pRoot->FirstChildElement("ConfigVersion");
           if (pElement == nullptr) return -1;
           eResult = pElement->QueryIntText(&iOutInt);
           if(iOutInt != configVersion)
           {
               //we don't recognize this version of config XML
                return -1;
           }

           pElement = pRoot->FirstChildElement("Channel1Key");
           if (pElement != nullptr)
           {
                eResult = pElement->QueryIntText(&iOutInt);
           }
           firstChannelButton = iOutInt;
           pElement = pRoot->FirstChildElement("Channel2Key");
           if (pElement != nullptr)
           {
                eResult = pElement->QueryIntText(&iOutInt);
           }
           secondChannelButton = iOutInt;
           pElement = pRoot->FirstChildElement("Channel3Key");
           if (pElement != nullptr)
           {
                eResult = pElement->QueryIntText(&iOutInt);
           }
           thirdChannelButton = iOutInt;


           return 0;
        }

        int DefaultConfig::saveDefaults()
        {
            XMLDocument xmlDoc;

            XMLNode * pRoot = xmlDoc.NewElement("Root");
            xmlDoc.InsertFirstChild(pRoot);

            XMLElement * pElement = xmlDoc.NewElement("ConfigVersion");
            pElement->SetText(1);
            pRoot->InsertEndChild(pElement);

            pElement = xmlDoc.NewElement("Channel1Key");
            pElement->SetText(firstChannelButton);
            pRoot->InsertEndChild(pElement);

            pElement = xmlDoc.NewElement("Channel2Key");
            pElement->SetText(secondChannelButton);
            pRoot->InsertEndChild(pElement);

            pElement = xmlDoc.NewElement("Channel3Key");
            pElement->SetText(thirdChannelButton);
            pRoot->InsertEndChild(pElement);

            /*XMLElement * pElement = xmlDoc.NewElement("IntValue");
            pElement->SetText(10);
            pRoot->InsertEndChild(pElement);

            pElement = xmlDoc.NewElement("FloatValue");
            pElement->SetText(0.5f);
            pRoot->InsertEndChild(pElement);

            pElement = xmlDoc.NewElement("Date");
            pElement->SetAttribute("day", 26);
            pElement->SetAttribute("month", "April");
            pElement->SetAttribute("year", 2014);
            pElement->SetAttribute("dateFormat", "26/04/2014");

            pRoot->InsertEndChild(pElement);

            pElement = xmlDoc.NewElement("List");

            for (int i = 0;i<6;i++)
            {
                XMLElement * pListElement = xmlDoc.NewElement("Item");
                pListElement->SetText(i);

                pElement->InsertEndChild(pListElement);
            }
            pRoot->InsertEndChild(pElement);
            */
            std::string path = getConfigPath();
            XMLError eResult = xmlDoc.SaveFile(path.c_str());
            XMLCheckResult(eResult);

            return 0;
        }
}
