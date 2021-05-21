/* Cora.Device.CollectAreasEnumerator.cpp

  Copyright (c) 2001, 2019 Campbell Scientific, Inc.

  Written by: Jon Trauntvein
  Date Begun: Tuesday 06 November 2001
  Last Change: Friday 30 August 2019
  Last Commit: $Date: 2019-10-04 11:26:12 -0600 (Fri, 04 Oct 2019) $ 

*/

#pragma hdrstop
#include "Cora.Device.CollectAreasEnumerator.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectAreasEnumeratorHelpers
      {
         class event_base: public Csi::Event
         {
         public:
            typedef CollectAreasEnumeratorClient client_type;
            client_type *client;
            typedef CollectAreasEnumerator lister_type;
            lister_type *lister;

            virtual void notify() = 0;
            
         protected:
            event_base(
               uint4 event_id,
               client_type *client_,
               lister_type *lister_):
               Event(event_id,lister_),
               client(client_),
               lister(lister_)
            { }
         };


         class event_failure: public event_base
         {
         public:
            static uint4 const event_id;
            typedef client_type::failure_type failure_type;
            failure_type failure;

            virtual void notify()
            { client->on_failure(lister,failure); }

         private:
            event_failure(
               client_type *client,
               lister_type *lister,
               failure_type failure_):
               event_base(event_id,client,lister),
               failure(failure_)
            { }

         public:
            static void create_and_post(
               client_type *client,
               lister_type *lister,
               failure_type failure)
            {
               try { (new event_failure(client,lister,failure))->post(); }
               catch(Csi::Event::BadPost &) { }
            }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Device::CollectAreasEnumerator::event_failure");


         class event_started: public event_base
         {
         public:
            static uint4 const event_id;

            virtual void notify()
            { client->on_started(lister); }

         private:
            event_started(
               client_type *client,
               lister_type *lister):
               event_base(event_id,client,lister)
            { }


         public:
            static void create_and_post(
               client_type *client,
               lister_type *lister)
            {
               try { (new event_started(client,lister))->post(); }
               catch(Csi::Event::BadPost &) { }
            }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Device::CollectAreasEnumerator::event_started");

         
         class event_area_added: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni area_name;
            typedef client_type::persistence_type persistence_type;
            persistence_type persistence;
            typedef client_type::type_id_type type_id_type;
            type_id_type type_id;

            virtual void notify()
            { client->on_area_added(lister,area_name,persistence,type_id); }

         private:
            event_area_added(
               client_type *client,
               lister_type *lister,
               StrUni const &area_name_,
               persistence_type persistence_,
               type_id_type type_id_):
               event_base(event_id,client,lister),
               area_name(area_name_),
               persistence(persistence_),
               type_id(type_id_)
            { }

         public:
            static void create_and_post(
               client_type *client,
               lister_type *lister,
               StrUni const &area_name,
               persistence_type persistence,
               type_id_type type_id)
            {
               try
               {
                  (new event_area_added(
                     client,
                     lister,
                     area_name,
                     persistence,
                     type_id))->post(); }
               catch(Csi::Event::BadPost &)
               { }
            }
         };


         uint4 const event_area_added::event_id =
         Csi::Event::registerType("Cora::Device::CollectAreasEnumerator::event_area_added");


         class event_area_deleted: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni area_name;

            virtual void notify()
            { client->on_area_deleted(lister,area_name); }

         private:
            event_area_deleted(
               client_type *client,
               lister_type *lister,
               StrUni const &area_name_):
               event_base(event_id,client,lister),
               area_name(area_name_)
            { }

         public:
            static void create_and_post(
               client_type *client,
               lister_type *lister,
               StrUni const &area_name)
            {
               try { (new event_area_deleted(client,lister,area_name))->post(); }
               catch(Csi::Event::BadPost &) { }
            }
         };


         uint4 const event_area_deleted::event_id =
         Csi::Event::registerType("Cora::Device::CollectAreasEnumerator::event_area_deleted");
      };


      CollectAreasEnumerator::CollectAreasEnumerator():
         state(state_standby),
         client(0)
      { }

      
      CollectAreasEnumerator::~CollectAreasEnumerator()
      { finish(); }

      
      void CollectAreasEnumerator::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void CollectAreasEnumerator::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void CollectAreasEnumerator::finish()
      {
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish


      void CollectAreasEnumerator::format_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_connection:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::failure_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::failure_server_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::failure_device_name_invalid:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::failure_not_supported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_failure

      
      void CollectAreasEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace CollectAreasEnumeratorHelpers;
         event_base *event = static_cast<event_base *>(ev.get_rep());
         
         if(ev->getType() == event_failure::event_id)
            finish();
         if(client_type::is_valid_instance(event->client))
            event->notify();
      } // receive

      
      void CollectAreasEnumerator::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_before_active || state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::collect_areas_enum_added_not:
               on_area_added_not(msg);
               break;
               
            case Messages::collect_areas_enum_deleted_not:
               on_area_deleted_not(msg);
               break;
               
            case Messages::collect_areas_enum_stopped_not:
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

      
      void CollectAreasEnumerator::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace CollectAreasEnumeratorHelpers;
         
         client_type::failure_type client_failure;
         switch(failure)
         {
         case devicebase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case devicebase_failure_session:
            client_failure = client_type::failure_connection;
            break;
            
         case devicebase_failure_invalid_device_name:
            client_failure = client_type::failure_device_name_invalid;
            break;
            
         case devicebase_failure_unsupported:
            client_failure = client_type::failure_not_supported;
            break;
            
         case devicebase_failure_security:
            client_failure = client_type::failure_server_security_blocked;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(client,this,client_failure);
      } // on_devicebase_failure

      
      void CollectAreasEnumerator::on_devicebase_session_failure()
      {
         using namespace CollectAreasEnumeratorHelpers;
         event_failure::create_and_post(client,this,client_type::failure_connection);
      } // on_devicebase_session_failure

      
      void CollectAreasEnumerator::on_devicebase_ready()
      {
         Csi::Messaging::Message start_cmd(
            device_session,Messages::collect_areas_enum_start_cmd);
         start_cmd.addUInt4(++last_tran_no);
         if(interface_version >= Csi::VersionNumber("1.3.9.1"))
         {
            start_cmd.addBool(true);
            wants_area_types = true;
         }
         else
            wants_area_types = false;
         router->sendMessage(&start_cmd);
         state = state_before_active; 
      } // on_devicebase_ready

      
      void CollectAreasEnumerator::on_area_added_not(Csi::Messaging::Message *message)
      {
         if(state == state_before_active || state == state_active)
         {
            using namespace CollectAreasEnumeratorHelpers;
            uint4 tran_no;
            uint4 count;
            StrUni area_name;
            uint4 persistence;
            uint4 type_id = 0;
            
            message->readUInt4(tran_no);
            message->readUInt4(count);
            for(uint4 i = 0; i < count; ++i)
            {
               message->readWStr(area_name);
               message->readUInt4(persistence);
               if(wants_area_types)
                  message->readUInt4(type_id);
               event_area_added::create_and_post(
                  client,
                  this,
                  area_name,
                  static_cast<client_type::persistence_type>(persistence),
                  static_cast<client_type::type_id_type>(type_id));
            }
            if(state == state_before_active)
            {
               state = state_active;
               event_started::create_and_post(client,this);
            }
         }
      } // on_area_added_not

      
      void CollectAreasEnumerator::on_area_deleted_not(Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            using namespace CollectAreasEnumeratorHelpers;
            uint4 tran_no;
            StrUni area_name;

            message->readUInt4(tran_no);
            message->readWStr(area_name);
            event_area_deleted::create_and_post(client,this,area_name);
         }
      } // on_area_deleted_not

      
      void CollectAreasEnumerator::on_stopped_not(Csi::Messaging::Message *message)
      {
         if(state == state_before_active)
         {
            using namespace CollectAreasEnumeratorHelpers;
            event_failure::create_and_post(client,this,client_type::failure_unknown);
         }
      } // on_stopped_not 
   };
};
