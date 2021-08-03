/* Cora.Broker.BrokerBase.cpp

   Copyright (C) 2000, 2007 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 30 May 2000
   Last Change: Thursday 29 November 2007
   Last Commit: $Date: 2007-11-29 15:04:32 -0600 (Thu, 29 Nov 2007) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.BrokerBase.h"
#include "Cora.LgrNet.Defs.h"
#include "Cora.Sec2.Defs.h"
#include "coratools.strings.h"
#include <assert.h>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class BrokerBase definitions
      ////////////////////////////////////////////////////////////
      BrokerBase::BrokerBase():
         broker_session(0),
         open_broker_id(0),
         state(brokerbase_state_standby),
         broker_access_level(Sec2::AccessLevels::level_root)
      { }

      
      BrokerBase::~BrokerBase()
      { finish(); }

      
      void BrokerBase::set_open_broker_active_name(StrUni const &open_broker_active_name_)
      {
         if(state == brokerbase_state_standby)
         {
            open_broker_active_name = open_broker_active_name_;
            if(open_broker_active_name.length() > 0)
               open_broker_id = 0;
         }
         else
            throw exc_invalid_state();
      } // set_open_broker_active_name


      void BrokerBase::set_open_broker_id(
         uint4 open_broker_id_,
         StrUni const &broker_name_)
      {
         if(state == brokerbase_state_standby)
         {
            open_broker_id = open_broker_id_;
            if(open_broker_id != 0)
            {
               if(broker_name_.length() > 0)
                  broker_name = open_broker_active_name = broker_name_;
               else
               {
                  broker_name.cut(0);
                  open_broker_active_name.cut(0);
               }
            }
         }
         else
            throw exc_invalid_state();
      } // set_open_broker_id


      void BrokerBase::start(router_handle &router)
      {
         if(state == brokerbase_state_standby)
         {
            if(open_broker_active_name.length() > 0 || open_broker_id != 0)
            {
               state = brokerbase_state_delegate;
               ClientBase::start(router);
            }
            else
               throw std::invalid_argument("Broker identifier not specified");
         }
         else
            throw exc_invalid_state();
      } // start


      void BrokerBase::start(ClientBase *other_client)
      {
         if(state == brokerbase_state_standby)
         {
            if(open_broker_active_name.length() > 0 || open_broker_id != 0)
            {
               state = brokerbase_state_delegate;
               ClientBase::start(other_client);
            }
            else
               throw std::invalid_argument("Broker identifier not specified");
         }
         else
            throw exc_invalid_state();
      } // start


      void BrokerBase::finish()
      {
         if(router.get_rep() && broker_session)
         {
            router->closeSession(broker_session);
            broker_session = 0;
         }
         state = brokerbase_state_standby;
         ClientBase::finish();
      } // finish


       void BrokerBase::onNetMessage(
          Csi::Messaging::Router *rtr,
          Csi::Messaging::Message *msg)
       {
          switch(msg->getMsgType())
          {
          case LgrNet::Messages::open_data_broker_session_ack:
          case LgrNet::Messages::open_active_data_broker_session_ack:
             on_open_broker_ses_ack(msg);
             break;

          case LgrNet::Messages::data_brokers_enum_not:
             on_enum_brokers_not(msg);
             break;

          case Messages::exception:
             on_broker_exception(msg);
             break;

          case Messages::announce_access_level:
             msg->reset();
             msg->readUInt4(broker_access_level);
             break;

          default:
             ClientBase::onNetMessage(rtr,msg);
             break;
          }
       } // onNetMessage


       void BrokerBase::onNetSesBroken(
          Csi::Messaging::Router *rtr,
          uint4 session_no,
          uint4 reason,
          char const *msg)
       {
          if(session_no == broker_session)
          {
             broker_session = 0;
             if(state == brokerbase_state_ready)
                on_brokerbase_session_failure();
             else
                on_brokerbase_failure(brokerbase_failure_session);
          }
          else
             ClientBase::onNetSesBroken(rtr,session_no,reason,msg);
       } // onNetSesBroken


      void BrokerBase::format_failure(
         std::ostream &out, brokerbase_failure_type failure)
      {
         using namespace BrokerBaseStrings;
         switch(failure)
         {
         case brokerbase_failure_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case brokerbase_failure_session:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case brokerbase_failure_invalid_id:
            out << my_strings[strid_invalid_id];
            break;
            
         case brokerbase_failure_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;
            
         case brokerbase_failure_security:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;

         default:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // format_failure

      
      void BrokerBase::on_corabase_ready()
      {
         state = brokerbase_state_attach;
         broker_session = router->openSession(this);
         if(open_broker_active_name.length() > 0)
         {
            Csi::Messaging::Message open_cmd(
               ClientBase::net_session,
               LgrNet_OpenActiveDataBrokerSesCmd);
            open_cmd.addUInt4(++last_tran_no);
            open_cmd.addWStr(open_broker_active_name);
            open_cmd.addUInt4(broker_session);
            router->sendMessage(&open_cmd);
         }
         else
         {
            Csi::Messaging::Message open_cmd(
               ClientBase::net_session,
               LgrNet_OpenDataBrokerSesCmd);
            assert(open_broker_id != 0);
            open_cmd.addUInt4(++last_tran_no);
            open_cmd.addUInt4(open_broker_id);
            open_cmd.addUInt4(broker_session);
            open_cmd.addBool(true); // send the broker name (if supported)
            router->sendMessage(&open_cmd); 
         }
      } // on_corabase_ready


      void BrokerBase::on_corabase_failure(corabase_failure_type failure)
      {
         brokerbase_failure_type broker_failure;
         switch(failure)
         {
         case corabase_failure_logon:
            broker_failure = brokerbase_failure_logon;
            break;
            
         case corabase_failure_session:
            broker_failure = brokerbase_failure_session;
            break;
            
         case corabase_failure_unsupported:
            broker_failure = brokerbase_failure_unsupported;
            break;
            
         case corabase_failure_security:
            broker_failure = brokerbase_failure_security;
            break;
            
         default:
            broker_failure = brokerbase_failure_unknown;
            break;
         }
         on_brokerbase_failure(broker_failure);
      } // on_corabase_failure


      void BrokerBase::on_corabase_session_failure()
      {
         // if we are in a ready state, we should be able to safely ignore the loss of the network
         // session since we already have a connection to the desired broker. A derived class can
         // overload this method if it is sensitive to the loss of the network session.
         if(state == brokerbase_state_delegate || state == brokerbase_state_attach)
            on_brokerbase_failure(brokerbase_failure_session);
      } // on_corabase_session_failure


      void BrokerBase::on_open_broker_ses_ack(Csi::Messaging::Message *msg)
      {
         uint4 tran_no;
         uint4 resp_code;

         if(state == brokerbase_state_attach)
         {
            msg->readUInt4(tran_no);
            msg->readUInt4(resp_code);
            if(resp_code == 1)
            {
               // if we opened using the active broker name, we are now finished
               if(msg->getMsgType() == LgrNet::Messages::open_active_data_broker_session_ack ||
                  broker_name.length() > 0)
               {
                  broker_name = open_broker_active_name;
                  state = brokerbase_state_ready;
                  on_brokerbase_ready();
               }
               else if(msg->getMsgType() == LgrNet::Messages::open_data_broker_session_ack &&
                       msg->whatsLeft() > 4)
               {
                  msg->readWStr(broker_name);
                  state = brokerbase_state_ready;
                  on_brokerbase_ready();
               }
               else
               {
                  Csi::Messaging::Message enum_brokers_cmd(
                     net_session,
                     LgrNet::Messages::data_brokers_enum_cmd);
                  enum_brokers_cmd.addUInt4(++last_tran_no);
                  router->sendMessage(&enum_brokers_cmd);
               }
            }
            else
               on_brokerbase_failure(brokerbase_failure_invalid_id);
         }
      } // on_open_broker_ses_ack


      void BrokerBase::on_broker_exception(Csi::Messaging::Message *message)
      {
         uint4 what_happened;
         brokerbase_failure_type failure;

         message->readUInt4(what_happened);
         switch(what_happened)
         {
         case 2:
         case 4:
            failure = brokerbase_failure_unsupported;
            break;

         case 3:
            failure = brokerbase_failure_security;
            break;

         default:
            failure = brokerbase_failure_unknown;
            break;
         }
         on_brokerbase_failure(failure);
      } // on_broker_exception


      void BrokerBase::on_enum_brokers_not(Csi::Messaging::Message *message)
      {
         if(state == brokerbase_state_attach)
         {
            uint4 tran_no;
            uint4 resp_code;
            uint4 num_brokers;
            bool found_broker = false;
            uint4 broker_id;
            uint4 broker_type;
            StrUni broker_name;
            uint4 op_code;
            
            message->readUInt4(tran_no);
            message->readUInt4(resp_code);
            message->readUInt4(num_brokers);
            for(uint4 i = 0; !found_broker && i < num_brokers; ++i)
            {
               message->readUInt4(op_code);
               message->readUInt4(broker_id);
               message->readUInt4(broker_type);
               message->readWStr(broker_name);
               if(op_code == 1 && broker_id == open_broker_id)
               {
                  found_broker = true;
                  this->broker_name = broker_name;
                  break;
               }
            }

            // if we found the broker, we are now finished, otherwise, there is a problem
            if(found_broker)
            {
               state = brokerbase_state_ready;
               on_brokerbase_ready();
            }
            else
               on_brokerbase_failure(brokerbase_failure_invalid_id);
         }
      } // on_enum_brokers_not
   };
};
         
