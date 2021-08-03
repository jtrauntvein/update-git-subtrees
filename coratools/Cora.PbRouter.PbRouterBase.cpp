/* Cora.PbRouter.PbRouterBase.cpp

   Copyright (C) 2002, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 10 June 2002
   Last Change: Tuesday 08 March 2005
   Last Commit: $Date: 2019-11-22 15:34:57 -0600 (Fri, 22 Nov 2019) $ (UTC)
   Last Changed by: $Author: amortenson $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.PbRouter.PbRouterBase.h"
#include "Cora.LgrNet.Defs.h"
#include "Cora.Sec2.Defs.h"
#include "coratools.strings.h" 


namespace Cora
{
   namespace PbRouter
   {
      ////////////////////////////////////////////////////////////
      // class PbRouterBase definitions
      ////////////////////////////////////////////////////////////
      PbRouterBase::PbRouterBase():
         pbrouter_session(0),
         pbrouterbase_state(pbrouterbase_state_standby),
         pbrouter_address(0),
         pbr_access_level(Sec2::AccessLevels::level_root)
      { } 
      

      PbRouterBase::~PbRouterBase()
      { finish(); }


      void PbRouterBase::set_pakbus_router_name(StrUni const &pakbus_router_name_)
      {
         if(pbrouterbase_state == pbrouterbase_state_standby)
            pakbus_router_name = pakbus_router_name_;
         else
            throw exc_invalid_state();
      } // set_pakbus_router_name

      
      void PbRouterBase::start(router_handle &router)
      {
         if(pbrouterbase_state == pbrouterbase_state_standby)
         {
            if(pakbus_router_name.length() != 0)
            {
               pbrouterbase_state = pbrouterbase_state_delegate;
               ClientBase::start(router);
            }
            else
               throw std::invalid_argument("PakBus Router name not specified");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void PbRouterBase::start(ClientBase *other_client)
      {
         if(pbrouterbase_state == pbrouterbase_state_standby)
         {
            if(pakbus_router_name.length() != 0)
            {
               pbrouterbase_state = pbrouterbase_state_delegate;
               ClientBase::start(other_client);
            }
            else
               throw std::invalid_argument("PakBus Router name not specified");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void PbRouterBase::finish()
      {
         if(router != 0 && pbrouter_session)
         {
            router->closeSession(pbrouter_session);
            pbrouter_session = 0;
         }
         pbrouter_address = 0;
         pbrouterbase_state = pbrouterbase_state_standby;
         ClientBase::finish();
      } // finish


      void PbRouterBase::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(pbrouterbase_state == pbrouterbase_state_attach)
         {
            if(msg->getMsgType() == Cora::LgrNet::Messages::open_pakbus_router_session_ack ||
               msg->getMsgType() == Cora::LgrNet::Messages::open_named_pakbus_router_session_ack)
            {
               // read the acknowledgement
               uint4 tran_no;
               uint4 outcome;
               msg->readUInt4(tran_no);
               msg->readUInt4(outcome);

               // process the ack
               switch(outcome)
               {
               case 1:
                  pbrouterbase_state = pbrouterbase_state_ready;
                  on_pbrouterbase_ready();
                  break;
                  
               case 2:
                  on_pbrouterbase_failure(pbrouterbase_failure_invalid_router_id);
                  finish();
                  break;
                  
               default:
                  on_pbrouterbase_failure(pbrouterbase_failure_unknown);
                  finish();
                  break;
               }
            }
            else
               ClientBase::onNetMessage(rtr,msg); 
         }
         if(msg->getMsgType() == Cora::PbRouter::Messages::announce_address)
         {
            msg->readUInt2(pbrouter_address);
            if(pbrouterbase_state == pbrouterbase_state_ready)
               on_pbrouter_address_change(pbrouter_address);
         }
         else if(msg->getMsgType() == Messages::announce_access_level)
         {
            msg->reset();
            msg->readUInt4(pbr_access_level);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage
      
      
      void PbRouterBase::onNetSesBroken(
         Csi::Messaging::Router *rtr,
         uint4 session_no,
         uint4 reason,
         char const *why)
      {
         if(session_no == pbrouter_session)
         {
            pbrouter_session = 0;
            on_pbrouterbase_failure(pbrouterbase_failure_session);
         }
         else
            ClientBase::onNetSesBroken(rtr,session_no,reason,why);
      } // onNetSesBroken

      
      void PbRouterBase::on_corabase_ready()
      {
         // we can now send the message to request attachment to the router
         pbrouterbase_state = pbrouterbase_state_attach;
         pbrouter_session = router->openSession(this);
         if(interface_version <= Csi::VersionNumber("1.3.4.7"))
         {
            Csi::Messaging::Message cmd(
               net_session,Cora::LgrNet::Messages::open_pakbus_router_session_cmd);
            cmd.addUInt4(++last_tran_no);
            pbrouter_address = static_cast<uint2>(
               wcstoul(pakbus_router_name.c_str(),0,10));
            cmd.addUInt2(pbrouter_address);
            cmd.addUInt4(pbrouter_session);
            router->sendMessage(&cmd);
         }
         else
         {
            Csi::Messaging::Message cmd(
               net_session,
               Cora::LgrNet::Messages::open_named_pakbus_router_session_cmd);
            cmd.addUInt4(++last_tran_no);
            cmd.addWStr(pakbus_router_name);
            cmd.addUInt4(pbrouter_session);
            router->sendMessage(&cmd);
         }
      } // on_corabase_ready

      
      void PbRouterBase::on_corabase_failure(corabase_failure_type failure)
      {
         pbrouterbase_failure_type pbrouter_failure;
         switch(failure)
         {
         case corabase_failure_logon:
            pbrouter_failure = pbrouterbase_failure_logon;
            break;
            
         case corabase_failure_session:
            pbrouter_failure = pbrouterbase_failure_session;
            break;
            
         case corabase_failure_unsupported:
            pbrouter_failure = pbrouterbase_failure_unsupported;
            break;
            
         case corabase_failure_security:
            pbrouter_failure = pbrouterbase_failure_security;
            break;
            
         default:
            pbrouter_failure = pbrouterbase_failure_unknown;
            break;
         }
         on_pbrouterbase_failure(pbrouter_failure);
      } // on_corabase_failure

      
      void PbRouterBase::on_corabase_session_failure()
      { on_pbrouterbase_failure(pbrouterbase_failure_session); }

      void PbRouterBase::format_failure(std::ostream &out, pbrouterbase_failure_type failure) 
      {
         using namespace PbRouterBaseStrings;
         switch(failure)
         {
            case pbrouterbase_failure_logon:
               ClientBase::describe_failure(out, corabase_failure_logon);
               break;
            case pbrouterbase_failure_session:
               ClientBase::describe_failure(out, corabase_failure_session);
               break;
            case pbrouterbase_failure_invalid_router_id:
               out << my_strings[strid_invalid_id];
               break;
            case pbrouterbase_failure_unsupported:
               ClientBase::describe_failure(out, corabase_failure_security);
               break;
            case pbrouterbase_failure_security:
               ClientBase::describe_failure(out, corabase_failure_security);
               break;
            case pbrouterbase_failure_unknown:
            default:
               ClientBase::describe_failure(out, corabase_failure_unknown);
               break;
         }
      } // format_failure

   };
};
