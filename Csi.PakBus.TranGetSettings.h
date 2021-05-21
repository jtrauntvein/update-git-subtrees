/* Csi.PakBus.TranGetSettings.h

   Copyright (C) 2002, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 08 May 2002
   Last Change: Friday 30 November 2012
   Last Commit: $Date: 2012-11-30 13:08:23 -0600 (Fri, 30 Nov 2012) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_TranGetSettings_h
#define Csi_PakBus_TranGetSettings_h

#include "Csi.PakBus.PakBusTran.h"
#include "Csi.InstanceValidator.h"
#include "StrAsc.h"
#include <list>
#include <algorithm>


namespace Csi
{
   namespace PakBus
   {
      //@group class forward declarations
      class TranGetSettings;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class TranGetSettingsClient
      ////////////////////////////////////////////////////////////
      class TranGetSettingsClient: public InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_success = 0,
            outcome_communication_failed = 1,
            outcome_unreachable = 2,
            outcome_unsupported = 3, 
         };
         virtual void on_complete(
            TranGetSettings *getter,
            outcome_type outcome,
            StrAsc const &settings) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class TranGetSettings
      ////////////////////////////////////////////////////////////
      class TranGetSettings: public PakBusTran
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         typedef TranGetSettingsClient client_type;
         TranGetSettings(
            Router *router,
            timer_handle &timer,
            priority_type priority,
            uint2 destination_address,
            client_type *client_,
            StrAsc const &setting_names_);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~TranGetSettings(); 

         ////////////////////////////////////////////////////////////
         // parse_settings
         //
         // Implements the algorithm to parse the settings string into
         // a set of name/value pairs.
         ////////////////////////////////////////////////////////////
         typedef std::pair<StrAsc, StrAsc> setting_type;
         typedef std::list<setting_type> settings_type;
         static void parse_settings(settings_type &settings, StrAsc const &s);
         
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
         { out << "PakCtrl::GetSettings"; }

      private:
         ////////////////////////////////////////////////////////////
         // send_command
         ////////////////////////////////////////////////////////////
         void send_command();

      private:
         ////////////////////////////////////////////////////////////
         // retry_count
         ////////////////////////////////////////////////////////////
         uint4 retry_count;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // setting_names
         ////////////////////////////////////////////////////////////
         StrAsc setting_names;
      };
   };
};


#endif
