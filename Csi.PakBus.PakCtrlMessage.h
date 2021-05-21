/* Csi.PakBus.PakCtrlMessage.h

   Copyright (C) 2001, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 06 March 2001
   Last Change: Wednesday 04 April 2018
   Last Commit: $Date: 2018-04-04 16:43:59 -0600 (Wed, 04 Apr 2018) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Csi_PakBus_PakCtrlMessage_h
#define Csi_PakBus_PakCtrlMessage_h


#include "Csi.PakBus.Message.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class PakCtrlMessage
      //
      // This class extends the PakBus::Message class to add header fields that cover messages
      // delivered in the PakBus PakCtrl layer
      //
      // As a subclass of class Message, PakCtrlMessage objects will have all of the fields that are
      // common to PakBus messages. They can also be passed in any place where a Message parameter
      // is required.
      ////////////////////////////////////////////////////////////
      class PakCtrlMessage: public ::Csi::PakBus::Message
      {
      public:
         ////////////////////////////////////////////////////////////
         // header_len_bytes
         //
         // The total header required by this class
         ////////////////////////////////////////////////////////////
         static uint4 const header_len_bytes;

         ////////////////////////////////////////////////////////////
         // max_body_len
         //
         // The maximum body length that is allowed for PakCtrl packets (taking into account the needs
         // of the larger header).
         ////////////////////////////////////////////////////////////
         static uint4 const max_body_len;
         
         ////////////////////////////////////////////////////////////
         // default constructor
         ////////////////////////////////////////////////////////////
         PakCtrlMessage();

         ////////////////////////////////////////////////////////////
         // construct from a base class
         ////////////////////////////////////////////////////////////
         PakCtrlMessage(Message &other);

         //@group header field access
         ////////////////////////////////////////////////////////////
         // message_type access methods
         ////////////////////////////////////////////////////////////
         typedef PakCtrl::Messages::message_type message_type_code;
         message_type_code get_message_type();
         void set_message_type(message_type_code message_type);

         ////////////////////////////////////////////////////////////
         // transaction_no access methods
         ////////////////////////////////////////////////////////////
         byte get_transaction_no();
         void set_transaction_no(byte transaction_no);
         //@endgroup

         /**
          * @return Overloads the base class version to return true for specific PakCtrl message
          * types.
          */
         virtual bool should_encrypt();
         
      private:
         static uint4 const message_type_start;
         static uint4 const transaction_no_start;
      };
   };
};


#endif
