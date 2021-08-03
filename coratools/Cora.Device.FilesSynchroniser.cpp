/* Cora.Device.FilesSynchroniser.cpp

   Copyright (C) 2008, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 04 January 2008
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.FilesSynchroniser.h"
#include "coratools.strings.h"
#include "boost/format.hpp"


namespace Cora
{
   namespace Device
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_status
         ////////////////////////////////////////////////////////////
         class event_status: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef FilesSynchroniser::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // status
            ////////////////////////////////////////////////////////////
            typedef client_type::status_type status_type;
            status_type status;

            ////////////////////////////////////////////////////////////
            // file_name
            ////////////////////////////////////////////////////////////
            StrAsc file_name;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               FilesSynchroniser *receiver,
               client_type *client,
               uint4 status,
               StrAsc const &file_name)
            {
               event_status *event = new event_status(
                  receiver, client, static_cast<status_type>(status), file_name);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_status(
               FilesSynchroniser *receiver,
               client_type *client_,
               status_type status_,
               StrAsc const &file_name_):
               Event(event_id, receiver),
               client(client_),
               status(status_),
               file_name(file_name_)
            { }
         };


         uint4 const event_status::event_id =
            Csi::Event::registerType("Cora::Device::FilesSynchroniser::event_status");


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
            typedef FilesSynchroniserClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               FilesSynchroniser *receiver,
               client_type *client,
               outcome_type outcome)
            {
               event_complete *event = new event_complete(receiver, client, outcome);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               FilesSynchroniser *receiver,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id, receiver),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::Device::FilesSynchroniser::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class FilesSynchroniser definitions
      ////////////////////////////////////////////////////////////
      FilesSynchroniser::FilesSynchroniser():
         state(state_standby),
         client(0),
         rules(new rules_type)
      { }


      void FilesSynchroniser::set_rules(char const *rules_)
      {
         if(state == state_standby)
         {
            rules.bind(new rules_type);
            if(!rules->read(rules_))
               throw std::invalid_argument("invalid rules format");
         }
         else
            throw exc_invalid_state();
      } // set_rules


      void FilesSynchroniser::set_rules(rules_handle rules_)
      {
         if(state == state_standby)
            rules = rules_;
         else
            throw exc_invalid_state();
      } // set_rules


      void FilesSynchroniser::start(client_type *client_, router_handle router)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("Invalid client pointer");
         client = client_;
         state = state_delegate;
         DeviceBase::start(router);
      } // start


      void FilesSynchroniser::start(client_type *client_, ClientBase *other_client)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("Invalid client pointer");
         client = client_;
         state = state_delegate;
         DeviceBase::start(other_client);
      } // start


      void FilesSynchroniser::finish()
      {
         DeviceBase::finish();
      } // finish


      void FilesSynchroniser::format_status(
         std::ostream &out, client_type::status_type status, StrAsc const &file_name)
      {
         using namespace FilesSynchroniserStrings;
         switch(status)
         {
         case client_type::status_getting_dir:
            out << my_strings[strid_status_getting_dir];
            break;
            
         case client_type::status_file_already_retrieved:
            out << boost::format(my_strings[strid_status_file_already_retrieved].c_str()) % file_name;
            break;
            
         case client_type::status_starting_retrieve:
            out << boost::format(my_strings[strid_status_starting_retrieve].c_str()) % file_name;
            break;
            
         case client_type::status_retrieve_failed:
            out << boost::format(my_strings[strid_status_retrieve_failed].c_str()) % file_name;
            break;
            
         case client_type::status_file_skipped:
            out << boost::format(my_strings[strid_status_file_skipped].c_str()) % file_name;
            break;
            
         case client_type::status_file_retrieved:
            out << boost::format(my_strings[strid_status_file_retrieved].c_str()) % file_name;
            break;
         }
      } // format_status


      void FilesSynchroniser::format_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace FilesSynchroniserStrings;
         switch(outcome)
         {
         case client_type::outcome_unknown:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_unknown);
            break;
            
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case client_type::outcome_comm_failed:
            out << my_strings[strid_outcome_comm_failed];
            break;
            
         case client_type::outcome_comm_disabled:
            out << my_strings[strid_outcome_comm_disabled];
            break;
            
         case client_type::outcome_invalid_logger_security:
            out << my_strings[strid_outcome_invalid_logger_security];
            break;
            
         case client_type::outcome_invalid_device_name:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_invalid_server_security:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_session_failed:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_invalid_logon:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_unsupported:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
         }
      } // format_outcome


      void FilesSynchroniser::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(device_session, Messages::synch_files_cmd);
         cmd.addUInt4(++last_tran_no);
         rules->write(&cmd);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready


      void FilesSynchroniser::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::outcome_type outcome = client_type::outcome_unknown;
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
            outcome = client_type::outcome_invalid_server_security;
            break;
         }
         event_complete::cpost(this, client, outcome);
      } // on_devicebase_failure


      void FilesSynchroniser::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::synch_files_status_not)
            {
               uint4 tran_no;
               uint4 status;
               StrAsc file_name;
               message->readUInt4(tran_no);
               message->readUInt4(status);
               message->readStr(file_name);
               event_status::cpost(this, client, status, file_name);
            }
            else if(message->getMsgType() == Messages::synch_files_ack)
            {
               uint4 tran_no;
               uint4 server_outcome;
               client_type::outcome_type outcome = client_type::outcome_unknown;
               
               message->readUInt4(tran_no);
               message->readUInt4(server_outcome);
               switch(server_outcome)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;
                  
               case 3:
                  outcome = client_type::outcome_comm_failed;
                  break;
                  
               case 4:
                  outcome = client_type::outcome_comm_disabled;
                  break;
                  
               case 5:
                  outcome = client_type::outcome_invalid_logger_security;
                  break;
               }
               event_complete::cpost(this, client, outcome);
            }
            else
               DeviceBase::onNetMessage(router, message);
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage


      void FilesSynchroniser::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_status::event_id)
         {
            event_status *event = static_cast<event_status *>(ev.get_rep());
            if(state == state_active && event->client == client && client_type::is_valid_instance(client))
               client->on_status(this, event->status, event->file_name);
         }
         else if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(this, event->outcome);
            }
         }
      } // receive
   };
};
