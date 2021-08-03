/*  File Name: $RCSfile: Cora.Device.CollectArea.CollectAreaMaintainer.cpp,v $

Copyright (C) 2001, 2016 Campbell Scientific, Inc.

Written By: Tyler Mecham
Date Begun: 9/28/2001 14:33:16

Last Changed By: $Author: jon $
Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
File Revision Number: $Revision: 27879 $
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectArea.CollectAreaMaintainer.h"
#include "Cora.Device.CollectArea.CollectAreaSettingTypes.h"
#include "Cora.Device.DeviceSettingTypes.h"
#include <sstream>
#include <assert.h>


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         namespace CollectAreaMaintainerHelpers
         {
            ////////////////////////////////////////////////////////////
            // class event_started
            ////////////////////////////////////////////////////////////
            class event_started: public Csi::Event
            {
            public:
               static uint4 const event_id;

               static void create_and_post(CollectAreaMaintainer *adder,CollectAreaMaintainerClient *client);

               void notify()
               { client->on_started(adder); }

               CollectAreaMaintainer *adder;
               CollectAreaMaintainerClient *client;

            private:

               event_started(CollectAreaMaintainer *adder_,
                             CollectAreaMaintainerClient *client_):
                  Event(event_id,adder_),
                  adder(adder_),
                  client(client_)
               { }
            };

            uint4 const event_started::event_id =
            Csi::Event::registerType("Cora::LgrNet::CollectAreaMaintainer::event_started");


            void event_started::create_and_post(CollectAreaMaintainer *adder,
                                                CollectAreaMaintainerClient *client)
            {
               try { (new event_started(adder,client))->post(); }
               catch(Event::BadPost &) { }
            } // create_and_post

                
            ////////////////////////////////////////////////////////////
            // class event_failure
            ////////////////////////////////////////////////////////////
            class event_failure: public Csi::Event
            {
            public:
               static uint4 const event_id;

               typedef CollectAreaMaintainerClient::failure_type failure_type;
               static void create_and_post(
                  CollectAreaMaintainer *adder,
                  CollectAreaMaintainerClient *client,
                  failure_type failure);

               void notify()
               { client->on_failure(adder,failure); }

               CollectAreaMaintainer *adder;
               CollectAreaMaintainerClient *client;
               failure_type failure;

            private:
               event_failure(CollectAreaMaintainer *adder_,
                             CollectAreaMaintainerClient *client_,
                             failure_type failure_):
                  Event(event_id,adder_),
                  adder(adder_),
                  client(client_),
                  failure(failure_)
               { }
            };


            uint4 const event_failure::event_id =
            Csi::Event::registerType("Cora::LgrNet::CollectAreaMaintainer::event_failure");


            void event_failure::create_and_post(CollectAreaMaintainer *adder,
                                                CollectAreaMaintainerClient *client,
                                                failure_type failure)
            {
               try { (new event_failure(adder,client,failure))->post(); }
               catch(Event::BadPost &) { }
            } // create_and_post
         };

            
         ////////////////////////////////////////////////////////////
         // class CollectAreaMaintainer definitions
         ////////////////////////////////////////////////////////////


         CollectAreaMaintainer::CollectAreaMaintainer(Csi::SharedPtr<OneShot> oneshot_):
            client(0),
            state(state_standby),
            pending_inlocs_exist(false),
            send_settings_id(0)
         {
            if(oneshot_ == 0)
               oneshot.bind(new OneShot);
            else
               oneshot = oneshot_;
            inlocs_area_creator.bind(new InlocsAreaCreator);
         }


         CollectAreaMaintainer::~CollectAreaMaintainer()
         {
            finish();
         }


         void CollectAreaMaintainer::start(
            CollectAreaMaintainerClient *client_,
            router_handle &router_)
         {
            if(state == state_standby)
            {
               pending_inlocs_exist = false;
               if(CollectAreaMaintainerClient::is_valid_instance(client_))
               {
                  state = state_attaching;
                  client = client_;
                  DeviceBase::start(router_);
               }
               else
                  throw std::invalid_argument("Invalid client pointer");
            }
            else
               throw exc_invalid_state();
         } // start


         void CollectAreaMaintainer::start(
            CollectAreaMaintainerClient *client_,
            ClientBase *other_component)
         {
            if(state == state_standby)
            {
               pending_inlocs_exist = false;
               if(CollectAreaMaintainerClient::is_valid_instance(client_))
               {
                  state = state_attaching;
                  client = client_;
                  DeviceBase::start(other_component);
               }
               else
                  throw std::invalid_argument("Invalid client pointer");
            }
            else
               throw exc_invalid_state();
         } // start
           

         void CollectAreaMaintainer::finish()
         {
            state = state_standby;
            setting_setters.clear();
            locations.clear();
            ref_counts.clear();
            client = 0;
            inlocs_area_creator->finish();
            DeviceBase::finish();
         }


         void CollectAreaMaintainer::set_collect_area_name(
            StrUni const &area_name_,
            bool make_unique)
         {
            if( state == state_standby )
            {
               inlocs_area_creator->set_collect_area_name(area_name_);
               inlocs_area_creator->set_make_unique_name(make_unique);
            }
            else
            {
               throw exc_invalid_state();
            }
         }


         bool CollectAreaMaintainer::exists(
            uint2 inloc_number,
            StrUni const &field_name)
         {
            bool alreadyExists = false;
            std::list<InlocId>::iterator it = locations.begin();
            while( it != locations.end() )
            {
               if( (*it).inloc_id == inloc_number || (*it).field_name == field_name )
               {
                  alreadyExists = true;
                  uint2 ref = ref_counts[inloc_number];
                  ref += 1;
                  ref_counts[inloc_number] = ref;
                  break;
               }
               ++it;
            }
            return alreadyExists;
         }


         void CollectAreaMaintainer::add_field(uint2 inloc_number, StrUni const &field_name)
         {
            if( !exists(inloc_number,field_name) )
            {
               ref_counts[inloc_number] = 1;
               locations.push_back(InlocId(inloc_number,field_name));
                
               //We only want to add fields to the creator before it is actually created
               //after that, we will have to modify the collect area setting to modify
               //the fields in the collect area.
               if( state == state_standby )
               {
                  inlocs_area_creator->add_field(inloc_number,field_name);
               }
               else if( state == state_attaching )
               {
                  pending_inlocs_exist = true;
               }
               else
               {
                  send_field_names();
               }
            }
         }
            
            
         void CollectAreaMaintainer::remove_field(uint2 inloc_number, StrUni const &field_name)
         {
            std::list<InlocId>::iterator it = locations.begin();
            while( it != locations.end() )
            {
               if((*it).inloc_id == inloc_number && 
                  (*it).field_name == field_name)
               {
                  break;
               }
               ++it;
            }

            //see if we found a match
            if( it != locations.end() )
            {
               int ref = ref_counts[inloc_number];
               ref -= 1;
               ref_counts[inloc_number] = ref;
               //Check to see if anyone else is referencing this column
               if( ref == 0 )
               {
                  //remove it from our list
                  locations.erase(it);
                    
                  //change the input locations in the collect area
                  Csi::SharedPtr<Cora::Device::CollectArea::SettingSetter> setting_setter(new Cora::Device::CollectArea::SettingSetter);
                  setting_setter->set_device_name(get_device_name());
                  setting_setter->set_collect_area_name(get_collect_area_name());
                  Cora::Device::SettingInlocIds *setting = new Cora::Device::SettingInlocIds(Cora::Device::CollectArea::Settings::inloc_ids);
                  std::ostringstream inlocs;
                  size_t size = locations.size();
                  inlocs << size;
                  std::list<InlocId>::iterator it = locations.begin();
                  while( it != locations.end() )
                  {
                     inlocs << " {" << (*it).inloc_id << " " << (*it).field_name << "}";
                     ++it;
                  }
                  setting->read(inlocs.str().c_str());
                  setting_setter->set_the_setting(setting);
                  setting_setter->start(this,this);
                  setting_setters.push_back(setting_setter);
               }
            }
         }
            
            
         void CollectAreaMaintainer::onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg)
         {
            DeviceBase::onNetMessage(rtr,msg);
         } // onNetMessage


         void CollectAreaMaintainer::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            using namespace CollectAreaMaintainerHelpers;

            if(ev->getType() == event_started::event_id)
            {
               state = state_steady;

               CollectAreaMaintainerHelpers::event_started *event = static_cast<event_started *>(ev.get_rep());
               if(CollectAreaMaintainerClient::is_valid_instance(event->client))
                  event->notify(); 
            }
            else if(ev->getType() == event_failure::event_id)
            {
               CollectAreaMaintainerHelpers::event_failure *event = static_cast<event_failure *>(ev.get_rep());
               if(CollectAreaMaintainerClient::is_valid_instance(event->client))
                  event->notify(); 
            }
            else
               assert(false);
         } // receive 


         void CollectAreaMaintainer::on_devicebase_ready()
         {
            inlocs_area_creator->set_device_name(get_device_name());
            inlocs_area_creator->start(this,this);
         }


         void CollectAreaMaintainer::on_devicebase_failure(devicebase_failure_type failure_)
         {
            using namespace CollectAreaMaintainerHelpers;
            CollectAreaMaintainerClient::failure_type failure;
            switch(failure_)
            {
            case devicebase_failure_logon:
               failure = CollectAreaMaintainerClient::failure_invalid_logon;
               break;

            case devicebase_failure_session:
               failure = CollectAreaMaintainerClient::failure_session_failed;
               break;

            case devicebase_failure_invalid_device_name:
               failure = CollectAreaMaintainerClient::failure_invalid_device_name;
               break;

            case devicebase_failure_unsupported:
               failure = CollectAreaMaintainerClient::failure_unsupported;
               break;

            case devicebase_failure_security:
               failure = CollectAreaMaintainerClient::failure_server_security_blocked;
               break;

            default:
               failure = CollectAreaMaintainerClient::failure_unknown;
               break;
            }
            event_failure::create_and_post(this,client,failure);
         } // on_devicebase_failure


         void CollectAreaMaintainer::on_devicebase_session_failure()
         {
            using namespace CollectAreaMaintainerHelpers;
            event_failure::create_and_post(this,client,CollectAreaMaintainerClient::failure_session_failed);
         } // on_devicebase_session_failure


         void CollectAreaMaintainer::on_complete(
            Cora::Device::CollectArea::InlocsAreaCreator *creator,
            Cora::Device::CollectArea::InlocsAreaCreatorClient::outcome_type outcome)
         {
            if( pending_inlocs_exist )
            {
               send_field_names();
            }
               
            //Enable the collect area for scheduled collection
            Csi::SharedPtr<Cora::Device::CollectArea::SettingSetter> setting_setter(new Cora::Device::CollectArea::SettingSetter);
            setting_setter->set_device_name(get_device_name());
            setting_setter->set_collect_area_name(get_collect_area_name());
            Cora::SettingBool *setting = new Cora::SettingBool(Cora::Device::CollectArea::Settings::schedule_enabled);
            setting->set_value(true);
            setting_setter->set_the_setting(setting);
            setting_setter->start(this,this);
            setting_setters.push_back(setting_setter);

            using namespace CollectAreaMaintainerHelpers;
            event_started::create_and_post(this,client);
         }

        
         void CollectAreaMaintainer::on_complete(Cora::Device::CollectArea::SettingSetter *setter,
                                                 Cora::Device::CollectArea::SettingSetterClient::outcome_type resp_code)
         {
            std::list<Csi::SharedPtr<Cora::Device::CollectArea::SettingSetter> >::iterator it = setting_setters.begin();
            while( it != setting_setters.end() )
            {
               if( (*it).get_rep() == setter )
               {
                  setting_setters.erase(it);
                  return;
               }
               ++it;
            }
         }


         void CollectAreaMaintainer::onOneShotFired(uint4 id)
         {
            if(id == send_settings_id)
            {
               send_settings_id = 0;
               //change the input locations in the collect area
               Csi::SharedPtr<Cora::Device::CollectArea::SettingSetter> setting_setter(new Cora::Device::CollectArea::SettingSetter);
               setting_setter->set_device_name(get_device_name());
               setting_setter->set_collect_area_name(get_collect_area_name());
               Cora::Device::SettingInlocIds *setting = new Cora::Device::SettingInlocIds(Cora::Device::CollectArea::Settings::inloc_ids);
               std::ostringstream inlocs;
               size_t size = locations.size();
               inlocs << size;
               std::list<InlocId>::iterator it = locations.begin();
               while( it != locations.end() )
               {
                  inlocs << " {" << (*it).inloc_id << " " << (*it).field_name << "}";
                  ++it;
               }
               setting->read(inlocs.str().c_str());
               setting_setter->set_the_setting(setting);
               setting_setter->start(this,this);
               setting_setters.push_back(setting_setter);
            }
         }


         void CollectAreaMaintainer::send_field_names()
         {
            if(send_settings_id == 0)
               send_settings_id = oneshot->arm(this,1000);
            else
               oneshot->reset(send_settings_id);
         }
      };
   };
};
