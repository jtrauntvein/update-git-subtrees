/* Csi.DevConfig.ControlTran.cpp

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 01 March 2012
   Last Change: Wednesday 07 March 2012
   Last Commit: $Date: 2012-03-07 11:33:05 -0600 (Wed, 07 Mar 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.ControlTran.h"


namespace Csi
{
   namespace DevConfig
   {
      ////////////////////////////////////////////////////////////
      // class ControlTran definitions
      ////////////////////////////////////////////////////////////
      ControlTran::ControlTran(action_type action_):
         action(action_),
         security_code(0)
      { }


      ControlTran::~ControlTran()
      { }


      void ControlTran::start(
         SharedPtr<SessionBase> session_, uint2 security_code_)
      {
         if(session == 0)
         {
            message_handle command(new Message);
            
            session = session_;
            security_code = security_code_;
            command->set_message_type(Messages::control_cmd);
            command->addUInt2(security_code);
            command->addByte(action);
            session->add_transaction(this, command, 1, 10000);
         }
      } // start


      void ControlTran::describe_outcome(
         std::ostream &out, outcome_type outcome)
      {
         // @todo: decode the outcome with translated strings
      } // describe_outcome

      
      void ControlTran::on_complete(
         message_handle &command, message_handle &response)
      {
         try
         {
            response->reset();
            byte rcd = response->readByte();
            outcome_type outcome = outcome_unknown;
            if(rcd >= outcome_settings_committed &&
               rcd <= outcome_wifi_scan_started)
               outcome = static_cast<outcome_type>(rcd);
            do_on_complete(outcome);
         }
         catch(std::exception &)
         { do_on_complete(outcome_unknown); }
      } // on_complete


      void ControlTran::do_on_complete(outcome_type outcome)
      {
         session.clear();
         on_complete(outcome);
      } // do_on_complete
      

      void ControlTran::on_failure(
         message_handle &command, failure_type failure)
      {
         outcome_type outcome(outcome_unknown);
         switch(failure)
         {
         case failure_link_failed:
            outcome = outcome_link_failed;
            break;
            
         case failure_timed_out:
            outcome = outcome_timed_out;
            break;
         }
         do_on_complete(outcome);
      }
   };
};


