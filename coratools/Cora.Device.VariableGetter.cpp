/* Cora.Device.VariableGetter.cpp

   Copyright (C) 2003, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 25 September 2003
   Last Change: Thursday 17 October 2019
   Last Commit: $Date: 2019-10-17 11:31:08 -0600 (Thu, 17 Oct 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.VariableGetter.h"
#include "Cora.Broker.Record.h"
#include "Cora.Broker.RecordDesc.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace
      {
         class event_complete: public Csi::Event
         {
         public:
            typedef VariableGetter::client_type client_type;
            client_type *client;
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;
            Csi::SharedPtr<Cora::Broker::Record> record;
            static uint4 const event_id;

         private:
            event_complete(
               VariableGetter *getter,
               client_type *client_,
               outcome_type outcome_,
               Csi::SharedPtr<Cora::Broker::Record> const &record_):
               Event(event_id,getter),
               client(client_),
               outcome(outcome_),
               record(record_)
            { }

         public:
            static void cpost(
               VariableGetter *getter,
               client_type *client,
               outcome_type outcome,
               Csi::SharedPtr<Cora::Broker::Record> record = 0)
            {
               try
               {
                  (new event_complete(getter, client, outcome, record))->post();
               }
               catch(Csi::Event::BadPost &)
               { }
            }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::VariableGetter::event_complete");
      };


      void VariableGetter::start(client_type *client_, router_handle &router)
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

      
      void VariableGetter::start(client_type *client_, ClientBase *other_component)
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

      
      void VariableGetter::finish()
      {
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish


      void VariableGetter::describe_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace VariableGetterStrings;
         switch(outcome)
         {
         default:
         case client_type::outcome_unknown:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
            
         case client_type::outcome_succeeded:
            out << common_strings[common_success];
            break;
            
         case client_type::outcome_connection_failed:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_server_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_invalid_table_name:
            out << my_strings[strid_invalid_table_name];
            break;
            
         case client_type::outcome_invalid_column_name:
            out << my_strings[strid_invalid_column_name];
            break;
            
         case client_type::outcome_invalid_subscript:
            out << my_strings[strid_invalid_subscript];
            break;
            
         case client_type::outcome_communication_failed:
            out << common_strings[common_comm_failed];
            break;
            
         case client_type::outcome_communication_disabled:
            out << common_strings[common_comm_disabled];
            break;
            
         case client_type::outcome_logger_security_blocked:
            out << common_strings[common_logger_security_blocked];
            break;
            
         case client_type::outcome_invalid_table_definition:
            out << common_strings[common_table_defs_invalid];
            break;
            
         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
         }
      } // describe_outcome

      
      void VariableGetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(this,event->outcome,event->record);
            }
            else
               finish();
         }
      } // receive 

      
      void VariableGetter::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Messages::get_variable_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addWStr(table_name);
         cmd.addWStr(column_name);
         cmd.addUInt4((uint4)array_address.size());
         for(array_address_type::iterator ai = array_address.begin();
             ai != array_address.end();
             ++ai)
            cmd.addUInt4(*ai);
         cmd.addUInt4(swath);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      
      void VariableGetter::on_devicebase_failure(devicebase_failure_type failure)
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

         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::cpost(this,client,outcome);
      } // on_devicebase_failure

      
      void VariableGetter::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::get_variable_ack)
            {
               uint4 tran_no;
               uint4 response;

               msg->readUInt4(tran_no);
               msg->readUInt4(response);
               if(response == 1)
               {
                  // parse the data from the server into a record
                  uint4 data_type;
                  uint4 num_values;
                  Csi::SharedPtr<Cora::Broker::RecordDesc> record_desc(
                     new Cora::Broker::RecordDesc(
                        get_device_name(),
                        get_table_name()));
                  Cora::Broker::ValueDesc value_desc;
                  Csi::SharedPtr<Cora::Broker::Record> record;
                  
                  msg->readUInt4(data_type);
                  msg->readUInt4(num_values);
                  value_desc.name = column_name;
                  value_desc.data_type = static_cast<CsiDbTypeCode>(data_type);
                  record_desc->values.reserve(num_values);
                  for(uint4 i = 0; i < num_values; ++i)
                  {
                     if(num_values > 1)
                     {
                        if(value_desc.array_address.empty())
                           value_desc.array_address.push_back(i + 1);
                        else
                           value_desc.array_address.back() = i + 1;
                     }
                     record_desc->values.push_back(new Cora::Broker::ValueDesc(value_desc)); 
                  }
                  record.bind(new Cora::Broker::Record(record_desc,*value_factory));
                  record->read(
                     0,
                     0,
                     Csi::LgrDate::system(),
                     msg->objAtReadIdx(),
                     msg->whatsLeft());
                  event_complete::cpost(
                     this,
                     client,
                     client_type::outcome_succeeded,
                     record);
               }
               else
               {
                  client_type::outcome_type outcome;
                  switch(response)
                  {
                  case 2:
                     outcome = client_type::outcome_invalid_table_name;
                     break;

                  case 3:
                     outcome = client_type::outcome_invalid_column_name;
                     break;

                  case 4:
                     outcome = client_type::outcome_communication_failed;
                     break;

                  case 5:
                     outcome = client_type::outcome_communication_disabled;
                     break;

                  case 6:
                     outcome = client_type::outcome_logger_security_blocked;
                     break;

                  case 8:
                     outcome = client_type::outcome_invalid_subscript;
                     break;

                  case 9:
                     outcome = client_type::outcome_invalid_table_definition;
                     break;

                  default:
                     outcome = client_type::outcome_unknown;
                     break;
                  }
                  event_complete::cpost(this,client,outcome);
               }
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage
   };
};

