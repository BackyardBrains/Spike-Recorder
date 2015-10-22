#include "FirmwareUpdater.h"

#include <iostream>
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
            if(doc.LoadFile("compatibility.xml")!= XML_NO_ERROR)
            {
                logError("Can not load XML file");
            }
            XMLElement *rootnode = doc.RootElement();


            //Get firmwares node
            XMLNode *firmwaresNode  = findChildWithName(rootnode, "firmwares");
            if(firmwaresNode==NULL)
            {
                logError("No firmwares in XML. Wrong XML format.");
                return;
            }

            //get list of firmwares

            XMLNode *firmwareNode = firmwaresNode ->FirstChild();
            while(firmwareNode!=NULL)
            {
                if( checkNodeName(firmwareNode, "firmware"))
                {

                    //Parse one Firmware node and put into value-object
                    BYBFirmwareVO newFirmware;
                    //go through all properties of one firmware
                    XMLElement * idNode = findChildWithName(firmwareNode, "id")->ToElement();
                    if(idNode)
                    {
                        newFirmware.id = atoi(idNode->GetText());
                    }

                    XMLElement * versionNode = findChildWithName(firmwareNode, "version")->ToElement();
                    if(versionNode)
                    {
                        newFirmware.version = versionNode->GetText();
                    }

                    XMLElement * descNode = findChildWithName(firmwareNode, "desc")->ToElement();
                    if(descNode)
                    {
                        newFirmware.description = descNode->GetText();
                    }

                    XMLElement * typeNode = findChildWithName(firmwareNode, "type")->ToElement();
                    if(typeNode)
                    {
                        newFirmware.type = typeNode->GetText();
                    }

                    XMLElement * urlNode = findChildWithName(firmwareNode, "url")->ToElement();
                    if(urlNode)
                    {
                        newFirmware.URL = urlNode->GetText();
                    }

                    //add to list of firmwares
                    if(newFirmware.id!=0)
                    {
                        firmwares.push_back(newFirmware);
                    }
                }

                firmwareNode = firmwareNode->NextSibling();
            }

            //check if we found some firmwares
            if(firmwares.size()==0)
            {
                logError("No firmwares in XML");
                return;
            }



        }

//============================ Utility functions ========================================
        void FirmwareUpdater::logError(const char * errorMessage)
        {
            std::string str(errorMessage);
            errorString = str;
            std::cout<<"Error: "<<errorString<<"\n";
        }

//================================== XML Utility functions ============================

        XMLNode * FirmwareUpdater::findChildWithName(XMLNode * parentNode, const char * name)
        {
            XMLNode *childNode = parentNode ->FirstChild();
            while(childNode!=NULL)
            {
                if( checkNodeName(childNode, name))
                {
                    break;
                }

                childNode = childNode->NextSibling();
            }
            if(childNode==NULL)
            {
                return NULL;
            }
            return childNode;
        }

        bool FirmwareUpdater::checkNodeName(XMLNode * node, const char * name)
        {
            if(node == NULL)
            {
                return false;
            }
            if(std::string(node->Value()).compare(name))
            {
                return true;
            }
            return false;
        }

}
