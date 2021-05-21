/* Cora.Device.TableResetter.cpp

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 06 January 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.TableResetter.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace TableResetterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef TableResetterClient client_type;
            typedef client_type::outcome_type outcome_type;
            client_type *client;
            outcome_type outcome;
            TableResetter *resetter;

         private:
            event_complete(
               TableResetter *resetter_,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,resetter_),
               resetter(resetter_),
               client(client_),
               outcome(outcome_)
            { }

         public:
            static void create_and_post(
               TableResetter *resetter,
               client_type *client,
               outcome_type outcome)
            {
               try {(new event_complete(resetter,client,outcome))->post(); }
               catch(BadPost &) { }
            }
         };


         uint4 const event_complete::event_id = 
         Csi::Event::registerType("Cora::Device::TableResetter::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class TableResetter definitions
      ////////////////////////////////////////////////////////////
      TableResetter::TableResetter():
         client(0),
         state(state_standby)
      { }
      

      TableResetter::~TableResetter()
      { finish(); }


      void TableResetter::set_table_name(StrUni const &table_name_)
      {
         if(state == state_standby)
            table_name = table_name_;
         else
            throw exc_invalid_state();
      } // set_table_name


      void TableResetter::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(TableResetterClient::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void TableResetter::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(TableResetterClient::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start
      

      void TableResetter::finish()
      {
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish


      void TableResetter::format_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace TableResetterStrings;
         switch(outcome)
         {
         case client_type::outcome_table_reset:
            out << my_strings[strid_table_reset];
            break;
            
         case client_type::outcome_communication_failed:
            out << my_strings[strid_communication_failed];
            break;
            
         case client_type::outcome_communication_disabled:
            out << my_strings[strid_communication_disabled];
            break;
            
         case client_type::outcome_invalid_table_name:
            out << my_strings[strid_invalid_table_name];
            break;
            
         case client_type::outcome_logger_security_blocked:
            out << my_strings[strid_logger_security_blocked];
            break;
            
         case client_type::outcome_server_security_blocked:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;
            
         case client_type::outcome_invalid_device_name:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_invalid_logon:
            ClientBase::describe_failure(out, corabase_failure_logon); 
            break;
            
         case client_type::outcome_session_failed:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         default:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // format_outcome
      
      
      void TableResetter::on_devicebase_ready()
      {
         if(get_interface_version() < Csi::VersionNumber("1.3.3.4") &&
            table_name.length() == 0)
         {
            using namespace TableResetterHelpers;
            event_complete::create_and_post(
               this,client,client_type::outcome_invalid_table_name);
         }
         else
         {
            Csi::Messaging::Message command(device_session,Messages::table_reset_cmd);
            command.addUInt4(++last_tran_no);
            if(table_name.length() > 0)
               command.addWStr(table_name);
            router->sendMessage(&command);
            state = state_active;
         }
      } // on_devicebase_ready
      
      
      void TableResetter::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace TableResetterHelpers;
         TableResetterClient::outcome_type outcome;
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = TableResetterClient::outcome_invalid_logon;
            break;

         case devicebase_failure_invalid_device_name:
            outcome = TableResetterClient::outcome_invalid_device_name;
            break;

         case devicebase_failure_unsupported:
            outcome = TableResetterClient::outcome_unsupported;
            break;

         case devicebase_failure_security:
            outcome = TableResetterClient::outcome_server_security_blocked;
            break;

         default:
            outcome = TableResetterClient::outcome_unknown;
            break;
         }
         event_complete::create_and_post(this,client,outcome);
      } // on_devicebase_failure
      
      
      void TableResetter::on_devicebase_session_failure()
      {
         using namespace TableResetterHelpers;
         event_complete::create_and_post(
            this,
            client,
            TableResetterClient::outcome_session_failed);
      } // on_devicebase_session_failure
      
      
      void TableResetter::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::table_reset_ack)
            {
               // read the message
               uint4 tran_no;
               uint4 resp_code;

               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);

               // decide what the outcome was
               using namespace TableResetterHelpers;
               TableResetterClient::outcome_type outcome;

               switch(resp_code)
               {
               case 1:
                  outcome = TableResetterClient::outcome_table_reset;
                  break;

               case 3:
                  outcome = TableResetterClient::outcome_communication_failed;
                  break;

               case 4:
                  outcome = TableResetterClient::outcome_communication_disabled;
                  break;

               case 5:
                  outcome = TableResetterClient::outcome_invalid_table_name;
                  break;

               case 6:
                  outcome = TableResetterClient::outcome_logger_security_blocked;
                  break;

               default:
                  outcome = TableResetterClient::outcome_unknown;
                  break;
               }
               event_complete::create_and_post(this,client,outcome);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage


      void TableResetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace TableResetterHelpers;
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            finish();
            if(TableResetterClient::is_valid_instance(event->client))
               event->client->on_complete(event->resetter,event->outcome);
         }
      } // receive
   };
};
