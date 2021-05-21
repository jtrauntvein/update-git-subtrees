/* Cora.Device.VariableSetter.h

   Copyright (C) 2000, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 07 June 2000
   Last Change: Thursday 26 February 2015
   Last Commit: $Date: 2015-02-26 10:46:26 -0600 (Thu, 26 Feb 2015) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_VariableSetter_h
#define Cora_Device_VariableSetter_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "CsiEvents.h"
#include <vector>
#include <locale>


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class VariableSetter;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // VariableSettingClient
      //
      // Defines the interface expected from a client to the VariableSetter class
      ////////////////////////////////////////////////////////////
      class VariableSetterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_succeeded = 1,
            outcome_connection_failed = 2,
            outcome_invalid_logon = 3,
            outcome_server_security_blocked = 4,
            outcome_column_read_only = 5,
            outcome_invalid_table_name = 6,
            outcome_invalid_column_name = 7,
            outcome_invalid_subscript = 8,
            outcome_invalid_data_type = 9,
            outcome_communication_failed = 10,
            outcome_communication_disabled = 11,
            outcome_logger_security_blocked = 12,
            outcome_unmatched_logger_table_definition = 13,
            outcome_invalid_device_name = 14
         };
         virtual void on_complete(VariableSetter *tran, outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class VariableSetter
      //
      // Defines a class that can set device variables on a datalogger in the LgrNet network.
      //
      // In order to use this class, an application should derive a custom class from class
      // VariableSetterClient. It should create an instance of this class and fill in the various
      // properties (including the logon name, password, and device name). It should then call
      // start() passing as a parameter to it a valid messaging router and a valid pointer to class
      // VariableSetterClient. After the transaction has completed its work, it will invoke the
      // client objects on_complete() method with a pointer to itself and an enumeration parameter
      // that describes the outcome.
      ////////////////////////////////////////////////////////////
      class VariableSetter: public DeviceBase, public Csi::EvReceiver
      {
      public:
         typedef std::vector<uint4> index_type;

      private: 
         //@group properties
         ////////////////////////////////////////////////////////////
         // table_name
         //
         // The name of the table that contains the variable to be set 
         ////////////////////////////////////////////////////////////
         StrUni table_name;

         ////////////////////////////////////////////////////////////
         // column_name
         //
         // The name of the column that contains the variable to be set 
         //////////////////////////////////////////////////////////// 
         StrUni column_name;

         ////////////////////////////////////////////////////////////
         // index
         //
         // The array address of the scalar within the column. This value defaults to an empty list
         // (assumes that the column is a scalar). If the column is not a scalar, this value must
         // have the same length as the number of dimensions in the array. This property can be set
         // using the set_index(index_type) or set_index(string) methods.
         //////////////////////////////////////////////////////////// 
         index_type index;

         ////////////////////////////////////////////////////////////
         // value_type
         //
         // Records the data_type of the value that should be sent. This value is set indirectly by
         // the client by calling the set_value_bool or set_value_float methods.
         //////////////////////////////////////////////////////////// 
         enum value_type_type
         {
            value_type_unset,
            value_type_bool,
            value_type_float,
            value_type_uint4,
            value_type_int4,
            value_type_uint2,
            value_type_int2,
            value_type_uint1,
            value_type_int1,
            value_type_string,   // will convert just before sending
            value_type_double
         } value_type;

         ////////////////////////////////////////////////////////////
         // value_variant
         //
         // Stores the current value set by one of the set_value_xxx() methods.
         ////////////////////////////////////////////////////////////
         union value_variant_type
         {
            bool v_bool;
            float v_float;
            uint4 v_uint4;
            int4 v_int4;
            uint2 v_uint2;
            int2 v_int2;
            byte v_uint1;
            char v_int1;
            double v_double;
         } value_variant;
         
         ////////////////////////////////////////////////////////////
         // value_string
         //
         // Stores the value set by set_value_string()
         //////////////////////////////////////////////////////////// 
         StrAsc value_string;

         ////////////////////////////////////////////////////////////
         // locale
         ////////////////////////////////////////////////////////////
         std::locale locale;
         
         //@endgroup
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         VariableSetter();
         
         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~VariableSetter();
         
         //@group properties setup methods
         ////////////////////////////////////////////////////////////
         // set_table_name
         //////////////////////////////////////////////////////////// 
         void set_table_name(StrUni const &table_name);
         
         ////////////////////////////////////////////////////////////
         // set_column_name
         //////////////////////////////////////////////////////////// 
         void set_column_name(StrUni const &column_name);
         
         ////////////////////////////////////////////////////////////
         // set_index(vector)
         //////////////////////////////////////////////////////////// 
         void set_index(index_type const &index_);
         
         ////////////////////////////////////////////////////////////
         // set_index(string)
         //
         // Sets the index by reading the string as a space (one or more) delimited list of unsigned
         // integers
         //////////////////////////////////////////////////////////// 
         void set_index(char const *index_string);
         void set_index(wchar_t const *index_string);
         
         ////////////////////////////////////////////////////////////
         // set_value_bool
         //
         // Sets the value to be sent to the server and sets the type property so that the
         // value_bool value will be sent to the server.
         //////////////////////////////////////////////////////////// 
         void set_value_bool(bool value_bool_);

         ////////////////////////////////////////////////////////////
         // set_value_double
         //
         // Sets the value to be sent to the server and sets the value_type
         // property so that the value_double value will be sent to the
         // server. 
         ////////////////////////////////////////////////////////////
         void set_value_double(double value_double_);
         
         ////////////////////////////////////////////////////////////
         // set_value_float
         //
         // Sets the value to be sent to the server and sets the value_type property so that the
         // value_float value will be sent to the server.
         //////////////////////////////////////////////////////////// 
         void set_value_float(float value_float_);

         ////////////////////////////////////////////////////////////
         // set_value_uint4
         ////////////////////////////////////////////////////////////
         void set_value_uint4(uint4 value);

         ////////////////////////////////////////////////////////////
         // set_value_int4
         ////////////////////////////////////////////////////////////
         void set_value_int4(int4 value);

         ////////////////////////////////////////////////////////////
         // set_value_uint2
         ////////////////////////////////////////////////////////////
         void set_value_uint2(uint2 value);

         ////////////////////////////////////////////////////////////
         // set_value_int2
         ////////////////////////////////////////////////////////////
         void set_value_int2(int2 value);

         ////////////////////////////////////////////////////////////
         // set_value_uint1
         ////////////////////////////////////////////////////////////
         void set_value_uint1(byte value);

         ////////////////////////////////////////////////////////////
         // set_value_int1
         ////////////////////////////////////////////////////////////
         void set_value_int1(char value);
         
         ////////////////////////////////////////////////////////////
         // set_value_string
         //
         // Sets the value to be sent as a string. Will perform the conversion just before the value
         // is sent to the server
         //////////////////////////////////////////////////////////// 
         void set_value_string(char const *value_string_);
         void set_value_string(wchar_t const *value_string_);

         ////////////////////////////////////////////////////////////
         // set_locale
         ////////////////////////////////////////////////////////////
         void set_locale(std::locale const &locale);
         
         //@endgroup

         //@group property access methods
         ////////////////////////////////////////////////////////////
         // get_table_name
         //////////////////////////////////////////////////////////// 
         StrUni const &get_table_name() const { return table_name; }

         ////////////////////////////////////////////////////////////
         // get_column_name
         //////////////////////////////////////////////////////////// 
         StrUni const &get_column_name() const { return column_name; }

         ////////////////////////////////////////////////////////////
         // get_index
         //////////////////////////////////////////////////////////// 
         index_type const &get_index() const { return index; }

         ////////////////////////////////////////////////////////////
         // get_locale
         ////////////////////////////////////////////////////////////
         std::locale const &get_locale() const
         { return locale; }
         
         //@endgroup
         
         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef VariableSetterClient client_type;
         virtual void start(
            VariableSetterClient *client,
            router_handle &router);
         virtual void start(
            VariableSetterClient *client,
            ClientBase *other_component);
         
         ////////////////////////////////////////////////////////////
         // finish
         //////////////////////////////////////////////////////////// 
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // describe_outcome
         ////////////////////////////////////////////////////////////
         static void describe_outcome(
            std::ostream &out, client_type::outcome_type outcome);
         
      protected:
         //@group methods overloaded from class DeviceBase
         virtual void on_devicebase_ready();
         
         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_devicebase_session_failure();

         ////////////////////////////////////////////////////////////
         // onNetMessage
         //////////////////////////////////////////////////////////// 
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // onNetSesBroken
         //////////////////////////////////////////////////////////// 
         virtual void onNetSesBroken(
            Csi::Messaging::Router *rtr,
            uint4 session_no,
            uint4 reason,
            char const *why);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         //////////////////////////////////////////////////////////// 
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // on_complete
         //////////////////////////////////////////////////////////// 
         void on_complete(VariableSetterClient::outcome_type outcome);

         ////////////////////////////////////////////////////////////
         // on_open_broker_ses_ack
         //////////////////////////////////////////////////////////// 
         void on_open_broker_ses_ack(Csi::Messaging::Message *msg); 

         ////////////////////////////////////////////////////////////
         // on_table_def_get_ack
         //////////////////////////////////////////////////////////// 
         void on_table_def_get_ack(Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_set_variable_ack
         //////////////////////////////////////////////////////////// 
         void on_set_variable_ack(Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // write_value
         //
         // Performs the work of writing the variable value but checks first or performs any
         // required conversions.
         //////////////////////////////////////////////////////////// 
         void write_value(
            uint4 data_type,
            Csi::Messaging::Message &out);

      private:
         ////////////////////////////////////////////////////////////
         // state
         //////////////////////////////////////////////////////////// 
         enum state
         {
            state_standby,      // ready to set settings
            state_delegate,     // NetNode events should be delegated to base class
            state_local         // NetNode events should be processed locally
         } state;

         ////////////////////////////////////////////////////////////
         // client
         //
         // The client that will receive event notification when the task is complete
         //////////////////////////////////////////////////////////// 
         VariableSetterClient *client;

         ////////////////////////////////////////////////////////////
         // broker_session
         //////////////////////////////////////////////////////////// 
         uint4 broker_session;
      };
   };
};

#endif
