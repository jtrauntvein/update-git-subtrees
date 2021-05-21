/* Cora.LgrNet.DeviceMapper.cpp

   Copyright (C) 2002, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 23 August 2002
   Last Change: Tuesday 22 January 2019
   Last Commit: $Date: 2019-01-22 15:04:50 -0600 (Tue, 22 Jan 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.DeviceMapper.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace DeviceMapperHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         protected:
            ////////////////////////////////////////////////////////////
            // mapper
            ////////////////////////////////////////////////////////////
            DeviceMapper *mapper;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef DeviceMapper::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               DeviceMapper *mapper_,
               client_type *client_):
               Event(event_id,mapper_),
               mapper(mapper_),
               client(client_)
            { }

         public:
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify() = 0;

            ////////////////////////////////////////////////////////////
            // get_client
            ////////////////////////////////////////////////////////////
            DeviceMapperClient *get_client() { return client; }
         };


         ////////////////////////////////////////////////////////////
         // class event_change_start
         ////////////////////////////////////////////////////////////
         class event_change_start: public event_base
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
               DeviceMapper *mapper,
               client_type *client)
            {
               try { (new event_change_start(mapper,client))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_change_start(mapper); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_change_start(
               DeviceMapper *mapper,
               client_type *client):
               event_base(event_id,mapper,client)
            { }
         };


         uint4 const event_change_start::event_id  =
         Csi::Event::registerType("Cora::LgrNet::DeviceMapper::event_change_start");


         ////////////////////////////////////////////////////////////
         // class event_change_complete
         ////////////////////////////////////////////////////////////
         class event_change_complete: public event_base
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
               DeviceMapper *mapper,
               client_type *client)
            {
               try { (new event_change_complete(mapper,client))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_change_complete(mapper); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_change_complete(
               DeviceMapper *mapper,
               client_type *client):
               event_base(event_id,mapper,client)
            { }
         };


         uint4 const event_change_complete::event_id  =
         Csi::Event::registerType("Cora::LgrNet::DeviceMapper::event_change_complete");


         ////////////////////////////////////////////////////////////
         // class event_device_added
         ////////////////////////////////////////////////////////////
         class event_device_added: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;
            
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            typedef client_type::device_handle device_handle;
            static void create_and_post(
               DeviceMapper *mapper,
               client_type *client,
               device_handle &device)
            {
               try { (new event_device_added(mapper,client,device))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_device_added(mapper,device); }

         private:
            ////////////////////////////////////////////////////////////
            // device
            ////////////////////////////////////////////////////////////
            device_handle device;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_device_added(
               DeviceMapper *mapper,
               client_type *client,
               device_handle &device_):
               event_base(event_id,mapper,client),
               device(device_)
            { }
         };


         uint4 const event_device_added::event_id  =
         Csi::Event::registerType("Cora::LgrNet::DeviceMapper::event_device_added");


         ////////////////////////////////////////////////////////////
         // class event_device_deleted
         ////////////////////////////////////////////////////////////
         class event_device_deleted: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;
            
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            typedef client_type::device_handle device_handle;
            static void create_and_post(
               DeviceMapper *mapper,
               client_type *client,
               device_handle &device)
            {
               try { (new event_device_deleted(mapper,client,device))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_device_deleted(mapper,device); }

         private:
            ////////////////////////////////////////////////////////////
            // device
            ////////////////////////////////////////////////////////////
            device_handle device;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_device_deleted(
               DeviceMapper *mapper,
               client_type *client,
               device_handle &device_):
               event_base(event_id,mapper,client),
               device(device_)
            { }
         };


         uint4 const event_device_deleted::event_id  =
         Csi::Event::registerType("Cora::LgrNet::DeviceMapper::event_device_deleted");


         ////////////////////////////////////////////////////////////
         // class event_device_renamed
         ////////////////////////////////////////////////////////////
         class event_device_renamed: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;
            
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            typedef client_type::device_handle device_handle;
            static void create_and_post(
               DeviceMapper *mapper,
               client_type *client,
               device_handle &device,
               StrUni const &old_name)
            {
               try { (new event_device_renamed(mapper,client,device,old_name))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_device_renamed(mapper,device,old_name); }

         private:
            ////////////////////////////////////////////////////////////
            // device
            ////////////////////////////////////////////////////////////
            device_handle device;

            ////////////////////////////////////////////////////////////
            // old_name
            ////////////////////////////////////////////////////////////
            StrUni old_name;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_device_renamed(
               DeviceMapper *mapper,
               client_type *client,
               device_handle &device_,
               StrUni const &old_name_):
               event_base(event_id,mapper,client),
               device(device_),
               old_name(old_name_)
            { }
         };


         uint4 const event_device_renamed::event_id  =
         Csi::Event::registerType("Cora::LgrNet::DeviceMapper::event_device_renamed");


         ////////////////////////////////////////////////////////////
         // class event_device_parent_change
         ////////////////////////////////////////////////////////////
         class event_device_parent_change: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;
            
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            typedef client_type::device_handle device_handle;
            static void create_and_post(
               DeviceMapper *mapper,
               client_type *client,
               device_handle &device,
               uint4 old_parent_id)
            {
               try { (new event_device_parent_change(mapper,client,device,old_parent_id))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_device_parent_change(mapper,device,old_parent_id); }

         private:
            ////////////////////////////////////////////////////////////
            // device
            ////////////////////////////////////////////////////////////
            device_handle device;

            ////////////////////////////////////////////////////////////
            // old_parent_id
            ////////////////////////////////////////////////////////////
            uint4 old_parent_id;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_device_parent_change(
               DeviceMapper *mapper,
               client_type *client,
               device_handle &device_,
               uint4 old_parent_id_):
               event_base(event_id,mapper,client),
               device(device_),
               old_parent_id(old_parent_id_)
            { }
         };


         uint4 const event_device_parent_change::event_id  =
         Csi::Event::registerType("Cora::LgrNet::DeviceMapper::event_device_parent_change");


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
               DeviceMapper *mapper,
               client_type *client,
               failure_type failure)
            {
               try { (new event_failure(mapper,client,failure))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(mapper,failure); }

         private:
            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            failure_type failure;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               DeviceMapper *mapper,
               client_type *client,
               failure_type failure_):
               event_base(event_id,mapper,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id  =
            Csi::Event::registerType("Cora::LgrNet::DeviceMapper::event_failure");


         class event_snapshot_restored: public event_base
         {
         public:
            static uint4 const event_id;

            static void cpost(DeviceMapper *sender, DeviceMapperClient *client)
            { (new event_snapshot_restored(sender, client))->post(); }

            virtual void notify()
            { client->on_snapshot_restored(mapper); }

         private:
            event_snapshot_restored(DeviceMapper *mapper, DeviceMapperClient *client):
               event_base(event_id, mapper, client)
            { }
         };


         uint4 const event_snapshot_restored::event_id(
            Csi::Event::registerType("Cora::LgrNet::DeviceMapper::event_snapshot_restored"));
      };


      ////////////////////////////////////////////////////////////
      // class DeviceMapper definitions
      ////////////////////////////////////////////////////////////
      DeviceMapper::DeviceMapper():
         client(0),
         state(state_standby)
      { }
      
         
      DeviceMapper::~DeviceMapper()
      { finish(); }

      
      void DeviceMapper::start(
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
               throw std::invalid_argument("Invalid client pointer specified");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void DeviceMapper::start(
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
               throw std::invalid_argument("Invalid client pointer specified");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void DeviceMapper::finish()
      {
         client = 0;
         state = state_standby;
         ClientBase::finish();
      } // finish


      void DeviceMapper::format_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_invalid_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::failure_session_broken:
            describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::failure_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::failure_server_security:
            describe_failure(out, corabase_failure_security);
            break;
            
         default:
            describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // format_failure


      bool DeviceMapper::find_device(device_handle &device, uint4 object_id)
      {
         bool rtn = false;
         master_list_type::iterator mli = master_list.find(object_id);
         if(mli != master_list.end())
         {
            device = mli->second;
            rtn = true;
         }
         else
            device.clear();
         return rtn;
      } // find_device


      bool DeviceMapper::find_device(device_handle &device, StrUni const &device_name)
      {
         bool rtn = false;
         device_map_type::iterator dmi = device_map.begin();
         while(!rtn && dmi != device_map.end())
         {
            if((*dmi)->get_name() == device_name)
            {
               rtn = true;
               device = *dmi;
            }
            else
               ++dmi;
         }
         if(!rtn)
            device.clear();
         return rtn;
      } // find_device
      

      DeviceMapper::device_handle DeviceMapper::make_device(
         uint4 object_id,
         DeviceTypes::device_type type_code)
      { return new DeviceMapperHelpers::Device(object_id,type_code); }

      
      void DeviceMapper::on_corabase_ready()
      {
         Csi::Messaging::Message start_cmd(
            net_session,
            Messages::network_map_enum_cmd);
         router->sendMessage(&start_cmd);
         state = state_active; 
      } // on_corabase_ready
      
      
      void DeviceMapper::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace DeviceMapperHelpers;
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


      void DeviceMapper::on_snapshot_restored()
      { DeviceMapperHelpers::event_snapshot_restored::cpost(this, client); }
      
      
      void DeviceMapper::on_corabase_session_failure()
      {
         using namespace DeviceMapperHelpers;
         event_failure::create_and_post(this,client,client_type::failure_session_broken);
      } // on_corabase_session_failure
      
      
      void DeviceMapper::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::network_map_advise_not)
            {
               // we need to post a message indicating that a notification has started and that
               // changes are coming.
               using namespace DeviceMapperHelpers;
               event_change_start::create_and_post(this,client);
               
               // read the message header parameters
               uint4 map_version;
               uint4 agent_tran_id;
               uint4 device_count;
               uint4 type;
               uint4 object_id;
               StrUni name;
               uint4 level;

               msg->readUInt4(map_version);
               msg->readUInt4(agent_tran_id);
               msg->readUInt4(device_count);

               // we can now process the device list.  We will clear the device map before doing so
               // as it will be rebuilt during the processing.
               device_map.clear();
               for(uint4 i = 0; i < device_count; ++i)
               {
                  // read the device parameters
                  msg->readUInt4(type);
                  msg->readUInt4(object_id);
                  msg->readWStr(name);
                  msg->readUInt4(level);

                  // look up this device in the master list.  If it is not present it should be
                  // created
                  master_list_type::iterator mli = master_list.find(object_id);
                  device_handle device;
                  bool device_created;
                  
                  if(mli != master_list.end())
                  {
                     device = mli->second;
                     device_created = false;
                  }
                  else
                  {
                     device = make_device(
                        object_id,
                        static_cast<DeviceTypes::device_type>(type));
                     device_created = true;
                     master_list[object_id] = device;
                  }

                  // we should look for changes to the device info (name, etc)
                  if(device->get_name() != name)
                  {
                     if(!device_created)
                        event_device_renamed::create_and_post(this,client,device,device->get_name());
                     device->set_name(name);
                  }
                  device->set_map_version(map_version);
                  device->set_indentation_level(level);
                  if(device_created)
                     event_device_added::create_and_post(this,client,device);
                  device_map.push_back(device);
               }

               // all of the devices that aren't marked with the new map version should now be
               // deleted
               master_list_type::iterator mli = master_list.begin();
               while(mli != master_list.end())
               {
                  if(mli->second->get_map_version() != map_version)
                  {
                     master_list_type::iterator dmli = mli++;
                     event_device_deleted::create_and_post(this,client,dmli->second);
                     master_list.erase(dmli);
                  }
                  else
                     ++mli;
               }

               // the last thing that should be done is to re-assign the child and parent ids in the
               // current device map.
               for(device_map_type::iterator dmi = device_map.begin();
                   dmi != device_map.end();
                   ++dmi)
               {
                  // get the handle to the current device and clear its child list
                  device_handle current = *dmi;
                  current->clear_children();

                  // we now need to scan back in the list to find this device's parent (if any)
                  device_map_type::iterator pi = dmi;
                  device_handle parent;
                  while(pi != device_map.begin())
                  {
                     if((*pi)->get_indentation_level() < current->get_indentation_level())
                     {
                        parent = *pi;
                        break;
                     }
                     --pi;
                  }
                  if(parent == 0)
                     parent = device_map.front();

                  // if this device has a parent, we need to do some processing to put the current
                  // device's id in the parent's list.  We should also evaluate whether the parent
                  // id on the current id has changed.
                  if(parent != 0)
                  {
                     parent->add_child(current->get_object_id());
                     if(current->get_parent_id() != parent->get_object_id())
                     {
                        event_device_parent_change::create_and_post(
                           this,client,current,current->get_parent_id());
                        current->set_parent_id(parent->get_object_id());
                     }
                  }
               }
               event_change_complete::create_and_post(this,client);
            }
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage
      
      
      void DeviceMapper::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace DeviceMapperHelpers;
         event_base *event = static_cast<event_base *>(ev.get_rep());
         client_type *client = event->get_client();
         if(client == this->client && client_type::is_valid_instance(client))
         {
            if(event->getType() == event_failure::event_id)
               finish();
            event->notify();
         }
         else
            finish();
      } // receive 
   };
};
