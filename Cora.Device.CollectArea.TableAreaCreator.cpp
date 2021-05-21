/* Cora.Device.CollectArea.TableAreaCreator.cpp

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 17 December 2004
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectArea.TableAreaCreator.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
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
               typedef TableAreaCreator::client_type client_type;
               client_type *client;

               ////////////////////////////////////////////////////////////
               // outcome
               ////////////////////////////////////////////////////////////
               typedef client_type::outcome_type outcome_type;
               outcome_type outcome;

            private:
               ////////////////////////////////////////////////////////////
               // constructor
               ////////////////////////////////////////////////////////////
               event_complete(
                  TableAreaCreator *creator,
                  client_type *client_,
                  outcome_type outcome_):
                  Event(event_id,creator),
                  client(client_),
                  outcome(outcome_)
               { }
               
            public:
               ////////////////////////////////////////////////////////////
               // cpost
               ////////////////////////////////////////////////////////////
               static void cpost(
                  TableAreaCreator *creator,
                  client_type *client,
                  outcome_type outcome)
               {
                  event_complete *event = 0;
                  try
                  {
                     event = new event_complete(creator,client,outcome);
                     event->post();
                  } 
                  catch(Event::BadPost &)
                  { delete event; }
               }
            };


            uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::Device::CollectArea::TableAreaCreator::event_complete");
         };

         
         ////////////////////////////////////////////////////////////
         // class TableAreaCreator definitions
         ////////////////////////////////////////////////////////////
         void TableAreaCreator::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            if(ev->getType() == event_complete::event_id)
            {
               event_complete *event = static_cast<event_complete *>(ev.get_rep());
               client_type *client = this->client;
               finish();
               if(client == event->client && client_type::is_valid_instance(client))
                  client->on_complete(this,event->outcome);
            }
         } // receive

         
         void TableAreaCreator::on_devicebase_ready()
         {
            Csi::Messaging::Message cmd(
               device_session,
               Messages::create_table_area_cmd);
            cmd.addUInt4(++last_tran_no);
            cmd.addWStr(area_name);
            state = state_active;
            router->sendMessage(&cmd);
         } // on_devicebase_ready

         
         void TableAreaCreator::onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg)
         {
            if(state == state_active)
            {
               if(msg->getMsgType() == Messages::create_table_area_ack)
               {
                  uint4 tran_no;
                  uint4 server_response;
                  client_type::outcome_type outcome;
                  
                  msg->readUInt4(tran_no);
                  msg->readUInt4(server_response);
                  switch(server_response)
                  {
                  case 1:
                     outcome = client_type::outcome_success;
                     break;
                     
                  case 3:
                     outcome = client_type::outcome_invalid_area_name;
                     break;
                     
                  default:
                     outcome = client_type::outcome_unknown;
                     break;
                  }
                  event_complete::cpost(this,client,outcome);
               }
               else
                  DeviceBase::onNetMessage(rtr,msg);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         } // onNetMessage

         
         void TableAreaCreator::on_devicebase_failure(devicebase_failure_type failure)
         {
            client_type::outcome_type outcome;
            switch(failure)
            {
            case devicebase_failure_logon:
               outcome = client_type::outcome_invalid_logon;
               break;
               
            case devicebase_failure_session:
               outcome = client_type::outcome_connection_failed;
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
            event_complete::cpost(this,client,outcome);
         } // on_devicebase_failure 
      };
   };
};

