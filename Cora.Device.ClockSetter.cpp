/* Cora.Device.ClockSetter.cpp

   Copyright (C) 2000, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 30 August 2000
   Last Change: Wednesday 10 March 2010
   Last Commit: $Date: 2010-03-10 16:34:23 -0600 (Wed, 10 Mar 2010) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.ClockSetter.h"
#include <assert.h>
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace ClockSetterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ClockSetterClient *client;
            ClockSetter *setter;
            typedef ClockSetterClient::outcome_type outcome_type;
            outcome_type outcome;
            Csi::LgrDate logger_time;
            int8 nsec_difference;
            static uint4 const event_id;

            static void create_and_post(
               ClockSetterClient *client,
               ClockSetter *setter,
               outcome_type outcome,
               int8 logger_time = 0,
               int8 nsec_difference = 0);

            void notify()
            { client->on_complete(setter,outcome,logger_time,nsec_difference); }

         private:
            event_complete(
               ClockSetterClient *client_,
               ClockSetter *setter_,
               outcome_type outcome_,
               Csi::LgrDate const &logger_time_,
               int8 nsec_difference_):
               Event(event_id,setter_),
               client(client_),
               setter(setter_),
               outcome(outcome_),
               logger_time(logger_time_),
               nsec_difference(nsec_difference_)
            { } 
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::ClockSetter::event_complete");


         void event_complete::create_and_post(
            ClockSetterClient *client,
            ClockSetter *setter,
            outcome_type outcome,
            int8 logger_time,
            int8 nsec_difference)
         {
            try
            { (new event_complete(client,setter,outcome,logger_time,nsec_difference))->post(); }
            catch(Csi::Event::BadPost &)
            { }
         } // create_and_post
      };


      ////////////////////////////////////////////////////////////
      // class ClockSetter definitions
      ////////////////////////////////////////////////////////////

      ClockSetter::ClockSetter():
         client(0),
         state(state_standby),
         should_set_clock(false),
         clock_transaction(0),
         send_server_time(false)
      { }

      
      ClockSetter::~ClockSetter()
      { finish(); }

      
      void ClockSetter::set_should_set_clock(bool should_set_clock_)
      {
         if(state == state_standby)
            should_set_clock = should_set_clock_;
         else
            throw exc_invalid_state();
      } // set_should_set_clock


      void ClockSetter::set_server_time(Csi::LgrDate const &server_time_)
      {
         if(state == state_standby)
         {
            server_time = server_time_;
            send_server_time = true;
         }
         else
            throw exc_invalid_state();
      } // set_server_time
      

      void ClockSetter::start(
         ClockSetterClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(ClockSetterClient::is_valid_instance(client_))
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


      void ClockSetter::start(
         ClockSetterClient *client_,
         ClientBase *other_client)
      {
         if(state == state_standby)
         {
            if(ClockSetterClient::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(other_client);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void ClockSetter::finish()
      {
         clock_transaction = 0;
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish


      bool ClockSetter::cancel()
      {
         // does the server version support cancel?
         bool rtn = false;
         if(interface_version >= Csi::VersionNumber("1.3.1"))
         {
            rtn = true;
            if(state == state_active)
            {
               // form the appropriate command to cancel the transaction
               Csi::Messaging::Message stop_command(
                  device_session,
                  should_set_clock ? Messages::clock_set_stop_cmd : Messages::clock_check_stop_cmd);
               stop_command.addUInt4(clock_transaction);
               router->sendMessage(&stop_command);
            }
            else if(state == state_delegate)
            {
               // we haven't started yet so we can finish here
               using namespace ClockSetterHelpers;
               
               state = state_standby;
               event_complete::create_and_post(client,this,client_type::outcome_cancelled);
            }
            else
               rtn = false;
         }
         return rtn;
      } // cancel


      void ClockSetter::format_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace ClockSetterStrings;
         switch(outcome)
         {
         case client_type::outcome_success_clock_checked:
            out << my_strings[strid_clock_checked];
            break;
            
         case client_type::outcome_success_clock_set:
            out << my_strings[strid_clock_set];
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
            
         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;

         case client_type::outcome_communication_failed:
            out << my_strings[strid_communication_failed];
            break;
            
         case client_type::outcome_communication_disabled:
            out << my_strings[strid_communication_disabled];
            break;
            
         case client_type::outcome_logger_security_blocked:
            out << my_strings[strid_logger_security_blocked];
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_outcome
      
      void ClockSetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace ClockSetterHelpers;
         event_complete *event = dynamic_cast<event_complete *>(ev.get_rep());
         assert(event != 0);
         finish();
         if(ClockSetterClient::is_valid_instance(event->client))
            event->notify();
      } // receive

      
      void ClockSetter::on_devicebase_ready()
      {
         state = state_active;
         if(should_set_clock)
         {
            Csi::Messaging::Message command(device_session,Messages::clock_set_cmd);
            clock_transaction = ++last_tran_no;
            command.addUInt4(clock_transaction);
            if(send_server_time)
               command.addInt8(server_time.get_nanoSec());
            router->sendMessage(&command);
         }
         else
         {
            Csi::Messaging::Message command(device_session,Messages::clock_check_cmd);
            clock_transaction = ++last_tran_no;
            command.addUInt4(clock_transaction);
            router->sendMessage(&command);
         }
      } // on_devicebase_ready

      
      void ClockSetter::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace ClockSetterHelpers;
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
         event_complete::create_and_post(client,this,outcome);
      } // on_devicebase_failure

      
      void ClockSetter::on_devicebase_session_failure()
      {
         using namespace ClockSetterHelpers;
         event_complete::create_and_post(client,this,client_type::outcome_session_failed);
      } // on_devicebase_session_failure

      
      void ClockSetter::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::clock_check_ack ||
               msg->getMsgType() == Messages::clock_set_ack)
            {
               // read the acknowledgement parameters
               uint4 tran_no;
               uint4 resp_code;
               int8 logger_time = 0;
               int8 time_difference = 0;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               if(resp_code == 1 || resp_code == 2)
               {
                  msg->readInt8(logger_time);
                  msg->readInt8(time_difference);
               }

               // map the server response code to a client response code
               using namespace ClockSetterHelpers;
               client_type::outcome_type outcome;
               switch(resp_code)
               {
               case 1:
                  outcome = client_type::outcome_success_clock_checked;
                  break;

               case 2:
                  outcome = client_type::outcome_success_clock_set;
                  break;

               case 3:
                  outcome = client_type::outcome_communication_failed;
                  break;

               case 4:
                  outcome = client_type::outcome_logger_security_blocked;
                  break;

               case 6:
                  outcome = client_type::outcome_communication_disabled;
                  break;

               case 7:
                  outcome = client_type::outcome_cancelled;
                  break;

               case 8:
                  outcome = client_type::outcome_device_busy;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::create_and_post(client,this,outcome,logger_time,time_difference);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage 
   };
};
