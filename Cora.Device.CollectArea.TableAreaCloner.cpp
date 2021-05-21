/* Cora.Device.CollectArea.TableAreaCloner.cpp

   Copyright (C) 2003, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 02 April 2003
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectArea.TableAreaCloner.h"
#include "Cora.Broker.ValueName.h"


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
               // client
               ////////////////////////////////////////////////////////////
               typedef TableAreaClonerClient client_type;
               client_type *client;

               ////////////////////////////////////////////////////////////
               // cloner
               ////////////////////////////////////////////////////////////
               typedef TableAreaCloner cloner_type;
               cloner_type *cloner;

               ////////////////////////////////////////////////////////////
               // notify
               ////////////////////////////////////////////////////////////
               virtual void notify() = 0;

            protected:
               ////////////////////////////////////////////////////////////
               // constructor
               ////////////////////////////////////////////////////////////
               event_base(
                  uint4 event_id,
                  cloner_type *cloner_,
                  client_type *client_):
                  Event(event_id,cloner_),
                  cloner(cloner_),
                  client(client_)
               { }
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
               // new_area_name
               ////////////////////////////////////////////////////////////
               StrUni new_area_name;

               ////////////////////////////////////////////////////////////
               // notify
               ////////////////////////////////////////////////////////////
               virtual void notify()
               {
                  if(client_type::is_valid_instance(client))
                     client->on_started(cloner,new_area_name);
               }

               ////////////////////////////////////////////////////////////
               // cpost
               ////////////////////////////////////////////////////////////
               static void cpost(
                  cloner_type *cloner,
                  client_type *client,
                  StrUni const &new_area_name)
               {
                  try{(new event_started(cloner,client,new_area_name))->post();}
                  catch(Csi::Event::BadPost &) { }
               }

            private:
               ////////////////////////////////////////////////////////////
               // constructor
               ////////////////////////////////////////////////////////////
               event_started(
                  cloner_type *cloner,
                  client_type *client,
                  StrUni const &new_area_name_):
                  event_base(event_id,cloner,client),
                  new_area_name(new_area_name_)
               { }
            };


            uint4 const event_started::event_id =
            Csi::Event::registerType("Cora::Device::CollectArea::TableAreaCloner::event_started");

            
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
               // failure
               ////////////////////////////////////////////////////////////
               typedef client_type::failure_type failure_type;
               failure_type failure;

               ////////////////////////////////////////////////////////////
               // cpost
               ////////////////////////////////////////////////////////////
               static void cpost(
                  cloner_type *cloner,
                  client_type *client,
                  failure_type failure)
               {
                  try{(new event_failure(cloner,client,failure))->post();}
                  catch(Csi::Event::BadPost &) { }
               }

               ////////////////////////////////////////////////////////////
               // notify
               ////////////////////////////////////////////////////////////
               virtual void notify()
               {
                  if(client_type::is_valid_instance(client))
                     client->on_failure(cloner,failure);
               }

            private:
               ////////////////////////////////////////////////////////////
               // constructor
               ////////////////////////////////////////////////////////////
               event_failure(
                  cloner_type *cloner,
                  client_type *client,
                  failure_type failure_):
                  event_base(event_id,cloner,client),
                  failure(failure_)
               { }
            };


            uint4 const event_failure::event_id =
            Csi::Event::registerType("Cora::Device::CollectArea::TableAreaCloner::event_failure");
         };


         ////////////////////////////////////////////////////////////
         // class TableAreaCloner definitions
         ////////////////////////////////////////////////////////////
         TableAreaCloner::TableAreaCloner():
            client(0),
            state(state_standby),
            is_permanent(false),
            copy_option(copy_none),
            record_no(0),
            file_mark_no(0),
            nsec(0)
         { }

         
         TableAreaCloner::~TableAreaCloner()
         { finish(); }

         
         void TableAreaCloner::set_source_area_name(StrUni const &source_area_name_)
         {
            if(state == state_standby)
               source_area_name = source_area_name_;
            else
               throw exc_invalid_state();
         } // set_source_area_name

         
         void TableAreaCloner::set_new_area_name(StrUni const &new_area_name_)
         {
            if(state == state_standby)
               new_area_name = new_area_name_;
            else
               throw exc_invalid_state();
         } // set_new_area_name

         
         void TableAreaCloner::set_is_permanent(bool is_permanent_)
         {
            if(state == state_standby)
               is_permanent = is_permanent_;
            else
               throw exc_invalid_state();
         } // set_is_permanent


         void TableAreaCloner::set_copy_option(
            copy_option_type copy_option_,
            uint4 file_mark_no_,
            uint4 record_no_,
            int8 nsec_)
         {
            if(state == state_standby)
            {
               copy_option = copy_option_;
               file_mark_no = file_mark_no_;
               record_no = record_no_;
               nsec = nsec_;
            }
            else
               throw exc_invalid_state();
         } // set_copy_option


         void TableAreaCloner::add_column(StrUni const &selector)
         {
            if(state == state_standby)
            {
               selectors.push_back(selector);
            }
            else
               throw exc_invalid_state();
         } // add_column


         void TableAreaCloner::clear_columns()
         {
            if(state == state_standby)
            {
               selectors.clear();
            }
            else
               throw exc_invalid_state();
         } // clear_columns

         
         void TableAreaCloner::start(
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

         
         void TableAreaCloner::start(
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

         
         void TableAreaCloner::finish()
         {
            state = state_standby;
            client = 0;
            DeviceBase::finish();
            lister.clear();
         } // finish

         
         void TableAreaCloner::on_devicebase_ready()
         {
            // we need to start the enumerator as well
            lister.bind(new CollectAreasEnumerator);
            lister->set_device_name(get_device_name()); 
            lister->start(this,this);
            
            // the type of message sent to the server depends upon the server's interface version.
            bool use_extended = false;
            if(get_interface_version() >= Csi::VersionNumber("1.3.6.5"))
               use_extended = true;
            
            // send a command to create the area
            Csi::Messaging::Message command(
               device_session,
               use_extended ? Messages::table_area_clone_ex_cmd : Messages::table_area_clone_cmd);
            command.addUInt4(++last_tran_no);
            command.addWStr(source_area_name);
            command.addWStr(new_area_name);
            command.addBool(is_permanent);
            if(use_extended)
            {
               command.addUInt4((uint4)selectors.size());
               for(selectors_type::iterator si = selectors.begin();
                   si != selectors.end();
                   ++si)
               {
                  typedef Cora::Broker::ValueName value_type;
                  value_type value(si->c_str());
                  command.addWStr(value.get_column_name());
                  command.addUInt4((uint4)value.size());
                  for(value_type::iterator vi = value.begin();
                      vi != value.end();
                      ++vi)
                     command.addUInt4(*vi);
               }
               command.addUInt4(copy_option);
               command.addUInt4(file_mark_no);
               command.addUInt4(record_no);
               command.addInt8(nsec);
            }
            else
               command.addBool(copy_option != copy_none); 
            
            state = state_active;
            router->sendMessage(&command);
         } // on_devicebase_ready
         
         
         void TableAreaCloner::on_devicebase_failure(devicebase_failure_type failure_)
         {
            client_type::failure_type failure;
            switch(failure_)
            {
            case devicebase_failure_logon:
               failure = client_type::failure_invalid_logon;
               break;
               
            case devicebase_failure_session:
               failure = client_type::failure_session_failed;
               break;
               
            case devicebase_failure_invalid_device_name:
               failure = client_type::failure_invalid_device_name;
               break;
               
            case devicebase_failure_unsupported:
               failure = client_type::failure_unsupported;
               break;
               
            case devicebase_failure_security:
               failure = client_type::failure_security_blocked;
               break;
               
            default:
               failure = client_type::failure_unknown;
               break;   
            }
            event_failure::cpost(this,client,failure);
         } // on_devicebase_failure
         
         
         void TableAreaCloner::on_devicebase_session_failure()
         {
            event_failure::cpost(this,client,client_type::failure_session_failed);
         } // on_devicebase_session_failure
         
         
         void TableAreaCloner::onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg)
         {
            if(state == state_active)
            {
               if(msg->getMsgType() == Messages::table_area_clone_ack ||
                  msg->getMsgType() == Messages::table_area_clone_ex_ack)
               {
                  uint4 tran_no;
                  uint4 outcome;
                  msg->readUInt4(tran_no);
                  msg->readUInt4(outcome);
                  if(outcome == 1)
                  {
                     msg->readWStr(new_area_name);
                     event_started::cpost(this,client,new_area_name);
                  }
                  else
                  {
                     client_type::failure_type failure;
                     switch(outcome)
                     {
                     case 3:
                        failure = client_type::failure_invalid_source_name;
                        break;

                     default:
                        failure = client_type::failure_unknown;
                        break;
                     }
                     event_failure::cpost(this,client,failure);
                  }
               }
               else
                  DeviceBase::onNetMessage(rtr,msg);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         } // onNetMessage

         
         void TableAreaCloner::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            event_base *event = static_cast<event_base *>(ev.get_rep());
            client_type *client = this->client;
            if(event->getType() == event_failure::event_id)
               finish();
            if(client == event->client)
               event->notify();
         } // receive


         void TableAreaCloner::on_area_deleted(
            CollectAreasEnumerator *lister,
            StrUni const &area_name)
         {
            if(state == state_active && area_name == new_area_name)
               event_failure::cpost(this,client,client_type::failure_area_deleted);
         } // on_area_deleted


         void TableAreaCloner::on_failure(
            CollectAreasEnumerator *lister,
            failure_type failure_)
         {
            if(state == state_active)
            {
               client_type::failure_type failure;
               switch(failure_)
               {
               case failure_connection:
                  failure = client_type::failure_session_failed;
                  break;
                  
               case failure_server_security_blocked:
                  failure = client_type::failure_security_blocked;
                  break;
                  
               case failure_device_name_invalid:
                  failure = client_type::failure_invalid_device_name;
                  break;
                  
               case failure_not_supported:
                  failure = client_type::failure_unsupported;
                  break;
                  
               default:
                  failure = client_type::failure_unknown;
                  break;
               }
               event_failure::cpost(this,client,failure);
            }
         } // on_failure
      };
   };
};

