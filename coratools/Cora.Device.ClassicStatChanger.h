/* Cora.Device.ClassicStatChanger.h

   Copyright (C) 2006, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 17 November 2006
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_Device_ClassicStatChanger_h
#define Cora_Device_ClassicStatChanger_h

#include "Cora.Device.DeviceBase.h"
#include "StrAsc.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class ClassicStatChanger;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class ClassicStatChangerClient
      ////////////////////////////////////////////////////////////
      class ClassicStatChangerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_session_failure = 3,
            outcome_invalid_device_name = 4,
            outcome_blocked_by_server = 5,
            outcome_unsupported = 6,
            outcome_comm_failed = 7,
            outcome_comm_disabled = 8,
            outcome_blocked_by_logger = 9,
            outcome_no_response_after = 10,
            outcome_invalid_window = 11
         };
         virtual void on_complete(
            ClassicStatChanger *changer,
            outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class ClassicStatChanger
      ////////////////////////////////////////////////////////////
      class ClassicStatChanger:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         ClassicStatChangerClient *client;

         ////////////////////////////////////////////////////////////
         // star_mode
         ////////////////////////////////////////////////////////////
         char star_mode;

         ////////////////////////////////////////////////////////////
         // window
         ////////////////////////////////////////////////////////////
         uint4 window;

         ////////////////////////////////////////////////////////////
         // window_value
         ////////////////////////////////////////////////////////////
         StrAsc window_value;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ClassicStatChanger():
            state(state_standby),
            star_mode('A'),
            window(1),
            client(0)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ClassicStatChanger()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // set_star_mode
         ////////////////////////////////////////////////////////////
         void set_star_mode(char star_mode_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            star_mode = star_mode_;
         }

         ////////////////////////////////////////////////////////////
         // get_star_mode
         ////////////////////////////////////////////////////////////
         char get_star_mode() const
         { return star_mode; }

         ////////////////////////////////////////////////////////////
         // set_window
         ////////////////////////////////////////////////////////////
         void set_window(uint4 window_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            window = window_;
         }

         ////////////////////////////////////////////////////////////
         // get_window
         ////////////////////////////////////////////////////////////
         uint4 get_window() const
         { return window; }

         ////////////////////////////////////////////////////////////
         // set_window_value
         ////////////////////////////////////////////////////////////
         void set_window_value(StrAsc const &window_value_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            window_value = window_value_;
         }

         ////////////////////////////////////////////////////////////
         // get_window_value
         ////////////////////////////////////////////////////////////
         StrAsc const &get_window_value() const
         { return window_value; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef ClassicStatChangerClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();
         
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
      protected:
         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_session_failure()
         { on_devicebase_failure(devicebase_failure_session); }

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router,
            Csi::Messaging::Message *message); 
      };
   };
};


#endif
