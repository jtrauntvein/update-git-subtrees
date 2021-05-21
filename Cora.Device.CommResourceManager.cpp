/* Cora.Device.CommResourceManager.cpp

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 05 July 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CommResourceManager.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace CommResourceManagerHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            typedef CommResourceManager manager_type;
            typedef CommResourceManagerClient client_type;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify() = 0;

            ////////////////////////////////////////////////////////////
            // get_client
            ////////////////////////////////////////////////////////////
            client_type *get_client() { return client; }
            
         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               manager_type *manager_,
               client_type *client_):
               Event(event_id,manager_),
               manager(manager_),
               client(client_)
            { }

         protected:
            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            client_type *client;

            ////////////////////////////////////////////////////////////
            // manager
            ////////////////////////////////////////////////////////////
            manager_type *manager;
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;
            
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               manager_type *manager,
               client_type *client)
            {
               try { (new event_started(manager,client))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            virtual void notify()
            { client->on_started(manager); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               manager_type *manager,
               client_type *client):
               event_base(event_id,manager,client)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Device::CommResourceManager::event_started");


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;
            
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            typedef client_type::failure_type failure_type; 
            static void create_and_post(
               manager_type *manager,
               client_type *client,
               failure_type failure)
            {
               try { (new event_failure(manager,client,failure))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            virtual void notify()
            { client->on_failure(manager,failure); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               manager_type *manager,
               client_type *client,
               failure_type failure_):
               event_base(event_id,manager,client),
               failure(failure_)
            { }

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            failure_type failure;
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Device::CommResourceManager::event_failure");
      };

      
      ////////////////////////////////////////////////////////////
      // class CommResoureManager definitions
      //////////////////////////////////////////////////////////// 
      CommResourceManager::CommResourceManager():
         client(0),
         state(state_standby),
         priority(priority_high)
      { }
      

      CommResourceManager::~CommResourceManager()
      { finish(); }

      
      void CommResourceManager::set_priority(priority_type priority_)
      {
         if(state == state_standby)
            priority = priority_;
         else
            throw exc_invalid_state();
      } // set_priority

      
      void CommResourceManager::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
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


      void CommResourceManager::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
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

      
      void CommResourceManager::finish()
      {
         connection_manager.clear();
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish


      void CommResourceManager::describe_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         using namespace CommResourceManagerStrings;
         switch(failure)
         {
         case client_type::failure_server_session_lost:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::failure_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::failure_server_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::failure_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::failure_device_shut_down_by_server:
            out << my_strings[strid_device_shut_down];
            break;
            
         case client_type::failure_device_communication_disabled:
            out << my_strings[strid_communication_disabled];
            break;
            
         case client_type::failure_unsupported:
            out << my_strings[strid_unsupported];
            break;
            
         case client_type::failure_unreachable:
            out << my_strings[strid_unreachable];
            break;
            
         case client_type::failure_max_time_online:
            out << my_strings[strid_max_time_online];
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // describe_failure

      
      void CommResourceManager::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace CommResourceManagerHelpers;
         event_base *event = static_cast<event_base *>(ev.get_rep());
         bool should_notify = true;

         if(event->get_client() != client || !client_type::is_valid_instance(client))
            should_notify = false;
         if(event->getType() == event_failure::event_id)
            finish();
         if(should_notify)
            event->notify();
      } // receive

      
      void CommResourceManager::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::manage_comm_resource_start_ack:
               on_start_ack(msg);
               break;
               
            case Messages::manage_comm_resource_stopped_not:
               on_stopped_not(msg);
               break;

            default:
               DeviceBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void CommResourceManager::on_devicebase_ready()
      {
         if(interface_version >= Csi::VersionNumber("1.3.1"))
         {
            Csi::Messaging::Message command(
               device_session,
               Messages::manage_comm_resource_start_cmd);
            command.addUInt4(++last_tran_no);
            command.addUInt4(priority);
            state = state_active;
            router->sendMessage(&command);
         }
         else
         {
            connection_manager.bind(new ConnectionManager);
            connection_manager->set_device_name(get_device_name());
            connection_manager->set_priority(
               static_cast<ConnectionManager::priority_type>(priority));
            connection_manager->set_keep_open(true);
            connection_manager->set_force_open(false);
            connection_manager->start(this,this);
         }
      } // on_devicebase_ready

      
      void CommResourceManager::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace CommResourceManagerHelpers;
         client_type::failure_type client_failure;
         switch(failure)
         {
         case devicebase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case devicebase_failure_session:
            client_failure = client_type::failure_server_session_lost;
            break;
            
         case devicebase_failure_invalid_device_name:
            client_failure = client_type::failure_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case devicebase_failure_security:
            client_failure = client_type::failure_server_security_blocked;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_devicebase_failure

      
      void CommResourceManager::on_devicebase_session_failure()
      {
         using namespace CommResourceManagerHelpers;
         event_failure::create_and_post(this,client,client_type::failure_server_session_lost);
      } // on_devicebase_session_failure


      void CommResourceManager::on_started(
         ConnectionManager *manager)
      {
         using namespace CommResourceManagerHelpers;
         event_started::create_and_post(this,client);
      } // on_started


      void CommResourceManager::on_failure(
         ConnectionManager *manager,
         failure_type failure,
         StrUni const &next_candidate)
      {
         using namespace CommResourceManagerHelpers;
         client_type::failure_type client_failure;
         switch(failure)
         {
         case failure_connection_failed:
            client_failure = client_type::failure_server_session_lost;
            break;
            
         case failure_invalid_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
                                                                   
         case failure_server_security_blocked:
            client_failure = client_type::failure_server_security_blocked;
            break;
            
         case failure_device_name_invalid:
            client_failure = client_type::failure_invalid_device_name;
            break;
            
         case failure_server_terminated_transaction:
            client_failure = client_type::failure_device_shut_down_by_server;
            break;
            
         case failure_device_does_not_support:
            client_failure = client_type::failure_unsupported;
            break;
            
         case failure_path_does_not_support:
            client_failure = client_type::failure_unreachable;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_failure

      
      void CommResourceManager::on_start_ack(Csi::Messaging::Message *message)
      {
         using namespace CommResourceManagerHelpers;
         event_started::create_and_post(this,client);
      } // on_start_ack

      
      void CommResourceManager::on_stopped_not(Csi::Messaging::Message *message)
      {
         // read the message
         uint4 tran_no;
         uint4 reason;
         message->readUInt4(tran_no);
         message->readUInt4(reason);

         // interpret the response
         using namespace CommResourceManagerHelpers;
         client_type::failure_type failure;

         switch(reason)
         {
         case 3:
            failure = client_type::failure_device_shut_down_by_server;
            break;
            
         case 4:
            failure = client_type::failure_device_communication_disabled;
            break;

         default:
            failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(this,client,failure);
      } // on_stopped_not
   };
}; 
      
