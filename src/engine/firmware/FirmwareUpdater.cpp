#include "FirmwareUpdater.h"

#include <iostream>
using namespace tinyxml2;

namespace BackyardBrains
{

        FirmwareUpdater::FirmwareUpdater()
        {
            LoadXMLFile();
        }


        //
        //Parse XML file
        //
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
            std::list<BYBFirmwareVO> allFirmwares;
            XMLNode *firmwareNode = firmwaresNode ->FirstChild();
            while(firmwareNode!=NULL)
            {
                if( checkNodeName(firmwareNode, "firmware"))
                {

                    //Parse one Firmware node and put into value-object
                    BYBFirmwareVO * newFirmware = new BYBFirmwareVO();
                    //go through all properties of one firmware
                    XMLElement * idNode = findChildWithName(firmwareNode, "id")->ToElement();
                    if(idNode)
                    {
                        newFirmware->id = atoi(idNode->GetText());
                    }

                    XMLElement * versionNode = findChildWithName(firmwareNode, "version")->ToElement();
                    if(versionNode)
                    {
                        newFirmware->version = std::string( versionNode->GetText());
                    }

                    XMLElement * descNode = findChildWithName(firmwareNode, "desc")->ToElement();
                    if(descNode)
                    {
                        newFirmware->description =std::string( descNode->GetText());
                    }

                    XMLElement * typeNode = findChildWithName(firmwareNode, "type")->ToElement();
                    if(typeNode)
                    {
                        newFirmware->type = std::string(typeNode->GetText());
                    }

                    XMLElement * urlNode = findChildWithName(firmwareNode, "url")->ToElement();
                    if(urlNode)
                    {
                        newFirmware->URL = std::string(urlNode->GetText());
                    }

                    //add to list of firmwares
                    if(newFirmware->id!=0)
                    {
                        allFirmwares.push_back((*newFirmware));
                    }
                }

                firmwareNode = firmwareNode->NextSibling();
            }

            //check if we found some firmwares
            if(allFirmwares.size()==0)
            {
                logError("No firmwares in XML");
                return;
            }

        //------- find our version of software
            XMLNode *softwareNode  = findChildWithName(rootnode, "software");
            if(softwareNode==NULL)
            {
                logError("No softwareNode in XML. Wrong XML format.");
                return;
            }

            XMLNode *buildNode = softwareNode ->FirstChild();
            while(buildNode!=NULL)
            {
                if( checkNodeName(buildNode, "build"))
                {
                    XMLElement * versionElement  = findChildWithName(buildNode, "version")->ToElement();
                    if(versionElement)
                    {
                        if(std::string(versionElement->GetText()).compare(CURRENT_VERSION_STRING)==0)
                        {
                            //found our version
                            XMLNode * tempFirmwaresNode  = findChildWithName(buildNode, "firmwares");
                            if(tempFirmwaresNode==NULL)
                            {
                                logError("No firmwares for our version of software in XML.");
                                return;
                            }
                            //find all the IDs of the firmwares that are compatibile with our version
                            XMLNode *idNode = tempFirmwaresNode ->FirstChild();
                            while(idNode!=NULL)
                            {
                                if( checkNodeName(idNode, "id"))
                                {
                                    XMLElement * idElement = idNode->ToElement();
                                    if(idElement)
                                    {
                                        int idToAdd = atoi(idElement->GetText());


                                        for( listBYBFirmwareVO::iterator ti = allFirmwares.begin();
                                            ti != allFirmwares.end();
                                            ti ++)
                                        {
                                            if(ti->id == idToAdd)
                                            {
                                                BYBFirmwareVO tempBYBFirmware;
                                                tempBYBFirmware.description = ((BYBFirmwareVO)(*ti)).description;
                                                tempBYBFirmware.URL =((BYBFirmwareVO)(*ti)).URL;
                                                tempBYBFirmware.version = ((BYBFirmwareVO)(*ti)).version;
                                                tempBYBFirmware.type = ((BYBFirmwareVO)(*ti)).type;
                                                tempBYBFirmware.id = ((BYBFirmwareVO)(*ti)).id;
                                                firmwares.push_back(tempBYBFirmware);
                                                break;
                                            }
                                        }

                                    }
                                }
                                idNode = idNode->NextSibling();
                            }
                            break;
                        }
                    }
                }

                buildNode = buildNode->NextSibling();
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
            if((std::string(node->Value()).compare(name))==0)
            {
                return true;
            }
            return false;
        }

}
