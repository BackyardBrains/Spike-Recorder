#include "BSLFirmwareUpdater.h"
#include <sstream>
#include <iostream>
#include <unistd.h>

namespace BackyardBrains {


    //
    // Constructor of BSL Firmware Updater
    //
    BSLFirmwareUpdater::BSLFirmwareUpdater()
    {



    }

    bool BSLFirmwareUpdater::downloadStartUp(bool * massErased )
    {
        unsigned char reply;
        DataBlock blankPass;

       // String^ bslVidPidIndexString = "PID="+myPID+",VID="+myVID+",DEVICEINDEX="+myDeviceIndex;
        std::stringstream sstm;

        sstm << "PID=0x200,VID=0x2047,DEVICEINDEX=0";
       // sstm << "PID=0x3e0,VID=0x2047,DEVICEINDEX=0";
        const std::string& tmp = sstm.str();
        unsigned char* bslVidPidIndex = (unsigned char*)(tmp.c_str());


        // Initialize USB HID BSL
        BSL_setFamily( FAMILY_FLASH );
        BSL_setCom( COM_USB );



        if(BSL_initialize_BSL((unsigned char *) bslVidPidIndex) == BSL_DEVICE_NOT_FOUND) // Allows explicit declaration of a different
        {
            sendErrorNotification("USB device was unplugged.");
            return false;                           // End programming early.
        }
        sendNotification("Sending password", 6);
        // Get Password text
        blankPass = BSL_getDefaultPass();         // Get Blank password

        // Send Password... Causes mass erase if password is wrong!
        reply = BSL_RX_Password(blankPass);
        if(reply)
        {
            // Wrong password provided.
            // Memory is being mass erased. Setting a 1sec delay.
            *massErased = true;
            sendNotification("Mass erase occured.",7);
            Sleep(2000);
            // Memory is blank now. Re-send blank password to access device.
            reply = BSL_RX_Password(blankPass);
            if(reply)
            {
                sendErrorNotification("Password FAILED!");
                return false;
            }
        }


        sendNotification("Password Sent Successfully.",8);
        // Get RAM_BSL Data


        // Send RAM BSL

        sendNotification("Loading BSL and downloading to MSP.",9);
        if(BSL_RX_TXT_File( "bsltext.txt", 1) != BSL_ACK)
        {
            sendErrorNotification("Loading BSL failed!.");
            return false;
        }
        BSL_LoadPC(0x2504);
        sendNotification("Done BSL loading.",10);

        // Close BSL connection
        BSL_close_BSL();
        sendNotification("Waiting to Re-initialize device.",11);
        Sleep(3000);

        // Re-initialize USB BSL
        BSL_setFamily( FAMILY_FLASH );
        BSL_setCom( COM_USB );

        if(BSL_initialize_BSL((unsigned char *) bslVidPidIndex) == BSL_DEVICE_NOT_FOUND) // Allows explicit declaration of a different
        {
            sendErrorNotification("USB device was unplugged.");
            return false;                           // End programming early.
        }

        // Since device is assumed blank now, resend a blank password
        reply = BSL_RX_Password(blankPass);


        sendNotification("Check BSL version.",12);
      // Let's make sure the new RAM BSL has initiated
      // Query for BSL version
      bslVersion = BSL_TX_BSL_Version_String();


      if (!strcmp((const char *) bslVersion, "ERROR"))
      {
            sendErrorNotification("Failed to read version from RAM BSL.");
            return false;                           // End programming early.
      }

      // Password failed after sending RAM_BSL. Trap it here.
      if(reply)
      {
        sendErrorNotification("Password failed after sending RAM BSL.");
        return false;
      }

      // Erase reset vector first
      sendNotification("Erase reset vector.",13);
      reply = BSL_eraseSegment(0xFFFF);

      if (reply != BSL_ACK)
      {
        sendNotification("Failed to erase reset vector.", 14);
      }

      return true;
    }


    void BSLFirmwareUpdater::customSelectedFirmware(const char * firmwareFilename)
    {
        firmwareFilenameGlobal = firmwareFilename;
       //start thread that will periodicaly read HID
        t1 = std::thread(&BSLFirmwareUpdater::updateThread, this, this);
        t1.detach();
    }


 void BSLFirmwareUpdater::updateThread(BSLFirmwareUpdater * ref)
{
      sendNotification("Update starting",1);


        Sleep(1000);
        sendNotification("Update starting",2);
        Sleep(1000);
        sendNotification("Update starting",3);
        Sleep(1000);
        sendNotification("Update starting",4);
        Sleep(1000);
        sendNotification("Update starting",5);

      bool memoryMassErased = false;            // Tracks if memory has been massed erased to prevent
                                                // another segment erase.

      if(!downloadStartUp(&memoryMassErased))
      {
        // Close BSL connection
        BSL_close_BSL();

        sendNotification("Unsuccessful in starting the BSL. Restarting.",5);

        // Failed the first time. Try again
        if(!downloadStartUp(&memoryMassErased))
        {
          // Quit after 2 tries
          sendErrorNotification("Unsuccessful in starting the BSL.");
          return;
        }
      }



      // Erases necessary memory segments
      // If memory is not massed erased, erase individual memory segments
        if(!memoryMassErased)
        {
            sendNotification("Erasing memory segments.",15);
            eraseDataSegment_File( firmwareFilenameGlobal);
        }


        sendNotification("Sending firmware.",16);
        BSL_RX_TXT_File((char *) firmwareFilenameGlobal, 1);
        sendNotification("Firmware sent. Reseting...",17);

      // Run CRC check
     /* worker->ReportProgress(80,"Verifying memory\r\n");
      if( CRC_Check_File((char *) firmwareFilenameGlobal) )
      {
        // Download error
        worker->ReportProgress(85,"Memory verification error\r\n");
        e->Result = false;
        return;
      }
      else
      {
        // Memory verified successfully
        worker->ReportProgress(85,"Memory successfully verified\r\n");
      }

*/
      // Force a BOR
      triggerForcedBOR();
      sendNotification("Done!",19);
}

    bool BSLFirmwareUpdater::triggerForcedBOR()
    {
      std::stringstream sstm;
        sstm << "@0120\n04 A5 \nq";
        const std::string& tmp = sstm.str();
        char* forcedBOR = (char*)(tmp.c_str());


      sendNotification("Resetting Device...",18);
      if( BSL_RX_TXT((char *) forcedBOR, 1) == BSL_ACK )
      {

            // Version 00.06.05.34 and above requires another dummy command to trigger the real BOR.

            std::stringstream sstm;
            sstm << "@0120\n04 A5 \nq";
            const std::string& tmp = sstm.str();
            char* forcedBOR = (char*)(tmp.c_str());
            BSL_RX_TXT((char *) forcedBOR, 1);

            return false;
      }
      else
      {
        return false;
      }
    }


    void BSLFirmwareUpdater::eraseDataSegment_File( const char * fileName )
    {
      DataBlock data;
      int BSL_bufferSize = 512;

      // Open file based on string
      if( BSL_openTI_TextForRead((char *)fileName) == ERROR_OPENING_FILE )
      {
        //return ERROR_OPENING_FILE;
        return;
      }

      // copy line by line until EOF to dataArray
      while( BSL_moreDataToRead_TextFile() )
      {
        data = BSL_readTI_TextFile(BSL_bufferSize);

        // Erase segment after every 512 bytes
        BSL_eraseSegment(data.startAddr);
      }

      BSL_closeTI_Text();

    }

//------------------- Logging and Notifications for user ----------
    //
    // Log progress of BSL
    //
    void BSLFirmwareUpdater::sendNotification(const char * notificationMessage,int stage)
    {
        std::string str(notificationMessage);
        currentStageMessage = str;
        std::cout<<currentStageMessage<<"\n";
        currentStage  = stage;
    }

    //
    // Log error message
    //
    void BSLFirmwareUpdater::sendErrorNotification(const char * errorMessage)
    {
        std::string str(errorMessage);
        errorString = str;
        currentStageMessage = str;
        std::cout<<"Error: "<<errorString<<"\n";
        currentStage  = -1;
    }
}
