/* Csi.DevConfig.SettingPoller.h

   Copyright (C) 2008, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 13 November 2008
   Last Change: Wednesday 05 January 2011
   Last Commit: $Date: 2011-01-10 15:54:50 -0600 (Mon, 10 Jan 2011) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_DevConfig_SettingPoller_h
#define Csi_DevConfig_SettingPoller_h

#include "Csi.DevConfig.SessionBase.h"
#include "Csi.DevConfig.Setting.h"


namespace Csi
{
   namespace DevConfig
   {
      ////////////////////////////////////////////////////////////
      // class SettingPoller
      //
      // Defines an object that interacts with a devconfig session in order to
      // refresh the values of certain selected settings from the device. 
      ////////////////////////////////////////////////////////////
      class SettingPoller: public TransactionClient
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SettingPoller();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SettingPoller();

         ////////////////////////////////////////////////////////////
         // add_setting
         ////////////////////////////////////////////////////////////
         typedef SharedPtr<Setting> setting_handle;
         void add_setting(setting_handle &setting);

         ////////////////////////////////////////////////////////////
         // clear_settings
         ////////////////////////////////////////////////////////////
         void clear_settings();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         virtual void start(
            Csi::SharedPtr<SessionBase> session_, uint2 security_code_);
         
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_link_failed = 2,
            outcome_timed_out = 3,
            outcome_partials_not_supported = 4
         };
         virtual void on_complete(outcome_type outcome) = 0;

         ////////////////////////////////////////////////////////////
         // describe_outcome
         ////////////////////////////////////////////////////////////
         static void describe_outcome(std::ostream &out, outcome_type outcome);
         
      protected:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         virtual void on_complete(
            message_handle &command, message_handle &response);

         ////////////////////////////////////////////////////////////
         // on_get_settings_ack
         ////////////////////////////////////////////////////////////
         void on_get_settings_ack(message_handle &command, message_handle &response);

         ////////////////////////////////////////////////////////////
         // on_get_fragment_ack
         ////////////////////////////////////////////////////////////
         void on_get_fragment_ack(message_handle &command, message_handle &response);
         
         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            message_handle &command, failure_type failure);

         ////////////////////////////////////////////////////////////
         // send_next
         ////////////////////////////////////////////////////////////
         void send_next();

         ////////////////////////////////////////////////////////////
         // do_on_complete
         ////////////////////////////////////////////////////////////
         void do_on_complete(outcome_type outcome);
         
      private:
         ////////////////////////////////////////////////////////////
         // session
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<SessionBase> session;
         
         ////////////////////////////////////////////////////////////
         // pending_settings
         ////////////////////////////////////////////////////////////
         typedef std::list<setting_handle> pending_settings_type;
         pending_settings_type pending_settings;

         ////////////////////////////////////////////////////////////
         // partials
         ////////////////////////////////////////////////////////////
         typedef std::pair<setting_handle, message_handle> partial_type;
         typedef std::list<partial_type> partials_type;
         partials_type partials;

         ////////////////////////////////////////////////////////////
         // security_code
         ////////////////////////////////////////////////////////////
         uint2 security_code;
      };
   };
};


#endif