/* Cora.Device.ProgramStatsGetter.cpp

   Copyright (C) 2005, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 29 December 2005
   Last Change: Saturday 09 February 2019
   Last Commit: $Date: 2019-02-11 11:37:35 -0600 (Mon, 11 Feb 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.ProgramStatsGetter.h"
#include "coratools.strings.h"
#include "Csi.StrAscStream.h"
#include "boost/format.hpp"


namespace Cora
{
   namespace Device
   {
      namespace
      {
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;

            typedef ProgramStatsGetter getter_type;
            typedef getter_type::client_type client_type;
            client_type *client;

            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            typedef client_type::program_stats_type program_stats_type;
            program_stats_type program_stats;

            event_complete(
               getter_type *getter,
               client_type *client_,
               outcome_type outcome_,
               program_stats_type const &program_stats_):
               Event(event_id,getter),
               client(client_),
               outcome(outcome_),
               program_stats(program_stats_)
            { }
            
            static void cpost(
               getter_type *getter,
               client_type *client,
               outcome_type outcome,
               program_stats_type const &program_stats = program_stats_type())
            {
               event_complete *event = new event_complete(
                  getter,
                  client,
                  outcome,
                  program_stats);
               event->post();
            }
         };


         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::Device::ProgramStatsGetter::event_complete");
      };
      

      void ProgramStatsGetterClient::program_stats_type::format_compile_state(std::ostream &out) const
      {
         using namespace ProgramStatsGetterStrings;
         switch(compile_state)
         {
         case 0:
            out << my_strings[strid_no_program_running];
            break;

         case 1:
            out << my_strings[strid_program_running];
            break;

         case 2:
            out << my_strings[strid_program_compile_error];
            break;

         case 3:
            out << my_strings[strid_program_stopped];
            break;
         }
      }


      void ProgramStatsGetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            finish();
            if(client_type::is_valid_instance(event->client))
               event->client->on_complete(this, event->outcome, event->program_stats);
         }
      } // receive


      void ProgramStatsGetter::format_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
            
         case client_type::outcome_success:
            out << common_strings[common_success];
            break;
            
         case client_type::outcome_session_failed:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
                     
         case client_type::outcome_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_server_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
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
            
         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
         }
      } // format_outcome


      void ProgramStatsGetter::format_program_stats(
         std::ostream &out, ProgramStatsGetterHelpers::program_stats_type const &stats)
      {
         using namespace ProgramStatsGetterStrings;
         Csi::OStrAscStream program_state;
         Csi::OStrAscStream compile_time;
         stats.format_compile_state(program_state);
         stats.compile_time.format(compile_time, "%c");
         out << boost::format(my_strings[strid_program_status].c_str()) %
            stats.program_name %
            stats.power_up_prog %
            program_state.str() %
            compile_time.str() %
            stats.os_version %
            stats.compile_result;
      } // format_program_stats
         
      
      void ProgramStatsGetter::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Messages::get_program_stats_cmd);
         cmd.addUInt4(++last_tran_no);
         if(interface_version >= Csi::VersionNumber("1.3.9.4"))
            cmd.addBool(use_cached);
         else
            use_cached = false;
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      
      void ProgramStatsGetter::on_devicebase_failure(
         devicebase_failure_type failure)
      {
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
         event_complete::cpost(this,client,outcome);
      } // on_devicebase_failure

      
      void ProgramStatsGetter::on_devicebase_session_failure()
      {
         event_complete::cpost(this,client,client_type::outcome_session_failed);
      } // on_devicebase_session_failure

      
      void ProgramStatsGetter::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::get_program_stats_ack)
            {
               uint4 tran_no;
               uint4 response;
               client_type::outcome_type outcome;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(response);
               switch(response)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;
                  
               case 2:
                  outcome = client_type::outcome_communication_failed;
                  break;
                  
               case 3:
                  outcome = client_type::outcome_logger_security_blocked;
                  break;
                  
               case 5:
                  outcome = client_type::outcome_communication_disabled;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               client_type::program_stats_type stats;
               if(outcome == client_type::outcome_success)
               {
                  int8 nsec;
                  msg->readStr(stats.os_version);
                  msg->readUInt2(stats.os_sig);
                  msg->readStr(stats.serial_no);
                  msg->readStr(stats.power_up_prog);
                  msg->readUInt4(stats.compile_state);
                  msg->readStr(stats.program_name);
                  msg->readUInt2(stats.program_sig);
                  msg->readInt8(nsec); stats.compile_time = nsec;
                  msg->readStr(stats.compile_result);
                  if(msg->whatsLeft() > 0)
                     msg->readStr(stats.station_name);
               }
               event_complete::cpost(this, client, outcome, stats);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage
   };
};


