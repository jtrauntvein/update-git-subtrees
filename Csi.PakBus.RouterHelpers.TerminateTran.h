/* Csi.PakBus.RouterHelpers.TerminateTran.h

   Copyright (C) 2005, 2008 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 27 October 2005
   Last Change: Thursday 18 December 2008
   Last Commit: $Date: 2008-12-18 15:56:00 -0600 (Thu, 18 Dec 2008) $ 
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_PakBus_RouterHelpers_TerminateTran_h
#define Csi_PakBus_RouterHelpers_TerminateTran_h

#include "Csi.PakBus.RouterHelpers.h"
#include "Csi.PakBus.PakBusTran.h"
#include "Csi.SharedPtr.h"


namespace Csi
{
   namespace PakBus
   {
      namespace RouterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class TerminateTran
         //
         // Defines a transaction object that will send an empty pakctrl
         // message to terminate the link associated with the detination
         // address. 
         ////////////////////////////////////////////////////////////
         class TerminateTran: public PakBusTran
         {
         private:
            ////////////////////////////////////////////////////////////
            // wait_for_focus_timer
            ////////////////////////////////////////////////////////////
            uint4 wait_for_focus_timer;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            TerminateTran(
               Router *router,
               SharedPtr<OneShot> &timer,
               uint2 destination_address):
               PakBusTran(
                  router,
                  timer,
                  Priorities::extra_high,
                  destination_address),
               wait_for_focus_timer(0)
            { }

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~TerminateTran()
            {
               if(wait_for_focus_timer)
                  timer->disarm(wait_for_focus_timer);
            }
            
            ////////////////////////////////////////////////////////////
            // start
            ////////////////////////////////////////////////////////////
            virtual void start()
            {
               wait_for_focus_timer = timer->arm(this,40000);
               request_focus(); 
            }

            ////////////////////////////////////////////////////////////
            // on_focus_start
            ////////////////////////////////////////////////////////////
            virtual void on_focus_start()
            {
               if(wait_for_focus_timer)
               {
                  timer->disarm(wait_for_focus_timer);
                  wait_for_focus_timer = 0;
               }
               if(router->route_port_is_active(get_destination_address()))
               {
                  message_handle message(new Message);
                  message->set_expected_response_interval(10000);
                  message->set_will_close(true);
                  send_message(message,ExpectMoreCodes::last);
                  pending_messages.clear();
               }
               post_close_event();
            }
            
            ////////////////////////////////////////////////////////////
            // on_failure
            ////////////////////////////////////////////////////////////
            virtual void on_failure(failure_type failure)
            {
               PakBusTran::on_failure(failure);
               post_close_event();
            }

            ////////////////////////////////////////////////////////////
            // onOneShotFired
            ////////////////////////////////////////////////////////////
            virtual void onOneShotFired(uint4 id)
            {
               if(id == wait_for_focus_timer)
               {
                  wait_for_focus_timer = 0;
                  post_close_event();
               }
               else
                  PakBusTran::onOneShotFired(id);
            }

            ////////////////////////////////////////////////////////////
            // get_transaction_description
            ////////////////////////////////////////////////////////////
            virtual void get_transaction_description(
               std::ostream &desc)
            {
               desc << "Terminate\",\"" << get_destination_address();
            }

            ////////////////////////////////////////////////////////////
            // will_terminate
            ////////////////////////////////////////////////////////////
            virtual bool will_terminate() const
            { return true; }

            ////////////////////////////////////////////////////////////
            // router_sponsored
            ////////////////////////////////////////////////////////////
            virtual bool router_sponsored() const
            { return true; }
         };
      };
   };
};


#endif
