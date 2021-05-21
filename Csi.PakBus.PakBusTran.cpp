/* Csi.PakBus.PakBusTran.cpp

   Copyright (C) 2002, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 18 March 2002
   Last Change: Thursday 29 November 2012
   Last Commit: $Date: 2012-11-30 10:08:16 -0600 (Fri, 30 Nov 2012) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.PakBusTran.h"
#include "Csi.PakBus.PakCtrlMessage.h"
#include "Csi.PakBus.Bmp5Message.h"
#include "Csi.PakBus.Router.h"
#include "Csi.Utils.h"
#include <algorithm>
#include <sstream>
#include "trace.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class PakBusTran definitions
      ////////////////////////////////////////////////////////////
      PakBusTran::PakBusTran(
         Router *router_,
         timer_handle &timer_,
         priority_type priority_,
         uint2 destination_address_):
         router(router_),
         timer(timer_),
         priority(priority_),
         destination_address(destination_address_),
         transaction_id(0),
         msec_time_out(0),
         round_trip_base(0),
         watch_dog_id(0),
         first_message_sent(0),
         messages_sent_count(0),
         started_session(false),
         closing_id(0),
         unroutable_id(0),
         was_preempted(false)
      { }


      PakBusTran::~PakBusTran()
      {
         if(watch_dog_id)
            timer->disarm(watch_dog_id);
         if(closing_id)
            timer->disarm(closing_id);
         if(unroutable_id)
            timer->disarm(unroutable_id);
      } // destructor


      void PakBusTran::on_failure(failure_type failure)
      {
         if(watch_dog_id != 0)
            timer->disarm(watch_dog_id);
         first_message_sent = false;
      } // on_failure

      
      void PakBusTran::on_sending_message(message_handle &message)
      {
         if(router)
         {
            // check the pending_messages queue to see if the message is present and waiting.
            pending_messages_type::iterator pmi = std::find(
               pending_messages.begin(),
               pending_messages.end(),
               message);
            if(pmi != pending_messages.end())
            {
               pending_messages.erase(pmi);
               
               // set the watch dog timer and set the base for the round trip
               round_trip_base = Csi::counter(0);
               message->set_expected_response_interval(get_time_out());
               if(!first_message_sent || watch_dog_id == 0)
               {
                  started_session = true;
                  first_message_sent = true;
                  if(msec_time_out > 0)
                  {
                     std::ostringstream log_message;
                     get_transaction_description(log_message);
                     watch_dog_id = timer->arm(this,get_time_out());
                     log_message << "\",\"" << get_time_out() << "\",\"" << watch_dog_id;
                     router->log_debug("arm transaction watchdog",log_message.str().c_str());
                     if(report_id >= 0)
                        router->set_report_timeout(report_id, get_time_out());
                  }
               }
               else if(watch_dog_id)
               {
                  std::ostringstream log_message;
                  get_transaction_description(log_message);
                  router->log_debug(
                     "reset transaction watchdog",
                     log_message.str().c_str());
                  timer->reset(watch_dog_id);
               }
            }
            else
            {
               std::ostringstream log_message;
               get_transaction_description(log_message);
               router->log_debug(
                  "message not queued for this transaction",
                  log_message.str().c_str());
            }
         }
      } // on_sending_message


      void PakBusTran::on_pakctrl_message(pakctrl_message_handle &message)
      {
         if(watch_dog_id)
            timer->reset(watch_dog_id);
      } // on_pakctrl_message


      void PakBusTran::on_bmp5_message(bmp5_message_handle &message)
      {
         if(watch_dog_id)
            timer->reset(watch_dog_id);
      } // on_bmp5_message
      
      
      void PakBusTran::send_bmp5_message(bmp5_message_handle message)
      {
         message->set_transaction_no(transaction_id);
         send_message(message.get_handle());
      } // send_bmp5_message
      
      
      void PakBusTran::send_pakctrl_message(pakctrl_message_handle message)
      {
         message->set_transaction_no(transaction_id);
         send_message(message.get_handle());
      } // send_pakctrl_message
      
      
      void PakBusTran::send_message(
         message_handle message,
         ExpectMoreCodes::expect_more_code_type expect_more_code)
      {
         if(router)
         {
            message->set_priority(priority);
            message->set_destination(destination_address);
            message->set_expect_more(expect_more_code);
            pending_messages.push_back(message);
            if(!router->send_message_from_app(message))
               unroutable_id = timer->arm(this,50);
         }
      } // send_message
      
      
      uint4 PakBusTran::get_round_trip_time() const
      { return Csi::counter(round_trip_base); }
      
      
      void PakBusTran::set_time_out(uint4 msec_time_out_)
      {
         // cancel the current watch dog
         if(watch_dog_id != 0)
         {
            timer->disarm(watch_dog_id);
            watch_dog_id = 0;
         }

         // re-arm if needed
         msec_time_out = msec_time_out_;
         if(first_message_sent && msec_time_out > 0)
         {
            watch_dog_id = timer->arm(this,get_time_out());
         }
      } // set_time_out

      
      void PakBusTran::reset_time_out()
      {
         if(watch_dog_id != 0)
            timer->reset(watch_dog_id);
      } // reset_time_out

      
      void PakBusTran::clear_time_out()
      {
         if(watch_dog_id != 0)
         {
            first_message_sent = false;
            timer->disarm(watch_dog_id);
            watch_dog_id = 0; 
         }
      } // clear_time_out

      
      void PakBusTran::on_close()
      {
         if(router)
         {
            // if there are any unsent messages, we should remove them from the router
            while(!pending_messages.empty())
            {
               router->cancel_message_from_app(pending_messages.front());
               pending_messages.pop_front();
            }
         }
      } // on_close

      
      void PakBusTran::on_new_transaction_id(byte transaction_id_)
      {
         transaction_id = transaction_id_;
         clear_time_out();
      } // on_new_transaction_id


      void PakBusTran::request_focus()
      {
         if(router)
            router->request_transaction_focus(destination_address,transaction_id);
      } // request_focus


      void PakBusTran::release_focus()
      {
         if(router)
         {
            if(watch_dog_id)
            {
               std::ostringstream log_message;
               get_transaction_description(log_message);
               log_message << "\",\"" << watch_dog_id;
               router->log_debug("PakBusTran release focus",log_message.str().c_str());
               timer->disarm(watch_dog_id);
               watch_dog_id = 0;
            }
            router->release_transaction_focus(destination_address,transaction_id);
         }
      } // release_focus


      uint4 PakBusTran::get_time_out()
      {
         uint4 rtn = 0;
         if(msec_time_out && router)
         {
            uint4 route_time = router->get_route_response_time(destination_address); 
            rtn = msec_time_out + route_time + route_time/4;
            if(rtn > 35000 &&
               router->route_port_should_cap_timeout(destination_address))
               rtn = 35000;
         }
         return rtn;
      } // get_time_out
      

      void PakBusTran::on_router_close()
      { router = 0; }


      bool PakBusTran::is_still_valid()
      {
         bool rtn = true;
         if(first_message_sent && closing_id == 0 && watch_dog_id == 0)
            rtn = false;
         return rtn;
      } // is_still_valid
      
      
      void PakBusTran::onOneShotFired(uint4 event_id)
      {
         if(router != 0)
         {
            std::ostringstream desc;
            
            get_transaction_description(desc);
            desc << "\",\""
                 << static_cast<uint2>(get_transaction_id());
            if(event_id == watch_dog_id)
            {
               router->log_debug("PakBusTran timeout",desc.str().c_str());
               first_message_sent = false;
               watch_dog_id = 0;
               on_failure(PakCtrl::DeliveryFailure::timed_out_or_resource_error);
            }
            else if(event_id == unroutable_id)
            {
               router->log_debug("PakBusTran unroutable",desc.str().c_str());
               unroutable_id = 0;
               on_failure(PakCtrl::DeliveryFailure::unreachable_destination);
            }
            else if(event_id == closing_id)
            {
               router->log_debug("PakBusTran closing",desc.str().c_str());
               router->close_transaction(
                  destination_address,
                  transaction_id);
            }
         }
      } // onOneShotFired


      void PakBusTran::post_close_event()
      {
         if(closing_id == 0)
            closing_id = timer->arm(this,10);
         if(watch_dog_id)
         {
            timer->disarm(watch_dog_id);
            watch_dog_id = 0;
         }
      } // post_close_event
   };
};

