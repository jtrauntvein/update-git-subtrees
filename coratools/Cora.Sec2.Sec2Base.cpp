/* Cora.Sec2.Sec2Base.cpp

   Copyright (C) 2002, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 23 December 2002
   Last Change: Monday 04 January 2016
   Last Commit: $Date: 2016-01-04 13:10:08 -0600 (Mon, 04 Jan 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Sec2.Sec2Base.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace Sec2
   {
      ////////////////////////////////////////////////////////////
      // class Sec2Base definitions
      ////////////////////////////////////////////////////////////
      Sec2Base::Sec2Base():
         sec2_session(0),
         open_transaction(0),
         sec2_access_level(AccessLevels::level_root)
      { }
         
      
      Sec2Base::~Sec2Base()
      { finish(); }

      
      void Sec2Base::start(router_handle &router)
      {
         if(state == corabase_state_standby)
            ClientBase::start(router);
         else
            throw exc_invalid_state();
      } // start

      
      void Sec2Base::start(ClientBase *other_client)
      {
         if(state == corabase_state_standby)
            ClientBase::start(other_client);
         else
            throw exc_invalid_state();
      } // start

      
      void Sec2Base::finish()
      {
         if(sec2_session != 0 && router != 0)
         {
            router->closeSession(sec2_session);
            sec2_session = 0;
         }
         ClientBase::finish();
      } // finish

      
      void Sec2Base::on_corabase_ready()
      {
         Csi::Messaging::Message command(
            net_session,
            LgrNet::Messages::open_security2_session_cmd);

         open_transaction = ++last_tran_no;
         command.addUInt4(open_transaction);
         sec2_session = router->openSession(this);
         command.addUInt4(sec2_session);
         router->sendMessage(&command);
      } // on_corabase_ready


      void Sec2Base::format_failure(std::ostream &out, sec2base_failure_type failure)
      {
         switch(failure)
         {
         case sec2base_failure_logon:
            describe_failure(out, corabase_failure_logon);
            break;

         case sec2base_failure_session:
            describe_failure(out, corabase_failure_session);
            break;

         case sec2base_failure_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;

         case sec2base_failure_security:
            describe_failure(out, corabase_failure_security);
            break;
         }
      } // format_failure
      
      
      void Sec2Base::on_corabase_failure(corabase_failure_type failure)
      {
         sec2base_failure_type sec2_failure;
         switch(failure)
         {
         case corabase_failure_logon:
            sec2_failure = sec2base_failure_logon;
            break;
            
         case corabase_failure_session:
            sec2_failure = sec2base_failure_session;
            break;
            
         case corabase_failure_unsupported:
            sec2_failure = sec2base_failure_unsupported;
            break;
            
         case corabase_failure_security:
            sec2_failure = sec2base_failure_security;
            break;
            
         default:
            sec2_failure = sec2base_failure_unknown;
            break;
         }
         on_sec2base_failure(sec2_failure);
      } // on_corabase_failure

      
      void Sec2Base::on_corabase_session_failure()
      {
         on_sec2base_failure(sec2base_failure_session);
      } // on_corabase_session_failure

      
      void Sec2Base::onNetSesBroken(
         Csi::Messaging::Router *rtr,
         uint4 session_no,
         uint4 reason,
         char const *why)
      {
         if(session_no == sec2_session)
         {
            open_transaction = sec2_session = 0;
            on_sec2base_failure(sec2base_failure_session);
         }
         else
            ClientBase::onNetSesBroken(rtr,session_no,reason,why);
      } // onNetSesBroken

      
      void Sec2Base::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(open_transaction != 0 &&
            msg->getMsgType() == LgrNet::Messages::open_security2_session_ack)
         {
            uint4 tran_no;
            uint4 outcome;
            msg->readUInt4(tran_no);
            msg->readUInt4(outcome);
            if(tran_no == open_transaction)
            {
               open_transaction = 0;
               if(outcome == 1)
                  on_sec2base_ready();
               else
               {
                  finish();
                  on_sec2base_failure(sec2base_failure_unknown);
               }
            }
         }
         else if(msg->getMsgType() == Messages::announce_access_level)
         {
            msg->reset();
            msg->readUInt4(sec2_access_level);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage 
   };
};
