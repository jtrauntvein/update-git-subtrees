/* Cora.Device.CollectScheduleResetter.h

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 09 January 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_Device_CollectScheduleResetter_h
#define Cora_Device_CollectScheduleResetter_h


#include "Cora.Device.DeviceBase.h"
#include "CsiEvents.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class CollectScheduleResetter;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class CollectScheduleResetterClient
      ////////////////////////////////////////////////////////////
      class CollectScheduleResetterClient: public Csi::InstanceValidator
      {
      public:
         ////////// on_complete
         // Called when the schedule resetter object has finished.
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_reset = 1,
            outcome_reset_but_disabled = 2,
            outcome_server_security_blocked = 3,
            outcome_invalid_device_name = 4,
            outcome_unsupported = 5,
            outcome_invalid_logon = 6,
            outcome_session_failed = 7,
         };
         virtual void on_complete(
            CollectScheduleResetter *resetter,
            outcome_type outcome) = 0; 
      };


      ////////////////////////////////////////////////////////////
      // class CollectScheduleResetter
      //
      // Defines an object that can reset the collection schedule on a specified datalogger
      // device. After creating an instance of this class, an application should invoke 
      // set_device_name() and other property setting methods if appropriate. It should then invoke
      // start() in order to start the process. Once the server transaction is complete, the
      // application's client object's on_complete() method will be invoked.
      ////////////////////////////////////////////////////////////
      class CollectScheduleResetter:
         public DeviceBase,
         public Csi::EvReceiver
      {
      private:
         //@group properties
         ////////// start_now
         // Set to true if collection should start immediately after the schedule has been reset.
         bool start_now;
         //@endgroup

      public:
         ////////// constructor
         CollectScheduleResetter();

         ////////// destructor
         virtual ~CollectScheduleResetter();

         ////////// set_start_now
         void set_start_now(bool start_now_);

         ////////// start
         void start(
            CollectScheduleResetterClient *client_,
            router_handle &router);
         void start(
            CollectScheduleResetterClient *client_,
            ClientBase *other_component);

         ////////// finish
         void finish();

      protected:
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

      private:
         ////////// receive
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////// client
         typedef CollectScheduleResetterClient client_type;
         client_type *client;

         ////////// state
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;
      };
   };
};

#endif
