/* Cora.Device.CollectArea.FsAreaCreator.cpp

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 20 December 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectArea.FsAreaCreator.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         namespace
         {
            ////////////////////////////////////////////////////////////
            // class event_base
            ////////////////////////////////////////////////////////////
            class event_base: public Csi::Event
            {
            public:
               ////////////////////////////////////////////////////////////
               // creator
               ////////////////////////////////////////////////////////////
               typedef FsAreaCreator creator_type;
               creator_type *creator;

               ////////////////////////////////////////////////////////////
               // client
               ////////////////////////////////////////////////////////////
               typedef FsAreaCreatorClient client_type;
               client_type *client;

            protected:
               ////////////////////////////////////////////////////////////
               // constructor
               ////////////////////////////////////////////////////////////
               event_base(
                  uint4 event_id,
                  creator_type *creator_,
                  client_type *client_):
                  Event(event_id,creator_),
                  creator(creator_),
                  client(client_)
               { }
               
            public:
               ////////////////////////////////////////////////////////////
               // notify
               ////////////////////////////////////////////////////////////
               virtual void notify() = 0;
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
               // collect_area_name
               ////////////////////////////////////////////////////////////
               StrUni collect_area_name;

               ////////////////////////////////////////////////////////////
               // create_and_post
               ////////////////////////////////////////////////////////////
               static void create_and_post(
                  creator_type *creator,
                  client_type *client,
                  StrUni const &collect_area_name)
               {
                  try{(new event_started(creator,client,collect_area_name))->post(); }
                  catch(Csi::Event::BadPost &) { }
               }

               ////////////////////////////////////////////////////////////
               // notify
               ////////////////////////////////////////////////////////////
               virtual void notify()
               {
                  if(client_type::is_valid_instance(client))
                     client->on_started(creator,collect_area_name);
               }

            private:
               ////////////////////////////////////////////////////////////
               // constructor
               ////////////////////////////////////////////////////////////
               event_started(
                  creator_type *creator,
                  client_type *client,
                  StrUni const &collect_area_name_):
                  event_base(event_id,creator,client),
                  collect_area_name(collect_area_name_)
               { } 
            };


            uint4 const event_started::event_id =
            Csi::Event::registerType("Cora::Device::CollectArea::FsAreaCreator::event_started");


            ////////////////////////////////////////////////////////////
            // event_failure
            ////////////////////////////////////////////////////////////
            class event_failure: public event_base
            {
            public:
               ////////////////////////////////////////////////////////////
               // event_id
               ////////////////////////////////////////////////////////////
               static uint4 const event_id;

               ////////////////////////////////////////////////////////////
               // failure
               ////////////////////////////////////////////////////////////
               typedef client_type::failure_type failure_type;
               failure_type failure;

               ////////////////////////////////////////////////////////////
               // create_and_post
               ////////////////////////////////////////////////////////////
               static void create_and_post(
                  creator_type *creator,
                  client_type *client,
                  failure_type failure)
               {
                  try{(new event_failure(creator,client,failure))->post(); }
                  catch(Csi::Event::BadPost &) { }
               }

               ////////////////////////////////////////////////////////////
               // notify
               ////////////////////////////////////////////////////////////
               virtual void notify()
               {
                  if(client_type::is_valid_instance(client))
                     client->on_failure(creator,failure);
               }

            private:
               ////////////////////////////////////////////////////////////
               // constructor
               ////////////////////////////////////////////////////////////
               event_failure(
                  creator_type *creator,
                  client_type *client,
                  failure_type failure_):
                  event_base(event_id,creator,client),
                  failure(failure_)
               { }
            };


            uint4 const event_failure::event_id =
            Csi::Event::registerType("Cora::Device::CollectArea::FsAreaCreator::event_failure");
         };


         ////////////////////////////////////////////////////////////
         // class FsAreaCreator definitions
         ////////////////////////////////////////////////////////////
         FsAreaCreator::FsAreaCreator():
            client(0),
            state(state_standby),
            is_permanent(false),
            area_id(1),
            make_unique_name(false)
         { }

         
         FsAreaCreator::~FsAreaCreator()
         { finish(); }

         
         void FsAreaCreator::set_collect_area_name(StrUni const &collect_area_name_)
         {
            if(state == state_standby)
               collect_area_name = collect_area_name_;
            else
               throw exc_invalid_state();
         } // set_collect_area_name

         
         void FsAreaCreator::set_is_permanent(bool is_permanent_)
         {
            if(state == state_standby)
               is_permanent = is_permanent_;
            else
               throw exc_invalid_state();
         } // set_is_permanent

         
         void FsAreaCreator::set_area_id(uint4 area_id_)
         {
            if(state == state_standby)
               area_id = area_id_;
            else
               throw exc_invalid_state();
         } // set_area_id


         void FsAreaCreator::set_make_unique_name(bool make_unique_name_)
         {
            if(state == state_standby)
               make_unique_name = make_unique_name_;
            else
               throw exc_invalid_state();
         } // set_make_unique_name

         
         void FsAreaCreator::start(
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

         
         void FsAreaCreator::start(
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

         
         void FsAreaCreator::finish()
         {
            state = state_standby;
            client = 0;
            DeviceBase::finish();
         } // finish

         
         void FsAreaCreator::on_devicebase_ready()
         {
            Csi::Messaging::Message command(
               device_session,Messages::final_storage_area_create_cmd);
            command.addUInt4(++last_tran_no);
            command.addWStr(collect_area_name);
            command.addUInt4(area_id);
            command.addBool(is_permanent);
            if(interface_version >= Csi::VersionNumber("1.3.3.6"))
               command.addBool(make_unique_name);
            state = state_active;
            router->sendMessage(&command);
         } // on_devicebase_ready

         
         void FsAreaCreator::on_devicebase_failure(devicebase_failure_type failure)
         {
            client_type::failure_type client_failure;
            switch(failure)
            {
            case devicebase_failure_logon:
               client_failure = client_type::failure_invalid_logon;
               break;
               
            case devicebase_failure_session:
               client_failure = client_type::failure_session_failed;
               break;
               
            case devicebase_failure_invalid_device_name:
               client_failure = client_type::failure_invalid_device_name;
               break;
               
            case devicebase_failure_unsupported:
               client_failure = client_type::failure_unsupported;
               break;
               
            case devicebase_failure_security:
               client_failure = client_type::failure_security_blocked;
               break;
               
            default:
               client_failure = client_type::failure_unknown;
               break;
            }
            event_failure::create_and_post(this,client,client_failure);
         } // on_devicebase_failure

         
         void FsAreaCreator::on_devicebase_session_failure()
         {
            event_failure::create_and_post(this,client,client_type::failure_session_failed);
         } // on_devicebase_session_failure

         
         void FsAreaCreator::onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg)
         {
            if(state == state_active)
            {
               if(msg->getMsgType() == Messages::final_storage_area_create_ack)
               {
                  uint4 tran_no;
                  uint4 resp_code;

                  msg->readUInt4(tran_no);
                  msg->readUInt4(resp_code);
                  if(resp_code == 1)
                  {
                     if(interface_version >= Csi::VersionNumber("1.3.3.6"))
                        msg->readWStr(collect_area_name);
                     event_started::create_and_post(this,client,collect_area_name);
                  }
                  else
                  {
                     client_type::failure_type failure;
                     switch(resp_code)
                     {
                     case 3:
                        failure = client_type::failure_invalid_area_name;
                        break;
                        
                     case 4:
                        failure = client_type::failure_invalid_area_id;
                        break;
                        
                     default:
                        failure = client_type::failure_unknown;
                        break;
                     }
                     event_failure::create_and_post(this,client,failure);
                  }
               }
               else
                  DeviceBase::onNetMessage(rtr,msg);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         } // onNetMessage

         
         void FsAreaCreator::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            event_base *event = dynamic_cast<event_base *>(ev.get_rep());
            if(event != 0)
            {
               if(event->client == client && client_type::is_valid_instance(client))
               {
                  if(event->getType() == event_failure::event_id)
                     finish();
                  event->notify();
               }
               else
                  finish();
            }
         } // receive 
      };
   };
};
