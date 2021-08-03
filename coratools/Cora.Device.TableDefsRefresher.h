/* Cora.Device.TableDefsRefresher.h

   Copyright (C) 2001, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 18 January 2001
   Last Change: Friday 18 October 2019
   Last Commit: $Date: 2019-10-18 14:35:34 -0600 (Fri, 18 Oct 2019) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_TableDefsRefresher_h
#define Cora_Device_TableDefsRefresher_h

#include "Cora.Device.DeviceBase.h"


namespace Cora
{
   namespace Device
   {
      class TableDefsRefresher;


      /**
       * Defines the interface that must be implemented by the application in order to use the
       * TableDefsRefresher component.
       */
      class TableDefsRefresherClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the component when the LoggerNet transaction has been completed.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param outcome Specifies the outcome of the transaction.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_communication_failed = 2,
            outcome_communication_disabled = 3,
            outcome_invalid_table_name = 4,
            outcome_logger_security_blocked = 5,
            outcome_server_security_blocked = 6,
            outcome_invalid_device_name = 7,
            outcome_unsupported = 8,
            outcome_invalid_logon = 9,
            outcome_session_failed = 10,
            outcome_in_progress = 11,
            outcome_network_locked = 12,
         };
         virtual void on_complete(TableDefsRefresher *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines a component that can be used to force the LoggerNet server to update its version of
       * the datalogger table definitions.  In order to use this component, the application must
       * provide an object that extends class TableDefsRefresherClient.  It must then create and
       * instance of this class, set the device name property and, optionally, the action property,
       * and call one of the two versions of start().  When the transaction is complete, the
       * client's on_complete() method will be called.
       */
      class TableDefsRefresher:
         public DeviceBase,
         public Csi::EvReceiver
      {
      public:
         /**
          * Constructor
          */
         TableDefsRefresher();

         /**
          * Destructor
          */
         virtual ~TableDefsRefresher();

         /**
          * @param action Specifies whether the new table definitions should be merged (the state of
          * unchanged tables and collect areas would not change) or whether the table definitions
          * and collect area states should be reset.
          */
         enum action_type
         {
            action_merge = 1,
            action_reset = 2
         };
         void set_action(action_type action);

         /**
          * @return Returns the action that should be taken.
          */
         action_type get_action() const
         { return action; }

         /**
          * Called to start the connection to the server.
          *
          * @param client_ Specifies the client object.
          *
          * @param router Specifies a messagiung router that has not yet been connected.
          *
          * @param other_component Specifies a component that has a connection that this component
          * can share.
          */
         typedef TableDefsRefresherClient client_type;
         void start(TableDefsRefresherClient *client_, router_handle &router);
         void start(TableDefsRefresherClient *client_, ClientBase *other_component);

         /**
          * Overloads the base class to return this class to a standby state.
          */
         virtual void finish();

         /**
          * Formats the specified outcome code to the specified stream.
          *
          * @param out Specifies the output stream.
          *
          * @param outcome Specifies the outcome to format.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

         /**
          * Overloads the base class version to handle asynch events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
      protected:
         /**
          * Overloads the base class version to start the device transaction.
          */
         virtual void on_devicebase_ready();

         /**
          * Overloads the base class version to handle a failure report.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure);
         virtual void on_devicebase_session_failure()
         { on_devicebase_failure(devicebase_failure_session); }

         /**
          * Overloads the base class version to handle an incoming message.
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);

      private:
         /**
          * Specifies the application client object.
          */
         TableDefsRefresherClient *client;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
         } state;

         /**
          * Specifies the action to be taken.
          */
         action_type action;
      };
      
   };
};

#endif
