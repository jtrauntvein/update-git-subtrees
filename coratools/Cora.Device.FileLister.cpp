/* Cora.Device.FileLister.cpp

   Copyright (C) 2002, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 10 April 2002
   Last Change: Monday 07 April 2014
   Last Commit: $Date: 2014-04-07 15:03:58 -0600 (Mon, 07 Apr 2014) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.FileLister.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace FileListerHelpers
      {
         ////////////////////////////////////////////////////////////
         // class ev_complete
         ////////////////////////////////////////////////////////////
         class ev_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef FileListerClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // lister
            ////////////////////////////////////////////////////////////
            FileLister *lister;

            ////////////////////////////////////////////////////////////
            // files
            ////////////////////////////////////////////////////////////
            typedef client_type::file_list_type files_type;
            files_type files;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            ev_complete(
               FileLister *lister_,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,lister_),
               lister(lister_),
               client(client_),
               outcome(outcome_)
            { }
            
         public:
            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static ev_complete *create(
               FileLister *lister,
               client_type *client,
               outcome_type outcome)
            { return new ev_complete(lister,client,outcome); } 
         };


         uint4 const ev_complete::event_id =
         Csi::Event::registerType("Cora::Device::FileLister:;event_complete"); 
      };


      ////////////////////////////////////////////////////////////
      // class FileLister definitions
      ////////////////////////////////////////////////////////////
      FileLister::FileLister():
         client(0),
         state(state_standby)
      { }


      FileLister::~FileLister()
      { finish(); }


      void FileLister::set_pattern(StrAsc const &pattern_)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         pattern = pattern_;
      }
      

      void FileLister::start(
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


      void FileLister::start(
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


      void FileLister::finish()
      {
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish


      void FileLister::describe_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_success:
            out << common_strings[common_success];
            break;
            
         case client_type::outcome_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_session_failure:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_blocked_by_server:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         case client_type::outcome_blocked_by_logger:
            out << common_strings[common_logger_security_blocked];
            break;
            
         case client_type::outcome_comm_disabled:
            out << common_strings[common_comm_disabled];
            break;
            
         case client_type::outcome_comm_failed:
            out << common_strings[common_comm_failed];
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         } 
      } // describe_outcome


      void FileLister::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(device_session,Messages::files_enum_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addStr(pattern);
         router->sendMessage(&cmd);
         state = state_active; 
      } // on_devicebase_ready


      void FileLister::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace FileListerHelpers;
         ev_complete *event = ev_complete::create(this,client,client_type::outcome_unknown);
         switch(failure)
         {
         case devicebase_failure_logon:
            event->outcome = client_type::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            event->outcome = client_type::outcome_session_failure;
            break;
            
         case devicebase_failure_invalid_device_name:
            event->outcome = client_type::outcome_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            event->outcome = client_type::outcome_unsupported;
            break;
            
         case devicebase_failure_security:
            event->outcome = client_type::outcome_blocked_by_server;
            break;
         }
         try { event->post(); }
         catch(Csi::Event::BadPost &) { }
      } // on_devicebase_failure


      void FileLister::on_devicebase_session_failure()
      {
         try
         {
            using namespace FileListerHelpers;
            ev_complete *event = ev_complete::create(
               this,client,client_type::outcome_session_failure);
            event->post();
         }
         catch(Csi::Event::BadPost &)
         { }
      } // on_devicebase_session_failure


      void FileLister::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace FileListerHelpers;
         if(ev->getType() == ev_complete::event_id)
         {
            ev_complete *event = static_cast<ev_complete *>(ev.get_rep());
            finish();
            if(client_type::is_valid_instance(event->client))
               event->client->on_complete(this,event->outcome,event->files);
         }
      } // receive


      void FileLister::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::files_enum_ack)
            {
               // crack the message
               using namespace FileListerHelpers;
               uint4 tran_no;
               uint4 resp_code;
               uint4 file_count;
               ev_complete *event = ev_complete::create(this,client,client_type::outcome_unknown);

               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               msg->readUInt4(file_count);
               if(resp_code == 1)
               {
                  uint4 attrib_count;
                  uint4 attrib_id;
                  uint4 attrib_size;
                  uint4 file_size;
                  
                  event->outcome = client_type::outcome_success;
                  for(uint4 i = 0; i < file_count; ++i)
                  {
                     file_type file;
                     msg->readStr(file.name);
                     msg->readUInt4(attrib_count);
                     for(uint4 j = 0; j < attrib_count; ++j)
                     {
                        msg->readUInt4(attrib_id);
                        msg->readUInt4(attrib_size);
                        switch(attrib_id)
                        {
                        case 1:
                           msg->readBool(file.attr_run_now);
                           file.attr_run_now_exists = true;
                           break;
                           
                        case 2:
                           msg->readBool(file.attr_run_on_power_up);
                           file.attr_run_on_power_up_exists = true;
                           break;
                           
                        case 3:
                           msg->readBool(file.attr_read_only);
                           file.attr_read_only_exists = true;
                           break;
                           
                        case 4:
                           msg->readUInt4(file_size);
                           file.attr_file_size = file_size;
                           file.attr_file_size_exists = true;
                           break;

                        case 5:
                           msg->readStr(file.attr_last_update);
                           file.attr_last_update_exists = true;
                           break;

                        case 6:
                           msg->readBool(file.attr_paused);
                           file.attr_paused_exists = true;
                           break;

                        case 7:
                           msg->readInt8(file.attr_file_size);
                           file.attr_file_size_exists = true;
                           break;
                           
                        default:
                           msg->movePast(attrib_size);
                           break;
                        }
                     }
                     event->files.push_back(file);
                  }
               }
               else
               {
                  // map the response code to an outcome
                  switch(resp_code)
                  {
                  case 2:
                     event->outcome = client_type::outcome_comm_disabled;
                     break;
                     
                  case 3:
                     event->outcome = client_type::outcome_comm_failed;
                     break;
                     
                  case 4:
                     event->outcome = client_type::outcome_blocked_by_logger;
                     break;
                  }
               }

               // post the event
               try { event->post(); }
               catch(Csi::Event::BadPost &) { } 
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage


      namespace FileListerHelpers
      {
         ////////////////////////////////////////////////////////////
         // class file_type definitions
         ////////////////////////////////////////////////////////////
         file_type::file_type():
            attr_run_now(false),
            attr_run_now_exists(false),
            attr_run_on_power_up(false),
            attr_run_on_power_up_exists(false),
            attr_read_only(false),
            attr_read_only_exists(false),
            attr_file_size(0),
            attr_file_size_exists(false),
            attr_last_update_exists(false),
            attr_paused_exists(false)
         { }


         file_type::file_type(file_type const &other):
            name(other.name),
            attr_run_now(other.attr_run_now),
            attr_run_now_exists(other.attr_run_now_exists),
            attr_run_on_power_up(other.attr_run_on_power_up),
            attr_run_on_power_up_exists(other.attr_run_on_power_up_exists),
            attr_read_only(other.attr_read_only),
            attr_read_only_exists(other.attr_read_only_exists),
            attr_file_size(other.attr_file_size),
            attr_file_size_exists(other.attr_file_size_exists), 
            attr_last_update_exists(other.attr_last_update_exists),
            attr_last_update(other.attr_last_update),
            attr_paused_exists(other.attr_paused_exists),
            attr_paused(other.attr_paused)
         { }
      }
   };
};
