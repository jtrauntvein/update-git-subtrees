/* Cora.Broker.TableResizer.cpp

   Copyright (C) 2005, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 January 2005
   Last Change: Monday 12 April 2021
   Last Commit: $Date: 2021-04-12 15:37:10 -0600 (Mon, 12 Apr 2021) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.TableResizer.h"
#include <iostream>


namespace Cora
{
   namespace Broker
   {
      namespace
      {
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef TableResizer::client_type client_type;
            client_type *client;
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

         private:
            event_complete(
               TableResizer *resizer, client_type *client_, outcome_type outcome_):
               Event(event_id,resizer),
               client(client_),
               outcome(outcome_)
            { }

         public:
            static void cpost(
               TableResizer *resizer,
               client_type *client,
               outcome_type outcome)
            {
               event_complete *event = new event_complete(resizer,client,outcome);
               try { event->post(); }
               catch(Csi::Event::BadPost &) { delete event; }
            }
         };
         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::Broker::TableResizer::event_complete");
      };

      
      void TableResizer::receive(Csi::SharedPtr<Csi::Event> &ev)
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

      void TableResizer::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_success:
            out << "success";
            break;
            
         case client_type::outcome_invalid_logon:
            format_failure(out, brokerbase_failure_logon);
            break;
            
         case client_type::outcome_connection_failed:
            format_failure(out, brokerbase_failure_session);
            break;
            
         case client_type::outcome_invalid_station_name:
            format_failure(out, brokerbase_failure_invalid_id);
            break;
            
         case client_type::outcome_unsupported:
            format_failure(out, brokerbase_failure_unsupported);
            break;
            
         case client_type::outcome_server_security_blocked:
            format_failure(out, brokerbase_failure_security);
            break;
            
         case client_type::outcome_invalid_table_name:
            out << "invalid table name";
            break;
            
         case client_type::outcome_invalid_size:
            out << "invalid size";
            break;
            
         case client_type::outcome_insufficient_resources:
            out << "insufficient resources";
            break;
            
         default:
            format_failure(out, brokerbase_failure_unknown);
            break;
         }
      } // describe_outcomew

      void TableResizer::on_brokerbase_ready()
      {
         Csi::Messaging::Message cmd(
            broker_session,
            Messages::table_resize_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addWStr(table_name);
         cmd.addUInt4(table_size);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_brokerbase_ready
      
      void TableResizer::onNetMessage(
         Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::table_resize_ack)
            {
               uint4 tran_no;
               uint4 server_outcome;
               client_type::outcome_type client_outcome;

               msg->readUInt4(tran_no);
               msg->readUInt4(server_outcome);
               switch(server_outcome)
               {
               case 1:
                  client_outcome = client_type::outcome_success;
                  break;

               case 2:
                  client_outcome = client_type::outcome_invalid_table_name;
                  break;

               case 3:
                  client_outcome = client_type::outcome_invalid_size;
                  break;

               case 5:
                  client_outcome = client_type::outcome_insufficient_resources;
                  break;

               default:
                  client_outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::cpost(this,client,client_outcome);
            }
            else
               BrokerBase::onNetMessage(rtr,msg);
         }
         else
            BrokerBase::onNetMessage(rtr,msg); 
      } // onNetMessage
      
      void TableResizer::on_brokerbase_failure(
         brokerbase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case brokerbase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;

         case brokerbase_failure_session:
            outcome = client_type::outcome_connection_failed;
            break;
            
         case brokerbase_failure_invalid_id:
            outcome = client_type::outcome_invalid_station_name;
            break;
            
         case brokerbase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case brokerbase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::cpost(this,client,outcome);
      } // on_brokerbase_failure 
   };
};

