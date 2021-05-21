/* Cora.Device.TablesEnumerator.h

   Copyright (C) 2000, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 21 July 2000
   Last Change: Friday 09 April 2021
   Last Commit: $Date: 2021-04-12 17:32:42 -0600 (Mon, 12 Apr 2021) $ 
   Committed by: $Author: jon $
*/


#ifndef Cora_Device_TablesEnumerator_h
#define Cora_Device_TablesEnumerator_h


#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include <list>


namespace Cora
{
   namespace Device
   {
      class TablesEnumerator;

      /**
       * Defines the interface that must be implemented by an application object in order to use the
       * TablesEnumerator component type.
       */
      class TablesEnumeratorClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called after the transaction has been started and  all tables in the initial set have
          * been reported.
          *
          * @param sender Specifies the component that is reporting this event.
          */
         virtual void on_started(TablesEnumerator *sender)
         { }

         /**
          * Called when a failure has ocurred that prevents the component from working.
          *
          * @param sender Specifies the component reporting this event.
          * @param failure Specifies a code that represents the type of failure.
          */
         enum failure_type
         {
            failure_unknown  = 0,
            failure_connection_failed = 1,
            failure_invalid_logon = 2,
            failure_server_security_blocked = 3,
            failure_device_name_invalid = 4,
         };
         virtual void on_failure(TablesEnumerator *sender, failure_type failure) = 0;

         /**
          * Called when a new table has been detected by the component
          *
          * @param sender Specifies the component reporting this event.
          * @param enabled Set to true if one or more collect areas for the table are enabled for
          * scheduled collection.
          * @param area_names Specifies the collection of collect areas that may write to the table.
          */
         typedef std::list<StrUni> area_names_type;
         virtual void on_table_added(
            TablesEnumerator *sender,
            StrUni const &table_name,
            bool enabled,
            area_names_type const &area_names)
         { }

         /**
          * Called when a previously reported table has been deleted.
          *
          * @param sender Specifies the component reporting this event.
          * @param table_name Specifies the name of the table that has been deleted.
          */
         virtual void on_table_deleted(
            TablesEnumerator *sender, StrUni const &table_name)
         { }

         /**
          * Called when one of the collect areas associated with this table has been enabled or
          * disabled for scheduled collection.
          *
          * @param sender Specifies the component reporting this event.
          * @param table_name Specifies the name of the table.
          * @param enabled Set to true if the table is enabled for scheduled collection.
          */
         virtual void on_table_enabled(
            TablesEnumerator *sender, StrUni const &table_name, bool enabled)
         { }
         
        /**
         * Called when the a collect area has been associated with the table.
         *
         * @param sender Specifies the component reporting this event.
         * @param area_name Specifies the name of the affected table.
         * @param area_names Specifies the current set of associated collect areas.
         */
         virtual void on_table_areas_changed(
            TablesEnumerator *sender,
            StrUni const &table_name,
            area_names_type const &area_names)
         { }
      };


      /**
       * Defines a component that can be used to monitor the set of data tables associated with a
       * datalogger event along with the collect area(s) associated with those tables.  In order to
       * use this component, the application must provide an object that implements the
       * TablesEnumeratorClient interface.  It should then create an instance if this class, set its
       * properties including device name, and call one of the two versions of start().  The
       * component will send an initial set of table names and then call the client's on_started()
       * method.  As tables are added or deleted after that, the component will notify the
       * application client.  If the transaction fails, the client will be notified through the
       * on_failure() notification.
       */
      class TablesEnumerator: public DeviceBase, public Csi::EvReceiver
      {
      private:
         /**
          * Specifis the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Specifies the application client object.
          */
      public:
         typedef TablesEnumeratorClient client_type;
      private:
         client_type *client;

         /**
          * Set to true if temporary tables should be reported.
          */
         bool send_temporaries;
         
      public:
         /**
          * Constructor
          */
         TablesEnumerator():
            state(state_standby),
            client(0),
            send_temporaries(false)
         { }

         /**
          * Destructor
          */
         virtual ~TablesEnumerator()
         { finish(); }

         /**
          * @return  Returns true if temporary tables should be reported.
          */
         bool get_send_temporaries() const
         { return send_temporaries; }

         /**
          * @param value Set to true if temporary tables should be reported.
          */
         void set_send_temporaries(bool value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            send_temporaries = value;
         }

         /**
          * Called to start the transaction with the server device.
          *
          * @param client_ Specifies the application's client object.
          * @param router Specifies a messaging router that has been created but not connected.
          * @param other_component Specifies another component that has a connection that can be
          * shared.
          */
         void start(
            client_type *client_, router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }
         void start(
            client_type *client_, ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(other_component);
         }
         
         /**
          * Called to release all resources for the server transaction.
          */
         virtual void finish() override
         {
            client = 0;
            state = state_standby;
            DeviceBase::finish();
         }

         /**
          * Overrides the base class version to handle asynch events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev) override;

         /**
          * Formats the specified failure code to the specified stream.
          */
         static void format_failure(std::ostream &out, client_type::failure_type failure);
         
         /**
          * Overrides the base class version to handle incoming messages.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg) override;

      protected:
         /**
          * Overrides the base class version to handle a failure report.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure) override;

         /**
          * Overrides the base class version to handle a session failure.
          */
         virtual void on_devicebase_session_failure() override
         { on_devicebase_failure(devicebase_failure_session); }

         /**
          * Overrides the base class version to handle a notification that the device session is
          * ready.
          */
         virtual void on_devicebase_ready() override;

      private:
         /**
          * Handles the start ack message.
          */
         void on_start_ack(Csi::Messaging::Message *message);

         /**
          * Handles the stopped notification.
          */
         void on_stopped_not(Csi::Messaging::Message *message);

         /**
          * handles the table added notification.
          */
         void on_table_added_not(Csi::Messaging::Message *message);

         /**
          * Handles the table deleted notification.
          */
         void on_table_deleted_not(Csi::Messaging::Message *message);

         /**
          * Handles the table enabled notification.
          */
         void on_table_enabled_not(Csi::Messaging::Message *message);

         /**
          * Handles the table areas changed notification.
          */
         void on_table_areas_not(Csi::Messaging::Message *message);
         
      };
   };
};

#endif
