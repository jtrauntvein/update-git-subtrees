/* Cora.LgrNet.CommPortsLister.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 15 April 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.CommPortsLister.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef CommPortsLister lister_type;
            typedef lister_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // lister
            ////////////////////////////////////////////////////////////
            lister_type *lister;
            
            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // names
            ////////////////////////////////////////////////////////////
            client_type::names_type names;

            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static event_complete *create(
               lister_type *lister,
               client_type *client,
               outcome_type outcome)
            { return new event_complete(lister,client,outcome); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               lister_type *lister_,
               client_type *client_,
               outcome_type outcome_):
               lister(lister_),
               client(client_),
               outcome(outcome_),
               Event(event_id,lister_)
            { } 
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::LgrNet::CommPortsLister::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class CommPortsLister definitions
      ////////////////////////////////////////////////////////////
      CommPortsLister::CommPortsLister():
         state(state_standby),
         client(0)
      { }

      
      CommPortsLister::~CommPortsLister()
      { finish(); }

      
      void CommPortsLister::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               ClientBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void CommPortsLister::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               ClientBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void CommPortsLister::finish()
      {
         state = state_standby;
         client = 0;
         ClientBase::finish();
      } // finish

      
      void CommPortsLister::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(
            net_session,
            Cora::LgrNet::Messages::list_comm_ports_cmd);
         cmd.addUInt4(++last_tran_no);
         if(get_interface_version() >= Csi::VersionNumber("1.3.11.4"))
         {
            cmd.addBool(true);
            requested_friendly_names = true;
         }
         else
            requested_friendly_names = false;
         state = state_active;
         router->sendMessage(&cmd);
      } // on_corabase_ready

      
      void CommPortsLister::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;

         case corabase_failure_session:
            outcome = client_type::outcome_session_broken;
            break;

         case corabase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;

         case corabase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;

         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         try
         {
            event_complete *event = event_complete::create(this,client,outcome);
            event->post();
         }
         catch(Csi::Event::BadPost &)
         { }
      } // on_corabase_failure

      
      void CommPortsLister::on_corabase_session_failure()
      {
         try
         {
            event_complete *event = event_complete::create(
               this,client,client_type::outcome_session_broken);
            event->post();
         }
         catch(Csi::Event::BadPost &)
         { }
      } // on_corabase_session_failure

      
      void CommPortsLister::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = event->client;
            finish();
            if(client_type::is_valid_instance(client) && client == event->client)
               event->client->on_complete(
                  event->lister,
                  event->outcome,
                  event->names);
         }
      } // receive

      
      void CommPortsLister::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Cora::LgrNet::Messages::list_comm_ports_ack)
            {
               uint4 tran_no;
               uint4 result;
               msg->readUInt4(tran_no);
               msg->readUInt4(result);
               if(result == 1)
               {
                  uint4 num_ports;
                  StrAsc port_name;
                  event_complete *event = event_complete::create(
                     this,client,client_type::outcome_success);
                  
                  msg->readUInt4(num_ports);
                  for(uint4 i = 0; i < num_ports; ++i)
                  {
                     msg->readStr(port_name);
                     event->names.push_back(client_type::name_type(port_name, ""));
                     if(requested_friendly_names)
                        msg->readStr(event->names.back().second);
                  }
                  try
                  { event->post(); }
                  catch(Csi::Event::BadPost &)
                  { }
               }
               else
               {
                  try
                  {
                     event_complete *event = event_complete::create(
                        this,client,client_type::outcome_unknown);
                     event->post();
                  }
                  catch(Csi::Event::BadPost &)
                  { }
               }
            }
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage
   };
};

