/* Cora.Device.VariableGetter.h

   Copyright (C) 2003, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 25 September 2003
   Last Change: Thursday 17 October 2019
   Last Commit: $Date: 2019-10-17 11:31:08 -0600 (Thu, 17 Oct 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_VariableGetter_h
#define Cora_Device_VariableGetter_h

#include <list>
#include <vector>
#include "Cora.Device.DeviceBase.h"
#include "Cora.Broker.Record.h"


namespace Cora
{
   namespace Device
   {
      class VariableGetter;


      /**
       * Defines the interface that the application must implement in order to use the
       * VariableGetter component.
       */
      class VariableGetterClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called after the server get varibale transaction has been completed.
          *
          * @param sender Specifies the component that is calling this method.
          *
          * @param outcome Specifies a code that describes the outcome.
          *
          * @param values Specifies a record that has a value (or aray of values).  This will only
          * be valid if the value of outcome is equal to outcome_succeeded.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_succeeded = 1,
            outcome_connection_failed = 2,
            outcome_invalid_logon = 3,
            outcome_server_security_blocked = 4,
            outcome_invalid_table_name = 5,
            outcome_invalid_column_name = 6,
            outcome_invalid_subscript = 7,
            outcome_communication_failed = 8,
            outcome_communication_disabled = 9,
            outcome_logger_security_blocked = 10,
            outcome_invalid_table_definition = 11,
            outcome_invalid_device_name = 12,
            outcome_unsupported = 13,
         };
         virtual void on_complete(
            VariableGetter *sender, outcome_type outcome, Csi::SharedPtr<Cora::Broker::Record> &values) = 0;
      };


      /**
       * Defines a component that allows the application to get a value as a scalar or as an array
       * directly from a datalogger table.   In order to use this component, the application must
       * provide an object that extends class VariableGetterClient.  It should then create an
       * instance of this class, set its properties including device name, table name, field name,
       * and field subscripts and must then call one of the two versions of start().  When the
       * LoggerNet transaction is complete, the component will call the client's on_complete()
       * method.
       */
      class VariableGetter:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the table to be polled.
          */
         StrUni table_name;

         /**
          * Specifies the field name.
          */
         StrUni column_name;

         /**
          * Specifies the field array subscripts.
          */
      public:
         typedef std::vector<uint4> array_address_type;
      private:
         array_address_type array_address;

         /**
          * Specifies the number of field values to be returned.
          */
         uint4 swath;

         /**
          * Specifies the factory that will be used to create values in the record.
          */
         Csi::SharedPtr<Cora::Broker::ValueFactory> value_factory;

         /**
          * Specifies the application client object.
          */
      public:
         typedef VariableGetterClient client_type;
      private:
         client_type *client;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;
         
      public:
         /**
          * Constructor.
          */
         VariableGetter():
            swath(1),
            client(0),
            state(state_standby)
         { value_factory.bind(new Cora::Broker::ValueFactory); }

         /**
          * Destructor
          */
         virtual ~VariableGetter()
         { finish(); }

         /**
          * @return Returns the table name.
          */
         StrUni const &get_table_name() const
         { return table_name; }

         /**
          * @param table_name_ Specifies the table name.
          */
         void set_table_name(StrUni const &table_name_)
         {
            if(state == state_standby)
               table_name = table_name_;
            else
               throw exc_invalid_state();
         }

         /**
          * @return Returns the column name property.
          */
         StrUni const &get_column_name() const
         { return column_name; }

         /**
          * @param column_name_ Specifies the column name property.
          */
         void set_column_name(StrUni const &column_name_)
         {
            if(state == state_standby)
               column_name = column_name_;
            else
               throw exc_invalid_state();
         }

         /**
          * @return Returns the starting array address property.
          */
         array_address_type get_array_address() const
         { return array_address; }

         /**
          * @param array_address_ Specifies the starting array address property.
          */
         void set_array_address(array_address_type const &array_address_)
         { 
            if(state == state_standby)
               array_address = array_address_;
            else
               throw exc_invalid_state();
         }

         /**
          * @return Returns the number of scalar values to retrieve starting at the array address.
          */
         uint4 get_swath() const
         { return swath; }

         /**
          * @param swath_ Specifies the number of values in the array starting at the array address
          * that will be expected.
          */
         void set_swath(uint4 swath_)
         {
            if(state == state_standby)
               swath = swath_;
            else
               throw exc_invalid_state();
         }

         /**
          * @return Returns the value factory object.
          */
         typedef Csi::SharedPtr<Cora::Broker::ValueFactory> value_factory_handle;
         value_factory_handle get_value_factory() const
         { return value_factory; }

         /**
          * @param value_factory_ Sets the object that will generate values for the return record.
          */
         void set_value_factory(value_factory_handle &value_factory_)
         {
            if(state == state_standby)
               value_factory = value_factory_;
            else
               throw exc_invalid_state();
         }

         /**
          * Called to start the connection to the server and start the loggernet transaction.
          *
          * @param client Specifies the application object that will be notified of the outcome.
          *
          * @param other_component Specifies a component that already has a LoggerNet connection
          * that can be shared with this component.
          *
          * @param router Specifies a newly created messaging router that has not yet been
          * connected.
          */
         void start(client_type *client, router_handle &router);
         void start(client_type *client, ClientBase *other_component);

         /**
          * Overloads the base class versio to release any local resources and to return this
          * component to standby state.
          */
         virtual void finish();

         /**
          * Overloads the base class version to handle asynch events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Formats the outcome code to the specified stream.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);
         
      protected:
         /**
          * Overloads the base class version to start the server transaction.
          */
         virtual void on_devicebase_ready();

         /**
          * Overloads the base class version to handle a failure.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         /**
          * Overloads the base class version to report a session failure.
          */
         virtual void on_devicebase_session_failure()
         {
            on_devicebase_failure(devicebase_failure_session);
         }

         /**
          * Overloads the base class version to handle an incoming message.
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);
      };
   };
}; 


#endif
