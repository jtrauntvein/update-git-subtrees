/* Csi.PakBus.PakCtrlMessage.cpp

   Copyright (C) 2001, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 06 March 2001
   Last Change: Wednesday 04 April 2018
   Last Commit: $Date: 2018-04-04 16:43:59 -0600 (Wed, 04 Apr 2018) $ 

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.PakCtrlMessage.h"
#include "Csi.PakBus.Defs.h"


namespace Csi
{
   namespace PakBus
   {
      uint4 const PakCtrlMessage::message_type_start = 0;
      uint4 const PakCtrlMessage::transaction_no_start = 1;
      uint4 const PakCtrlMessage::header_len_bytes = 2;
      uint4 const PakCtrlMessage::max_body_len = Message::max_body_len - header_len_bytes;
      

      PakCtrlMessage::PakCtrlMessage():
         Message(header_len_bytes)
      { }


      PakCtrlMessage::PakCtrlMessage(Message &other):
         Message(other,header_len_bytes)
      { }


      PakCtrlMessage::message_type_code PakCtrlMessage::get_message_type()
      {
         byte temp;
         msg->read(&temp,sizeof(temp),message_type_start,false);
         return static_cast<message_type_code>(temp);
      } // get_message


      void PakCtrlMessage::set_message_type(message_type_code message_type)
      { replaceByte(message_type,message_type_start); }


      byte PakCtrlMessage::get_transaction_no()
      {
         byte temp;
         msg->read(&temp,1,transaction_no_start,false);
         return temp;
      } // get_transaction_no


      void PakCtrlMessage::set_transaction_no(byte transaction_no)
      { replaceByte(transaction_no,transaction_no_start); }

      
      bool PakCtrlMessage::should_encrypt()
      {
         bool rtn(false);
         switch(get_message_type())
         {
         case PakCtrl::Messages::get_settings_cmd:
         case PakCtrl::Messages::get_settings_ack:
         case PakCtrl::Messages::set_settings_cmd:
         case PakCtrl::Messages::set_settings_ack:
         case PakCtrl::Messages::devconfig_get_settings_cmd:
         case PakCtrl::Messages::devconfig_get_settings_ack:
         case PakCtrl::Messages::devconfig_set_settings_cmd:
         case PakCtrl::Messages::devconfig_set_settings_ack:
         case PakCtrl::Messages::devconfig_get_setting_fragment_cmd:
         case PakCtrl::Messages::devconfig_get_setting_fragment_ack:
         case PakCtrl::Messages::devconfig_set_setting_fragment_cmd:
         case PakCtrl::Messages::devconfig_set_setting_fragment_ack:
         case PakCtrl::Messages::devconfig_control_cmd:
         case PakCtrl::Messages::devconfig_control_ack:
            rtn = true;
            break;
         }
         return rtn;
      }
   };
};
