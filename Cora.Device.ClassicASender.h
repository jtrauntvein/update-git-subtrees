/* Cora.Device.ClassicASender.h

   Copyright (C) 2008, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 25 January 2008
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_Device_ClassicASender_h
#define Cora_Device_ClassicASender_h

#include "Cora.Device.DeviceBase.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class ClassicASender;
      //@endgroup

      
      ////////////////////////////////////////////////////////////
      // class ClassicASenderClient
      ////////////////////////////////////////////////////////////
      class ClassicASenderClient: public Csi::InstanceValidator
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
            outcome_comm_disabled = 8
         };
         virtual void on_complete(
            ClassicASender *sender,
            outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class ClassicASender
      //
      // Defines a component that can be used to force the server to send an
      // "A" command to a classic logger to update the statistics associated
      // with that logger.  In order to use this component, the application
      // must provide an object derived from class ClassicASenderClient
      // (typedefed as client_type in the definition).  It should then create
      // an instance of this class, set the appropriate properties including
      // device name, and invoke one of the two start() methods.  When the
      // server transaction is complete, the application will be notified via
      // its client's on_complete() method. 
      ////////////////////////////////////////////////////////////
      class ClassicASender:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         ClassicASenderClient *client;

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
         // do_reset
         //
         // Specifies whether the error counters in the datalogger should be
         // reset. 
         ////////////////////////////////////////////////////////////
         bool do_reset;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ClassicASender():
            do_reset(false),
            client(0),
            state(state_standby)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ClassicASender()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // get_do_reset
         ////////////////////////////////////////////////////////////
         bool get_do_reset() const
         { return do_reset; }

         ////////////////////////////////////////////////////////////
         // set_do_reset
         ////////////////////////////////////////////////////////////
         void set_do_reset(bool do_reset_)
         {
            if(state == state_standby)
               do_reset = do_reset_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef ClassicASenderClient client_type;
         void start(
            client_type *client_, router_handle &router);
         void start(
            client_type *client_, ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // format_outcome
         ////////////////////////////////////////////////////////////
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

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
