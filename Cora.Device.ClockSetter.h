/* Cora.Device.ClockSetter.h

   Copyright (C) 2000, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 30 August 2000
   Last Change: Wednesday 10 March 2010
   Last Commit: $Date: 2010-03-10 16:34:23 -0600 (Wed, 10 Mar 2010) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_Device_ClockSetter_h
#define Cora_Device_ClockSetter_h


#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"
#include "Csi.LgrDate.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class ClockSetter;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class ClockSetterClient
      //
      // Defines the interface that should be inmplemented by a client to a ClockSetter object.
      ////////////////////////////////////////////////////////////
      class ClockSetterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the clock check or set has been completed. The logger_time and
         // nanoseconds_difference parameters should be ignore unless the value of outcome is equal
         // to outcome_success_clock_checked or outcome_success_clock_set.
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success_clock_checked = 1,
            outcome_success_clock_set = 2,
            outcome_session_failed = 3,
            outcome_invalid_logon = 4,
            outcome_server_security_blocked = 5,
            outcome_communication_failed = 6,
            outcome_communication_disabled = 7,
            outcome_logger_security_blocked = 8,
            outcome_invalid_device_name = 9,
            outcome_unsupported = 10,
            outcome_cancelled = 11,
            outcome_device_busy = 12,
         };
         virtual void on_complete(
            ClockSetter *setter,
            outcome_type outcome,
            Csi::LgrDate const &logger_time,
            int8 nanoseconds_difference) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class ClockSetter
      //
      // Defines an object that has the capability of checking or setting device clocks. To use this
      // class, a client object must inherit from class ClockSetterClient. Once an object of this
      // class is created, the application should call set_xxx() methods such as set_device_name()
      // to set properties. It can then call start(). Once the clock set operation is finished, this
      // object will invoke the client's on_complete() method.
      ////////////////////////////////////////////////////////////
      class ClockSetter: public DeviceBase, public Csi::EvReceiver
      {
      private:
         //@group properties
         
         ////////////////////////////////////////////////////////////
         // should_set_clock
         //
         // Set to true if the server should attempt to set the datalogger clock rather than just
         // checking it. The default value for this property is false.
         ////////////////////////////////////////////////////////////
         bool should_set_clock;

         ////////////////////////////////////////////////////////////
         // send_server_time
         ////////////////////////////////////////////////////////////
         bool send_server_time;

         ////////////////////////////////////////////////////////////
         // server_time
         ////////////////////////////////////////////////////////////
         Csi::LgrDate server_time;
         
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ClockSetter();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ClockSetter();

         //@group property set methods
         
         ////////////////////////////////////////////////////////////
         // set_should_set_clock
         ////////////////////////////////////////////////////////////
         void set_should_set_clock(bool should_set_clock_);

         ////////////////////////////////////////////////////////////
         // set_server_time
         ////////////////////////////////////////////////////////////
         void set_server_time(Csi::LgrDate const &server_time_);
         
         //@endgroup

         ////////////////////////////////////////////////////////////
         // start
         //
         // Starts the clock check/set operation with the server. Once the operation is complete,
         // the client's on_complete() method will be invoked unless the client has called finish().
         ////////////////////////////////////////////////////////////
         typedef ClockSetterClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_client);
         
         ////////////////////////////////////////////////////////////
         // finish
         //
         // Invoked to cancel the client notification. Note that this will not cancel the clock
         // check operation with the server.
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // cancel
         ////////////////////////////////////////////////////////////
         virtual bool cancel();

         ////////////////////////////////////////////////////////////
         // format_outcome
         ////////////////////////////////////////////////////////////
         static void format_outcome(
            std::ostream &out, client_type::outcome_type outcome);

      protected:
         ////////// receive
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         //@group methods overloaded from class DeviceBase
         ////////// on_devicebase_ready
         virtual void on_devicebase_ready();
         
         ////////// on_devicebase_failure
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////// on_devicebase_session_failure
         virtual void on_devicebase_session_failure();

         ////////// onNetMessage
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);
         //@endgroup

      private:
         ////////// client
         ClockSetterClient *client;

         ////////// state
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // clock_transaction
         ////////////////////////////////////////////////////////////
         uint4 clock_transaction;
      };
   };
};

#endif
