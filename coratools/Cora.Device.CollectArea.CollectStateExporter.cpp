/* Cora.Device.CollectArea.CollectStateExporter.cpp

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 16 December 2004
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectArea.CollectStateExporter.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         namespace
         {
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
               typedef CollectStateExporterClient client_type;
               client_type *client;

               ////////////////////////////////////////////////////////////
               // outcome
               ////////////////////////////////////////////////////////////
               typedef client_type::outcome_type outcome_type;
               outcome_type outcome;

               ////////////////////////////////////////////////////////////
               // records_collected
               ////////////////////////////////////////////////////////////
               Csi::RangeList records_collected;

               ////////////////////////////////////////////////////////////
               // records_expected
               ////////////////////////////////////////////////////////////
               Csi::RangeList records_expected;

            private:
               ////////////////////////////////////////////////////////////
               // constructor
               ////////////////////////////////////////////////////////////
               event_complete(
                  CollectStateExporter *exporter,
                  client_type *client_,
                  outcome_type outcome_,
                  Csi::RangeList const &records_collected_,
                  Csi::RangeList const &records_expected_):
                  Event(event_id,exporter),
                  client(client_),
                  outcome(outcome_),
                  records_collected(records_collected_),
                  records_expected(records_expected_)
               { }

            public:
               ////////////////////////////////////////////////////////////
               // cpost (with state)
               ////////////////////////////////////////////////////////////
               static void cpost(
                  CollectStateExporter *exporter,
                  client_type *client,
                  outcome_type outcome,
                  Csi::RangeList const &records_collected,
                  Csi::RangeList const &records_expected)
               {
                  event_complete *event = 0;
                  try
                  {
                     event = new event_complete(
                        exporter,
                        client,
                        outcome,
                        records_collected,
                        records_expected);
                     event->post();
                  }
                  catch(Event::BadPost &)
                  { delete event; }
               }
            
               ////////////////////////////////////////////////////////////
               // cpost (with outcome only)
               ////////////////////////////////////////////////////////////
               static void cpost(
                  CollectStateExporter *exporter,
                  client_type *client,
                  outcome_type outcome)
               {
                  event_complete *event = 0;
                  try
                  {
                     Csi::RangeList records_collected, records_expected;
                     event = new event_complete(
                        exporter,
                        client,
                        outcome,
                        records_collected,
                        records_expected);
                     event->post();
                  }
                  catch(Event::BadPost &)
                  { delete event; }
               }
            };


            uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::Device::CollectArea::CollectStateExporter::on_complete");
         };

      
         ////////////////////////////////////////////////////////////
         // class CollectStateExporter definitions
         ////////////////////////////////////////////////////////////
         void CollectStateExporter::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            if(ev->getType() == event_complete::event_id)
            {
               event_complete *event = static_cast<event_complete *>(ev.get_rep());
               client_type *client = this->client;
               finish();
               if(client_type::is_valid_instance(client) && client == event->client)
               {
                  client->on_complete(
                     this,
                     event->outcome,
                     event->records_collected,
                     event->records_expected);
               }
            }
         } // receive

      
         void CollectStateExporter::onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg)
         {
            if(state == state_active)
            {
               if(msg->getMsgType() == Messages::export_area_collect_state_ack)
               {
                  uint4 tran_no;
                  uint4 server_response;
                  client_type::outcome_type outcome;
                  Csi::RangeList records_collected, records_expected;
                  uint4 count;
               
                  msg->readUInt4(tran_no);
                  msg->readUInt4(server_response);
                  switch(server_response)
                  {
                  case 1:
                     outcome = client_type::outcome_success;
                     msg->readUInt4(count);
                     for(uint4 i = 0; i < count; ++i)
                     {
                        uint4 begin, end;
                        msg->readUInt4(begin);
                        msg->readUInt4(end);
                        records_collected.add_range(begin,end);
                     }
                     msg->readUInt4(count);
                     for(uint4 i = 0; i < count; ++i)
                     {
                        uint4 begin, end;
                        msg->readUInt4(begin);
                        msg->readUInt4(end);
                        records_expected.add_range(begin,end);
                     }
                     break;

                  case 2:
                     outcome = client_type::outcome_invalid_collect_area_name;
                     break;
                  
                  case 3:
                     outcome = client_type::outcome_invalid_collect_area_type;
                     break;
                  
                  default:
                     outcome = client_type::outcome_unknown;
                     break;
                  }
                  event_complete::cpost(
                     this,
                     client,
                     outcome,
                     records_collected,
                     records_expected);
               }
               else
                  DeviceBase::onNetMessage(rtr,msg);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         } // onNetMessage


         void CollectStateExporter::on_devicebase_ready()
         {
            Csi::Messaging::Message cmd(
               device_session,
               Messages::export_area_collect_state_cmd);
            cmd.addUInt4(++last_tran_no);
            cmd.addWStr(area_name);
            state = state_active;
            router->sendMessage(&cmd);
         } // on_devicebase_ready

      
         void CollectStateExporter::on_devicebase_failure(devicebase_failure_type failure)
         {
            client_type::outcome_type outcome;
            switch(failure)
            {
            case devicebase_failure_security:
               outcome = client_type::outcome_server_security_blocked;
               break;
            
            case devicebase_failure_logon:
               outcome = client_type::outcome_invalid_logon;
               break;
            
            case devicebase_failure_session:
               outcome = client_type::outcome_connection_failed;
               break;
            
            case devicebase_failure_invalid_device_name:
               outcome = client_type::outcome_invalid_device_name; 
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
