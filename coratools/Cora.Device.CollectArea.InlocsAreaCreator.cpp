/*  Cora.Device.CollectArea.InlocsAreaCreator.cpp

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written By: Tyler Mecham
   Date Begun: Friday 28 September 2001
   
   Last Changed By: $Author: jon $
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   File Revision Number: $Revision: 27879 $
   
*/


#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectArea.InlocsAreaCreator.h"
#include <assert.h>


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         namespace InlocsAreaCreatorHelpers
         {
            ////////////////////////////////////////////////////////////
            // class event_complete
            ////////////////////////////////////////////////////////////
            class event_complete: public Csi::Event
            {
            public:
               static uint4 const event_id;

               typedef InlocsAreaCreatorClient::outcome_type outcome_type;
               static void create_and_post(
                  InlocsAreaCreator *adder,
                  InlocsAreaCreatorClient *client,
                  outcome_type outcome);

               void notify()
               { client->on_complete(adder,outcome); }

            private:
               InlocsAreaCreator *adder;
               InlocsAreaCreatorClient *client;
               outcome_type outcome;

               event_complete(InlocsAreaCreator *adder_,
                              InlocsAreaCreatorClient *client_,
                              outcome_type outcome_):
                  Event(event_id,adder_),
                  adder(adder_),
                  client(client_),
                  outcome(outcome_)
               { }

               friend class Cora::Device::CollectArea::InlocsAreaCreator;
            };


            uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::LgrNet::InlocsAreaCreator::event_complete");


            void event_complete::create_and_post(
               InlocsAreaCreator *adder,
               InlocsAreaCreatorClient *client,
               outcome_type outcome)
            {
               try { (new event_complete(adder,client,outcome))->post(); }
               catch(Event::BadPost &) { }
            } // create_and_post
         };

            
         ////////////////////////////////////////////////////////////
         // class InlocsAreaCreator definitions
         ////////////////////////////////////////////////////////////
         InlocsAreaCreator::InlocsAreaCreator():
            client(0),
            temporary(true),
            state(state_standby),
            make_unique_name(false)
         { }


         InlocsAreaCreator::~InlocsAreaCreator()
         { finish(); }


         void InlocsAreaCreator::start(
            client_type *client_,
            router_handle &router_)
         {
            if(state == state_standby)
            {
               if(client_type::is_valid_instance(client_))
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


         void InlocsAreaCreator::start(
            client_type *client_,
            ClientBase *other_component)
         {
            if(state == state_standby)
            {
               if(client_type::is_valid_instance(client_))
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


         void InlocsAreaCreator::set_temporary(bool temporary_)
         {
            if( state == state_standby )
               temporary = temporary_;
            else
               throw exc_invalid_state();
         } // set_temporary


         void InlocsAreaCreator::set_collect_area_name(StrUni const &area_name_)
         {
            if( state == state_standby )
               area_name = area_name_;
            else
               throw exc_invalid_state();
         } // set_collect_area_name


         void InlocsAreaCreator::add_field(uint2 inloc_number, StrUni const &field_name)
         {
            if( state == state_standby )
               locations.push_back(InlocId(inloc_number,field_name));
            else
               throw exc_invalid_state();
         } // add_field


         void InlocsAreaCreator::clear_fields()
         {
            if(state == state_standby)
               locations.clear();
            else
               throw exc_invalid_state();
         } // clear_fields


         void InlocsAreaCreator::set_make_unique_name(bool make_unique_name_)
         {
            if(state == state_standby)
               make_unique_name = make_unique_name_;
            else
               throw exc_invalid_state();
         } // set_make_unique_name
            
            
         void InlocsAreaCreator::onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg)
         {
            if(msg->getMsgType() == Messages::input_locations_area_create_ack)
            {
               uint4 tran_no;
               uint4 resp_code;
               client_type::outcome_type outcome;

               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               switch(resp_code)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  if(make_unique_name &&
                     get_interface_version() >= Csi::VersionNumber("1.3.3.6"))
                     msg->readWStr(area_name);
                  break;

               case 2:
                  outcome = client_type::outcome_invalid_tran_no;
                  break;

               case 3:
                  outcome = client_type::outcome_invalid_collect_area_name;
                  break;

               case 4:
                  outcome = client_type::outcome_invalid_number_of_identifiers;
                  break;

               case 5:
                  outcome = client_type::outcome_invalid_inloc_id;
                  break;

               case 6:
                  outcome = client_type::outcome_invalid_field_name;
                  break;

               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               InlocsAreaCreatorHelpers::event_complete::create_and_post(this,client,outcome); 
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         } // onNetMessage


         void InlocsAreaCreator::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            using namespace InlocsAreaCreatorHelpers;

            if(ev->getType() == event_complete::event_id)
            {
               event_complete *event = static_cast<event_complete *>(ev.get_rep());
               client_type *client = this->client;
               if(event->outcome != client_type::outcome_success)
                  finish();
               if(event->client == client && client_type::is_valid_instance(event->client))
                  event->notify(); 
            }
         } // receive 


         void InlocsAreaCreator::on_devicebase_ready()
         {
            state = state_steady;

            Csi::Messaging::Message cmd(
               device_session,
               Messages::input_locations_area_create_cmd);

            cmd.addUInt4(++last_tran_no);
            cmd.addWStr(area_name.c_str());
            cmd.addBool(!temporary);
            cmd.addUInt4((uint4)locations.size());
            for(locations_type::iterator li = locations.begin(); li != locations.end(); ++li)
            {
               cmd.addUInt2(li->inloc_id);
               cmd.addWStr(li->field_name.c_str());
            }
            if(interface_version >= Csi::VersionNumber("1.3.3.6"))
               cmd.addBool(make_unique_name);
            router->sendMessage(&cmd);
         }


         void InlocsAreaCreator::on_devicebase_failure(devicebase_failure_type failure)
         {
            using namespace InlocsAreaCreatorHelpers;
            client_type::outcome_type outcome;
            switch(failure)
            {
            case devicebase_failure_logon:
               outcome = client_type::outcome_invalid_logon;
               break;

            case devicebase_failure_session:
               outcome = client_type::outcome_session_failed;
               break;

            case devicebase_failure_invalid_device_name:
               outcome = client_type::outcome_invalid_device_name;
               break;

            case devicebase_failure_unsupported:
               outcome = client_type::outcome_unsupported;
               break;

            case devicebase_failure_security:
               outcome = client_type::outcome_server_security_blocked;
               break;

            default:
               outcome = client_type::outcome_unknown;
               break;
            }
            event_complete::create_and_post(this,client,outcome);
         } // on_devicebase_failure


         void InlocsAreaCreator::on_devicebase_session_failure()
         {
            using namespace InlocsAreaCreatorHelpers;
            event_complete::create_and_post(this,client,client_type::outcome_session_failed);
         } // on_devicebase_session_failure


         void InlocsAreaCreator::finish()
         {
            state = state_standby;
            client = 0;
            DeviceBase::finish();
         } // finish
      };
   };
};
