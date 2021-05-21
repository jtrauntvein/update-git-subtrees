/* Cora.Device.ClassicStatsDumper.h

   Copyright (C) 2006, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 16 November 2006
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_Device_ClassicStatsDumper_h
#define Cora_Device_ClassicStatsDumper_h

#include "Cora.Device.DeviceBase.h"
#include "StrAsc.h"
#include <list>


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class ClassicStatsDumper;
      //@endgroup

      
      class ClassicStatsDumperClient: public Csi::InstanceValidator
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
            outcome_blocked_by_logger = 9
         };
         typedef std::list<StrAsc> results_type;
         virtual void on_complete(
            ClassicStatsDumper *dumper,
            outcome_type outcome,
            results_type const &results) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class ClassicStatsDumper
      ////////////////////////////////////////////////////////////
      class ClassicStatsDumper:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         ClassicStatsDumperClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // star_mode
         ////////////////////////////////////////////////////////////
         byte star_mode;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ClassicStatsDumper():
            state(state_standby),
            client(0),
            star_mode('A')
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ClassicStatsDumper()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // set_star_mode
         ////////////////////////////////////////////////////////////
         void set_star_mode(byte star_mode_)
         {
            if(state == state_standby)
               star_mode = star_mode_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_star_mode
         ////////////////////////////////////////////////////////////
         byte get_star_mode() const
         { return star_mode; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef ClassicStatsDumperClient client_type;
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
