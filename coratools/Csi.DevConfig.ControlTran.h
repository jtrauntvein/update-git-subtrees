/* Csi.DevConfig.ControlTran.h

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Thursday 01 March 2012
   Last Change: Thursday 01 March 2012
   Last Commit: $Date: 2012-03-06 15:41:59 -0600 (Tue, 06 Mar 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_ControlTran_h
#define Csi_DevConfig_ControlTran_h

#include "Csi.DevConfig.SessionBase.h"


namespace Csi
{
   namespace DevConfig
   {
      ////////////////////////////////////////////////////////////
      // class ControlTran
      //
      // Defines a component that the application can use to launch
      // a control transaction outside of the purview of the SettingsManager
      // object.
      //
      // In order to use this component, the application mus subclass it and
      // overload the on_complete() method. 
      ////////////////////////////////////////////////////////////
      class ControlTran: public TransactionClient
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         typedef ControlCodes::action_type action_type;
         ControlTran(action_type action_);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ControlTran();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         virtual void start(
            SharedPtr<SessionBase> session_, uint2 security_code_);

         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called to inform the application that the transaction is complete. 
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_link_failed = -1,
            outcome_timed_out = -2,
            outcome_unknown = 0,
            outcome_settings_committed = ControlCodes::outcome_committed,
            outcome_invalid_security_code = ControlCodes::outcome_invalid_security_code,
            outcome_session_ended = ControlCodes::outcome_session_ended,
            outcome_discarded_with_reboot = ControlCodes::outcome_discarded_with_reboot,
            outcome_reverted_to_defaults = ControlCodes::outcome_reverted_to_defaults,
            outcome_session_timer_reset = ControlCodes::outcome_session_timer_reset,
            outcome_locked_by_other = ControlCodes::outcome_locked_by_other,
            outcome_system_error = ControlCodes::outcome_system_error,
            outcome_invalid_action = ControlCodes::outcome_invalid_action,
            outcome_wifi_scan_started = ControlCodes::outcome_wifi_scan_started
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
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            message_handle &command, failure_type failure);

         ////////////////////////////////////////////////////////////
         // do_on_complete
         ////////////////////////////////////////////////////////////
         void do_on_complete(outcome_type outcome);

      private:
         ////////////////////////////////////////////////////////////
         // session
         ////////////////////////////////////////////////////////////
         SharedPtr<SessionBase> session;

         ////////////////////////////////////////////////////////////
         // action
         ////////////////////////////////////////////////////////////
         ControlCodes::action_type action;

         ////////////////////////////////////////////////////////////
         // security_code
         ////////////////////////////////////////////////////////////
         uint2 security_code;
      };
   };
};


#endif
