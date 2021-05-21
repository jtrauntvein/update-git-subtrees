/* Cora.Device.ConnectionManager.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Carl Zmola, revised by Jon Trauntvein
   Date Begun: 30 June 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.ConnectionManager.h"
#include "Cora.Defs.h"
#include <assert.h>


namespace Cora
{
   namespace Device
   {
      namespace ConnectionManagerHelpers
      {

         ////////////////////////////////////////////////////////////
         // class event_on_failure declaration and definitions
         ////////////////////////////////////////////////////////////

         class event_on_failure:public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef ConnectionManagerClient client_type;
            typedef client_type::failure_type failure_type;
            failure_type failure;
            client_type *client;
            StrUni next_candidate;

            static void create_and_post(
               ConnectionManager *receiver,
               client_type *client_,
               failure_type failure_,
               wchar_t const *next_candidate = L"");

         private:
            event_on_failure(
               ConnectionManager *receiver,
               client_type *client_,
               failure_type failure_,
               wchar_t const *next_candidate_):
               Event(event_id,receiver),
               client(client_),
               failure(failure_),
               next_candidate(next_candidate_)
            { }
         };


         uint4 const event_on_failure::event_id = 
         Csi::Event::registerType("Cora::Device::ConnectionMangagerHelpers::event_on_failure");


         void event_on_failure::create_and_post(
            ConnectionManager *receiver,
            client_type *client_,
            failure_type failure_,
            wchar_t const *next_candidate)
         {
            try
            {
               event_on_failure *ev = new event_on_failure(
                  receiver,client_,failure_,next_candidate);
               ev->post();
            }
            catch(Csi::Event::BadPost &)
            {} 
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_on_started declarations and definitions
         //////////////////////////////////////////////////////////// 
         class event_on_started:public Csi::Event
         {
         public:
            static uint4 const event_id;
            ConnectionManagerClient *client;

            static void create_and_post(ConnectionManager *receiver,
                                        ConnectionManagerClient *client_);

         private:
            event_on_started(ConnectionManager *receiver,
                             ConnectionManagerClient *client_):
               Event(event_id,receiver),
               client(client_)
            { }
         };


         uint4 const event_on_started::event_id = 
         Csi::Event::registerType("Cora::Device::ConnectionMangagerHelpers::event_on_started");


         void event_on_started::create_and_post(ConnectionManager *receiver,
                                                ConnectionManagerClient *client_)
         {
            try
            {
               event_on_started *ev = new event_on_started(receiver, client_);
               ev->post();
            }
            catch(Csi::Event::BadPost &)
            {} 
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_on_state_change declarations and definitions
         ////////////////////////////////////////////////////////////
         
         class event_on_state_change:public Csi::Event
         {
         public:
            static uint4 const event_id;
            bool online;
            ConnectionManagerClient *client;

            static void create_and_post(ConnectionManager *receiver,
                                        ConnectionManagerClient *client_,
                                        bool online_);

         private:
            event_on_state_change(ConnectionManager *receiver,
                                  ConnectionManagerClient *client_,
                                  bool online_):
               Event(event_id,receiver),
               client(client_),
               online(online_)
            { }
         };


         uint4 const event_on_state_change::event_id = 
         Csi::Event::registerType("Cora::Device::ConnectionMangagerHelpers::event_on_state_change");


         void event_on_state_change::create_and_post(ConnectionManager *receiver,
                                                     ConnectionManagerClient *client_,
                                                     bool online_)
         {
            try
            {
               event_on_state_change *ev = new event_on_state_change(receiver, client_, online_);
               ev->post();
            }
            catch(Csi::Event::BadPost &)
            {} 
         } // create_and_post
      }; // namespace ConnectionManagerHelpers


      ////////////////////////////////////////////////////////////
      // class ConnectionManager definitions
      ////////////////////////////////////////////////////////////
      
      ConnectionManager::ConnectionManager():
         priority(priority_high),
         force_open(true),
         keep_open(true),
         state(state_standby)
      { }


      ConnectionManager::~ConnectionManager()
      { finish(); }


      void ConnectionManager::set_priority(priority_type priority_)
      {
         if(state == state_standby)
            priority = priority_;
         else
            throw exc_invalid_state();
      } // set_priority


      void ConnectionManager::set_keep_open(bool keep_open_)
      {
         if (state == state_standby)
            keep_open = keep_open_;
         else
            throw exc_invalid_state();
      } // set_keep_open


      void ConnectionManager::set_force_open(bool force_open_)
      {
         if (state == state_standby)
            force_open=force_open_;
         else
            throw exc_invalid_state();
      } // set_force_open


      void ConnectionManager::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace ConnectionManagerHelpers;
         if (ev->getType() == event_on_failure::event_id)
         {
            event_on_failure *pev = static_cast<event_on_failure *>(ev.get_rep());
            if(pev->client == client)
            {
               finish();
               if(ConnectionManagerClient::is_valid_instance(pev->client)) 
                  pev->client->on_failure(this,pev->failure,pev->next_candidate);
            }
         }
         else if (ev->getType() == event_on_started::event_id)
         {
            event_on_started *pev = static_cast<event_on_started *>(ev.get_rep());
            if(pev->client == client &&
               ConnectionManagerClient::is_valid_instance(client))
               client->on_started(this);
         }
         else if (ev->getType() == event_on_state_change::event_id)
         {
            event_on_state_change *pev = static_cast<event_on_state_change *>(ev.get_rep());
            if(client == pev->client &&
               ConnectionManagerClient::is_valid_instance(client))
               client->on_state_change(this,pev->online);
         }
      } // receive


      void ConnectionManager::onNetMessage(Csi::Messaging::Router *router, 
                                           Csi::Messaging::Message *message)
      {
         if(state == state_local)
         {
            // process only messages that I am expecting back from the server.
            // pass all other messages to the inherited message handler so
            // they can be processed correctly.
            // these will be connection management messages.
            switch (message->getMsgType())
            {
               
            case Cora::Device::Messages::connection_management_start_ack:
               on_start_ack(message);
               break;
      
            case Cora::Device::Messages::connection_management_status_not:
               on_status_notification(message);
               break;

            default:
               DeviceBase::onNetMessage(router,message);
            }
         }
         else
            DeviceBase::onNetMessage(router,message);
      }


      void ConnectionManager::on_start_ack(Csi::Messaging::Message *message)
      {
         using namespace ConnectionManagerHelpers;
         uint4 tran_no;
         uint4 resp_code;
         StrUni next_candidate;
         
         message->readUInt4(tran_no);
         message->readUInt4(resp_code);

         //###############################################
         // do not call on_xxx notifications directly, 
         // use events that call these notifications.
         // Need to unwind the stack so a client can delete
         // things durring notifictions and engage in other 
         // unsportsmanlike acts.
         //
         if(resp_code == 1)
            event_on_started::create_and_post(this,client);
         else
         {
            ConnectionManagerClient::failure_type failure;
            switch (resp_code) 
            {
            case 2:
               failure = ConnectionManagerClient::failure_unexpected;
               break;
               
            case 3:
               failure = ConnectionManagerClient::failure_device_does_not_support;
               break;
               
            case 4:
               failure = ConnectionManagerClient::failure_path_does_not_support;
               message->readWStr(next_candidate);
               break;
                
            default:
               failure = ConnectionManagerClient::failure_unknown;
               break;
            };
            event_on_failure::create_and_post(this,client,failure,next_candidate.c_str());
         }
      } // on_start_ack


      void ConnectionManager::on_status_notification(Csi::Messaging::Message *message)
      {
         // read the message
         using namespace ConnectionManagerHelpers;
         uint4 tran_no;
         bool server_terminated;
         bool on_line;
         
         message->readUInt4(tran_no);
         message->readBool(server_terminated);
         message->readBool(on_line);
         event_on_state_change::create_and_post(this,client,on_line);

         //###############################################
         // do not call on_xxx notifications directly, 
         // use events that call these notifications.
         // Need to unwind the stack so a client can delete
         // things durring notifictions and engage in other 
         // unsportsmanlike acts.
         if(server_terminated)
         {
            event_on_failure::create_and_post(
               this, 
               client, 
               ConnectionManagerClient::failure_server_terminated_transaction);
         }
      }  // on_status_notification


      void ConnectionManager::on_devicebase_failure(devicebase_failure_type failure)
      {
         // map the device failure into a client failure
         using namespace ConnectionManagerHelpers;
         ConnectionManagerClient::failure_type client_failure;

         switch(failure)
         {
         case devicebase_failure_logon:
            client_failure = ConnectionManagerClient::failure_invalid_logon;
            break;
            
         case devicebase_failure_session:
            client_failure = ConnectionManagerClient::failure_connection_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            client_failure = ConnectionManagerClient::failure_device_name_invalid;
            break;

         default:
            client_failure = ConnectionManagerClient::failure_unknown;
         }
         event_on_failure::create_and_post(this,client,client_failure);
      } // on_devicebase_failure


      void ConnectionManager::on_devicebase_session_failure()
      {
         using namespace ConnectionManagerHelpers;
         event_on_failure::create_and_post(this,
                                           client,
                                           ConnectionManagerClient::failure_connection_failed);
      } // on_devicebase_session_failure


      void ConnectionManager::on_devicebase_ready()
      {
         Csi::Messaging::Message message(device_session,
                                         Cora::Device::Messages::connection_management_start_cmd);
         message.addUInt4(++last_tran_no);
         message.addUInt4(priority);
         message.addBool(keep_open);
         message.addBool(force_open);
         router->sendMessage(&message);
         state = state_local;
      } // on_devicebase_ready()


      void  ConnectionManager::start(
         ConnectionManagerClient *client_, 
         router_handle &router)
      {
         if(state == state_standby)
         {
            if (ConnectionManagerClient::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void  ConnectionManager::start(
         ConnectionManagerClient *client_, 
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if (ConnectionManagerClient::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void ConnectionManager::finish()
      {
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish 
   }
}

