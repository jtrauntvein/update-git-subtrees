/* Cora.Device.ManualPoller.h

   Copyright (C) 2001, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 11 July 2001
   Last Change: Tuesday 29 September 2020
   Last Commit: $Date: 2020-10-02 10:01:41 -0600 (Fri, 02 Oct 2020) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_ManualPoller_h
#define Cora_Device_ManualPoller_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      class ManualPoller;

      /**
       * Defines the interface that the application object that uses the ManualPoller component will
       * need to implement.
       */
      class ManualPollerClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the component when a notification that some values have been collected.
          *
          * @param sender  Specifies the component sending this message.
          *
          * @param values_expected Specifies the number of values that are expected to come from all
          * collect areas.
          *
          * @param values_stored Specifies the number of values that have been collected from 
          * all collect areas.
          */
         virtual void on_status_notification(
            ManualPoller *sender, uint4 values_expected, uint4 values_stored)
         { }

         /**
          * Called when the manual poll transaction has been completed.
          *
          * @param sender Specifies the component sending this message.
          *
          * @param outcome Specifies a code that describes the outcome of the transaction.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_server_session_failed = 3,
            outcome_invalid_device_name = 4,
            outcome_unsupported = 5,
            outcome_server_security_blocked = 6,
            outcome_logger_security_blocked = 7,
            outcome_comm_failure = 8,
            outcome_communication_disabled = 9,
            outcome_table_defs_invalid = 10,
            outcome_aborted = 11,
            outcome_logger_locked = 12,
            outcome_file_io_failed = 13,
            outcome_no_table_defs = 14,
         };
         virtual void on_complete(ManualPoller *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines a component that can be used to conduct a manual poll operation on a station in the
       * LoggerNet server.  An application can use this compopnent by providing an object derived
       * from class ManualPollerClient and creating an instance of this class.  The application can
       * then set the component properties including device name (set_device_name()) and can then
       * call one of the two versions of start().  As data is collected from the datalogger, the
       * client object will be notified of status through calls to the client's
       * on_status_notification() method.  When the manual poll transaction is complete, the client
       * will be notified through a call to its on_complete() method.
       */
      class ManualPoller:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the application client.
          */
         ManualPollerClient *client;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;
         
         /**
          * Specifies the transaction number.
          */
         uint4 poll_transaction;

         /**
          * Set to true if the newest record should always be stored.
          */
         bool always_store_newest;
         
      public:
         /**
          * Default Constructor
          */
         ManualPoller():
            client(0),
            state(state_standby),
            poll_transaction(0),
            always_store_newest(false)
         { }

         /**
          * Destructor
          */
         virtual ~ManualPoller()
         { finish(); }

         /**
          * @return Returns true if the always_store_newest property is set.
          */
         bool get_always_store_newest() const
         { return always_store_newest; }

         /**
          * @param value Set to true if the always store newest flag should be passed to the server.
          */
         void set_always_store_newest(bool value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            always_store_newest = value;
         }

         /**
          * Called to start the manual poll transaction with the server.
          *
          * @param client_ Specifies the application object that will be notified of status and
          * completion.
          *
          * @param router Specifies a messaging router that has not yet been connected to any
          * instance of LoggerNet.
          *
          * @param other_component Specifies a separate component that already has a connection to
          * LoggerNet that this component can share.
          */
         typedef ManualPollerClient client_type;
         void start(client_type *client_, router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client ptr");
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }
         void start(client_type *client_, ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client ptr");
            client = client_;
            state = state_delegate;
            DeviceBase::start(other_component);
         }

         /**
          * Overloads the base class version to ensure that the internal state is reset.
          */
         virtual void finish() override
         {
            state = state_standby;
            client = 0;
            poll_transaction = 0;
            DeviceBase::finish();
         }

         /**
          * Overrides the base class version to send a stop manual poll command if the transaction
          * is still active.
          */
         virtual bool cancel() override;

         /**
          * Writes a string describing the specified outcome to the outcome stream.
          */
         static void format_outcome(
            std::ostream &out, client_type::outcome_type outcome);

         /**
          * Overrides the base class version to handle async events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Overrides the base class version to handle incoming session messages.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         /**
          * Overrides the base class version to handle the notification that the session is ready.
          */
         virtual void on_devicebase_ready() override;

         /**
          * Overrides the base class version to handle the notification that a failure has occurred.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure) override;

         /**
          * Overrides the base class version to handle a notification that a session has failed with
          * the device.
          */
         virtual void on_devicebase_session_failure() override
         { on_devicebase_failure(devicebase_failure_session); }
      };
   };
};


#endif
