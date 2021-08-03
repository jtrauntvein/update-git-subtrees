/* Cora.Device.Rf95tTester.cpp

   Copyright (C) 2004, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 06 April 2004
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.Rf95tTester.h"


namespace Cora
{
   namespace Device
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
            // tester
            ////////////////////////////////////////////////////////////
            Rf95tTester *tester;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef Rf95tTester::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // prom_sig
            ////////////////////////////////////////////////////////////
            uint2 prom_sig;

            ////////////////////////////////////////////////////////////
            // records
            ////////////////////////////////////////////////////////////
            typedef client_type::records_type records_type;
            records_type records;

            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static void cpost(
               Rf95tTester *tester,
               client_type *client,
               outcome_type outcome,
               uint2 prom_sig = 0,
               records_type const &records = records_type())
            {
               try
               {
                  (new event_complete(
                     tester,
                     client,
                     outcome,
                     prom_sig,
                     records))->post();
               }
               catch(Csi::Event::BadPost &)
               { } 
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               Rf95tTester *tester_,
               client_type *client_,
               outcome_type outcome_,
               uint2 prom_sig_,
               records_type const &records_):
               Event(event_id,tester_),
               tester(tester_),
               client(client_),
               outcome(outcome_),
               prom_sig(prom_sig_),
               records(records_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::Rf95tTester::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class Rf95tTester definitions
      ////////////////////////////////////////////////////////////
      void Rf95tTester::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               if(!repeaters.empty())
               {
                  client = client_;
                  state = state_delegate;
                  DeviceBase::start(router);
               }
               else
                  throw std::invalid_argument("Empty repeaters list");
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void Rf95tTester::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               if(!repeaters.empty())
               {
                  client = client_;
                  state = state_delegate;
                  DeviceBase::start(other_component);
               }
               else
                  throw std::invalid_argument("Empty repeaters list");
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void Rf95tTester::finish()
      {
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish

      
      void Rf95tTester::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Messages::rf_quality_test_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt4((uint4)repeaters.size());
         for(uint4 i = 0; i < repeaters.size(); ++i)
            cmd.addByte(repeaters[i]);
         state = state_active;
         router->sendMessage(&cmd); 
      } // on_devicebase_ready

      
      void Rf95tTester::on_devicebase_failure(devicebase_failure_type failure)
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
         }
         event_complete::cpost(this,client,outcome);
      } // on_devicebase_failure

      
      void Rf95tTester::on_devicebase_session_failure()
      { event_complete::cpost(this,client,client_type::outcome_session_failed); }

      
      void Rf95tTester::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active && msg->getMsgType() == Messages::rf_quality_test_ack)
         {
            uint4 tran_no;
            uint4 resp_code;
            msg->readUInt4(tran_no);
            msg->readUInt4(resp_code);
            if(resp_code == 0)
            {
               uint2 prom_sig;
               uint4 record_count;
               client_type::record_type record;
               client_type::records_type records;

               msg->readUInt2(prom_sig);
               msg->readUInt4(record_count);
               records.reserve(record_count);
               for(uint4 i = 0; i < record_count; ++i)
               {
                  msg->readByte(record.test_packet_size);
                  msg->readByte(record.front2t);
                  msg->readByte(record.back2t);
                  msg->readByte(record.front1t);
                  msg->readByte(record.back1t);
                  records.push_back(record);
               }
               event_complete::cpost(
                  this,
                  client,
                  client_type::outcome_success,
                  prom_sig,
                  records);
            }
            else
            {
               client_type::outcome_type outcome;
               switch(resp_code)
               {
               case 1:
                  outcome = client_type::outcome_logger_security_blocked;
                  break;
                  
               case 2:
                  outcome = client_type::outcome_rf_link_failed;
                  break;
                  
               case 3:
                  outcome = client_type::outcome_in_progress;
                  break;
                  
               case 4:
                  outcome = client_type::outcome_comm_disabled;
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
      } // onNetMessage

      
      void Rf95tTester::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(state == state_active)
            {
               finish();
               if(client_type::is_valid_instance(event->client))
               {
                  event->client->on_complete(
                     this,
                     event->outcome,
                     event->prom_sig,
                     event->records);
               }
            }
         }
      } // receive
   };
};
