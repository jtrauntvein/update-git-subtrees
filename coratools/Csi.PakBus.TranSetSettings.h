/* Csi.PakBus.TranSetSettings.h

   Copyright (C) 2002, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 07 May 2002
   Last Change: Friday 30 November 2012
   Last Commit: $Date: 2012-11-30 13:08:23 -0600 (Fri, 30 Nov 2012) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_TranSetSettings_h
#define Csi_PakBus_TranSetSettings_h

#include "Csi.PakBus.PakBusTran.h"
#include "Csi.InstanceValidator.h"
#include "StrAsc.h"


namespace Csi
{
   namespace PakBus
   {
      //@group class forward declarations
      class TranSetSettings;
      //@endgroup

      
      ////////////////////////////////////////////////////////////
      // class TranSetSettingsClient
      //
      // Defines a the interface for the object that will receive completion event notifications
      // from the setter.
      ////////////////////////////////////////////////////////////
      class TranSetSettingsClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_success = 0,
            outcome_read_only = 1,
            outcome_out_of_space = 2,
            outcome_syntax_error = 3,
            outcome_access_denied = 4,
            outcome_communication_failed = 5,
            outcome_unreachable = 6,
            outcome_unsupported = 7,
         };
         virtual void on_complete(
            TranSetSettings *setter,
            outcome_type outcome,
            uint4 failed_offset) = 0;
      };

      
      ////////////////////////////////////////////////////////////
      // class TranSetSettings
      ////////////////////////////////////////////////////////////
      class TranSetSettings: public PakBusTran
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         typedef TranSetSettingsClient client_type;
         TranSetSettings(
            Router *router,
            timer_handle &timer,
            priority_type priority,
            uint2 destination_address,
            client_type *client_,
            StrAsc const &settings_);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~TranSetSettings();

         ////////////////////////////////////////////////////////////
         // get_settings
         ////////////////////////////////////////////////////////////
         StrAsc const &get_settings() const { return settings; }

      protected:
         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         virtual void start();

         ////////////////////////////////////////////////////////////
         // on_focus_start
         ////////////////////////////////////////////////////////////
         virtual void on_focus_start();
         
         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_pakctrl_message
         ////////////////////////////////////////////////////////////
         virtual void on_pakctrl_message(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // get_transaction_description
         ////////////////////////////////////////////////////////////
         virtual void get_transaction_description(std::ostream &out)
         { out << "PakCtrl::SetSettings"; }

      private:
         ////////////////////////////////////////////////////////////
         // send_command
         ////////////////////////////////////////////////////////////
         void send_command();
         
      private:
         ////////////////////////////////////////////////////////////
         // settings
         //
         // The settings string that should be sent to the destination
         ////////////////////////////////////////////////////////////
         StrAsc settings;

         ////////////////////////////////////////////////////////////
         // retry_count
         ////////////////////////////////////////////////////////////
         uint4 retry_count;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;
      };
   };
};


#endif
