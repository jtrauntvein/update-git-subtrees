/* Cora.LgrNet.NetworkMapper.cpp

   Copyright (C) 2000, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 11 November 2000
   Last Change: Tuesday 22 January 2019
   Last Commit: $Date: 2019-01-22 15:04:50 -0600 (Tue, 22 Jan 2019) $ 
   Committed by: $Author: jon $
   
*/


#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.NetworkMapper.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace NetworkMapperHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            NetworkMapper *mapper;
            NetworkMapperClient *client;
            
         protected:
            event_base(
               NetworkMapper *mapper_,
               NetworkMapperClient *client_,
               uint4 event_id):
               Event(event_id,mapper_),
               mapper(mapper_),
               client(client_)
            { }

         public:
            ////////// notify
            virtual void notify()  = 0;
         };


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            static uint4 const event_id;
            typedef NetworkMapperClient::failure_type failure_type;
            failure_type failure;
            
         private:
            event_failure(
               NetworkMapper *mapper,
               NetworkMapperClient *client,
               failure_type failure_):
               event_base(mapper,client,event_id),
               failure(failure_)
            { }

         public:
            static void create_and_post(
               NetworkMapper *mapper,
               NetworkMapperClient *client,
               failure_type failure)
            {
               try {(new event_failure(mapper,client,failure))->post(); }
               catch(Event::BadPost &) { }
            }

            virtual void notify()
            { client->on_failure(mapper,failure); } 
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::LgrNet::NetworkMapper::event_failure");


         ////////////////////////////////////////////////////////////
         // class event_notify
         ////////////////////////////////////////////////////////////
         class event_notify: public event_base
         {
         public:
            static uint4 const event_id;
            uint4 network_map_version;
            uint4 agent_transaction_id;
            bool first_notification;
            uint4 device_count;
            
         private:
            event_notify(
               NetworkMapper *mapper,
               NetworkMapperClient *client,
               uint4 network_map_version_,
               uint4 agent_transaction_id_,
               bool first_notification_,
               uint4 device_count_):
               event_base(mapper,client,event_id),
               network_map_version(network_map_version_),
               agent_transaction_id(agent_transaction_id_),
               first_notification(first_notification_),
               device_count(device_count_)
            { }

         public:
            static void create_and_post(
               NetworkMapper *mapper,
               NetworkMapperClient *client,
               uint4 network_map_version,
               uint4 agent_transaction_id,
               bool first_notification,
               uint4 device_count)
            {
               try
               {
                  (new event_notify(
                     mapper,
                     client,
                     network_map_version,
                     agent_transaction_id,
                     first_notification,
                     device_count))->post();
               }
               catch(Event::BadPost &) { }
            }

            virtual void notify()
            {
               client->on_notify(
                  mapper,
                  network_map_version,
                  agent_transaction_id,
                  first_notification,
                  device_count);
            } 
         };


         uint4 const event_notify::event_id =
         Csi::Event::registerType("Cora::LgrNet::NetworkMapper::event_notify");


         ////////////////////////////////////////////////////////////
         // class event_device
         ////////////////////////////////////////////////////////////
         class event_device: public event_base
         {
         public:
            static uint4 const event_id;
            DevTypeCode device_type;
            uint4 device_object_id;
            StrUni name;
            uint4 level;
            bool last_device;
            
         private:
            event_device(
               NetworkMapper *mapper,
               NetworkMapperClient *client,
               DevTypeCode device_type_,
               uint4 device_object_id_,
               StrUni const &name_,
               uint4 level_,
               bool last_device_):
               event_base(mapper,client,event_id),
               device_type(device_type_),
               device_object_id(device_object_id_),
               name(name_),
               level(level_),
               last_device(last_device_)
            { }

         public:
            static void create_and_post(
               NetworkMapper *mapper,
               NetworkMapperClient *client,
               DevTypeCode device_type,
               uint4 device_object_id,
               StrUni const &name,
               uint4 level,
               bool last_device)
            {
               try
               {
                  (new event_device(
                     mapper,
                     client,
                     device_type,
                     device_object_id,
                     name,
                     level,
                     last_device))->post();
               }
               catch(Event::BadPost &) { }
            }

            virtual void notify()
            {
               client->on_device(
                  mapper,
                  device_type,
                  device_object_id,
                  name,
                  level,
                  last_device);
            } 
         };


         uint4 const event_device::event_id =
            Csi::Event::registerType("Cora::LgrNet::NetworkMapper::event_device");
         

         /**
          * Defines the event that will report when a snapshot has been restored.
          */
         class event_snapshot_restored: public event_base
         {
         public:
            static uint4 const event_id;

            static void cpost(NetworkMapper *mapper, NetworkMapperClient *client)
            { (new event_snapshot_restored(mapper, client))->post(); }

            virtual void notify()
            { client->on_snapshot_restored(mapper); }
            
         private:
            event_snapshot_restored(NetworkMapper *mapper, NetworkMapperClient *client):
               event_base(mapper, client, event_id)
            { }
         };

         uint4 const event_snapshot_restored::event_id(
            Csi::Event::registerType("Cora::LgrNet::NetworkMapper::event_snapshot_restored"));
      };


      ////////////////////////////////////////////////////////////
      // class NetworkMapper definitions
      ////////////////////////////////////////////////////////////
      NetworkMapper::NetworkMapper():
         state(state_standby),
         client(0)
      { }

      
      NetworkMapper::~NetworkMapper()
      { finish(); }


      void NetworkMapper::start(
         NetworkMapperClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(NetworkMapperClient::is_valid_instance(client_))
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


      void NetworkMapper::start(
         NetworkMapperClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(NetworkMapperClient::is_valid_instance(client_))
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

      
      void NetworkMapper::finish()
      {
         state = state_standby;
         client = 0;
         ClientBase::finish();
      } // finish


      void NetworkMapper::format_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_unknown:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
            
         case client_type::failure_invalid_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::failure_session_broken:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::failure_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::failure_server_security:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;
         }
      } // format_failure

      
      void NetworkMapper::on_corabase_ready()
      {
         Csi::Messaging::Message command(
            net_session,
            Messages::network_map_enum_cmd);
         router->sendMessage(&command);
         state = state_before_active; 
      } // on_corabase_ready

      
      void NetworkMapper::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace NetworkMapperHelpers;
         client_type::failure_type client_failure;
         switch(failure)
         {
         case corabase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case corabase_failure_session:
            client_failure = client_type::failure_session_broken;
            break;
            
         case corabase_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case corabase_failure_security:
            client_failure = client_type::failure_server_security;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_corabase_failure

      
      void NetworkMapper::on_corabase_session_failure()
      {
         using namespace NetworkMapperHelpers;
         event_failure::create_and_post(this,client,client_type::failure_session_broken);
      } // on_corabase_session_failure


      void NetworkMapper::on_snapshot_restored()
      {
         NetworkMapperHelpers::event_snapshot_restored::cpost(this, client);
      } // on_snapshot_restored

      
      void NetworkMapper::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace NetworkMapperHelpers;
         event_base *event = static_cast<event_base *>(ev.get_rep());
         if(event->client == client)
         {
            if(event->getType() == event_failure::event_id)
               finish();
            if(client_type::is_valid_instance(event->client))
               event->notify();
            else
               finish();
         }
      } // receive


      void NetworkMapper::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active || state == state_before_active)
         {
            if(msg->getMsgType() == Messages::network_map_advise_not)
            {
               using namespace NetworkMapperHelpers;
               uint4 version;
               uint4 agent_tran;
               uint4 count;
               uint4 type;
               uint4 object_id;
               StrUni name;
               uint4 level;
               
               msg->readUInt4(version);
               msg->readUInt4(agent_tran);
               msg->readUInt4(count);
               event_notify::create_and_post(
                  this,
                  client,
                  version,agent_tran,
                  state == state_before_active,
                  count);
               if(state == state_before_active)
                  state = state_active;
               for(uint4 i = 0; i < count; ++i)
               {
                  msg->readUInt4(type);
                  msg->readUInt4(object_id);
                  msg->readWStr(name);
                  msg->readUInt4(level);
                  event_device::create_and_post(
                     this,
                     client,
                     static_cast<DevTypeCode>(type),
                     object_id,
                     name,
                     level,
                     i == count - 1); 
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
