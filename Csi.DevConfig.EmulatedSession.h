/* Csi.DevConfig.EmulatedSession.h

   Copyright (C) 2010, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 29 June 2010
   Last Change: Friday 08 December 2017
   Last Commit: $Date: 2017-12-08 18:05:03 -0600 (Fri, 08 Dec 2017) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_DevConfig_EmulatedSession_h
#define Csi_DevConfig_EmulatedSession_h

#include "Csi.DevConfig.SessionBase.h"
#include "Csi.DevConfig.ConfigSummary.h"
#include "Csi.Events.h"
#include <list>


namespace Csi
{
   namespace DevConfig
   {
      ////////////////////////////////////////////////////////////
      // class EmulatedSession
      //
      // Defines a devconfig session derivative that emulates a read-only
      // devconfig protocol session using the contents of a device settings
      // summary file.
      ////////////////////////////////////////////////////////////
      class EmulatedSession:
         public SessionBase,
         public EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         typedef SharedPtr<ConfigSummary> summary_handle;
         EmulatedSession(summary_handle &summary_);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~EmulatedSession();

         ////////////////////////////////////////////////////////////
         // add_transaction
         ////////////////////////////////////////////////////////////
         virtual void add_transaction(
            TransactionClient *client,
            message_handle command,
            uint4 max_retry_count,
            uint4 extra_timeout_interval,
            byte tran_no = 0);

         ////////////////////////////////////////////////////////////
         // supports_reset
         ////////////////////////////////////////////////////////////
         virtual bool supports_reset();

         ////////////////////////////////////////////////////////////
         // is_emulation
         ////////////////////////////////////////////////////////////
         virtual bool is_emulation() const
         { return true; }

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(SharedPtr<Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // on_get_settings_command
         ////////////////////////////////////////////////////////////
         void on_get_settings_command(
            TransactionClient *client, message_handle &command);

         ////////////////////////////////////////////////////////////
         // on_get_setting_fragment_command
         ////////////////////////////////////////////////////////////
         void on_get_setting_fragment_command(
            TransactionClient *client, message_handle &command);
         
         ////////////////////////////////////////////////////////////
         // on_set_settings_command
         ////////////////////////////////////////////////////////////
         void on_set_settings_command(
            TransactionClient *client, message_handle &command);

         ////////////////////////////////////////////////////////////
         // on_set_setting_fragment_command
         ////////////////////////////////////////////////////////////
         void on_set_setting_fragment_command(
            TransactionClient *client, message_handle &command);

         ////////////////////////////////////////////////////////////
         // on_control_command
         ////////////////////////////////////////////////////////////
         void on_control_command(
            TransactionClient *client, message_handle &command);
         
      private:
         ////////////////////////////////////////////////////////////
         // summary
         ////////////////////////////////////////////////////////////
         summary_handle summary;

         ////////////////////////////////////////////////////////////
         // last_tran_no
         ////////////////////////////////////////////////////////////
         byte last_tran_no;

         /**
          * Specifies the collection of settings that were loaded from the summary and sorted by
          * their identifiers
          */
         typedef SharedPtr<Setting> setting_handle;
         typedef std::deque<setting_handle> settings_type;
         settings_type settings;
      };
   };
};


#endif
