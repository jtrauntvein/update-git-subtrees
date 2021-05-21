/* Cora.Device.TableResetter.h

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 06 January 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_TableResetter_h
#define Cora_Device_TableResetter_h


#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class TableResetter;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class TableResetterClient
      //
      // Defines the interface for a client object to a TableResetter.
      ////////////////////////////////////////////////////////////
      class TableResetterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_table_reset = 1,
            outcome_communication_failed = 2,
            outcome_communication_disabled = 3,
            outcome_invalid_table_name = 4,
            outcome_logger_security_blocked = 5,
            outcome_server_security_blocked = 6,
            outcome_invalid_device_name = 7,
            outcome_unsupported = 8,
            outcome_invalid_logon = 9,
            outcome_session_failed = 10,
         };
         virtual void on_complete(
            TableResetter *resetter,
            outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class TableResetter
      //
      // This class defines an object that will execute cora server Device Table Reset transaction.
      //
      // Use of this object involves creating an instance, invoking the appropriate set_xxx()
      // methods including set_device_name() and set_table_name() (these are required), and then
      // invoking the start() method. Once the transaction has completed, the on_complete() method
      // of the client pointer specified in start() will be invoked with an argument that relates
      // the outcome of the transaction.
      //
      // If this component is used with servers that support an interface version of 1.3.3.4 and
      // newer, set_table_name() does not need to be invoked and will default to an empty string.
      // Under these conditions, the component will reset all of the datalogger and server tables
      // associated with that datalogger. 
      ////////////////////////////////////////////////////////////
      class TableResetter:
         public DeviceBase,
         public Csi::EvReceiver
      {
      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // table_name
         //
         // Identifies the table that should be reset
         ////////////////////////////////////////////////////////////
         StrUni table_name;
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         TableResetter();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~TableResetter();

         ////////////////////////////////////////////////////////////
         // set_table_name
         ////////////////////////////////////////////////////////////
         void set_table_name(StrUni const &table_name);

         ////////////////////////////////////////////////////////////
         // get_table_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_table_name() const
         { return table_name; }

         ////////////////////////////////////////////////////////////
         // start
         //
         // Called to start the server transaction. Once the transaction has completed, the client
         // objects on_complete() method will be invoked to indicate the event.
         ////////////////////////////////////////////////////////////
         typedef TableResetterClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         //
         // Called to abort notification for the transaction. This will not abort the server
         // transaction once it has been started.
         ////////////////////////////////////////////////////////////
         void finish();

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
         virtual void on_devicebase_session_failure();

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // client
         //
         // The object that will receive the completion event when the transaction is complete
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
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
