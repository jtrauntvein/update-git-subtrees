/* Cora.Device.ProgramFileReceiver.h

   Copyright (C) 2000, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 7 September 2000
   Last Change: Friday 21 January 2011
   Last Commit: $Date: 2011-01-21 10:32:34 -0600 (Fri, 21 Jan 2011) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_ProgramFileReceiver_h
#define Cora_Device_ProgramFileReceiver_h
 
#include "Cora.Device.DeviceBase.h"
#include "Cora.Device.Defs.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class ProgramFileReceiver;
      //@endgroup
      
 
      ////////////////////////////////////////////////////////////
      // class ProgramFileReceiverClient
      ////////////////////////////////////////////////////////////
      class ProgramFileReceiverClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the server transaction has been completed
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 1,
            outcome_success = 0,
            outcome_communication_failure = 3,
            outcome_communication_disabled = 4, 
            outcome_logger_security_blocked = 5,
            outcome_invalid_server_logon = 6, 
            outcome_server_connection_failure  = 7,
            outcome_invalid_device_name = 8,
            outcome_cannot_open_file = 9,
            outcome_server_security_blocked = 10,
            outcome_not_supported = 11,
            outcome_cancelled = 12,
         };
         virtual void on_complete(
            ProgramFileReceiver *receiver,
            outcome_type outcome) = 0;

         ////////////////////////////////////////////////////////////
         // on_progress
         //
         // Called when a block of data has been received by the server
         ////////////////////////////////////////////////////////////
         virtual void on_progress(
            ProgramFileReceiver *receiver,
            uint4 receivedbytes)
         { }
      };



      ////////////////////////////////////////////////////////////
      // class ProgramFileReceiver
      //
      // Defines a component that allows an application to retrieve the currently running program in
      // a datalogger device into a specified file name.  In order to use this component, the
      // application must provide a client object derived from class ProgramFileReceiverClient
      // (typdefed as client_type in this class).  It must then create an instance of this class,
      // set appropriate properties like set_device_name() and set_out_file_name() and then invoke
      // start().  If the server transaction is started, the component will start receiving the
      // program file contents from the server and will write those contents into the file specified
      // when set_out_file_name() was called.  When the server transaction is complete, the client's
      // on_complete() method will be called and this component will return to a standby state.
      //
      // The application can abort the process at any time by deleting the component or by invoking
      // finish(). 
      ////////////////////////////////////////////////////////////
      class ProgramFileReceiver: public DeviceBase, public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ProgramFileReceiver();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ProgramFileReceiver();

         ////////////////////////////////////////////////////////////
         // set_out_file_name
         ////////////////////////////////////////////////////////////
         void set_out_file_name (StrAsc const &out_file_name_);

         ////////////////////////////////////////////////////////////
         // get_out_file_name
         ////////////////////////////////////////////////////////////
         StrAsc const &get_out_file_name() const
         { return out_file_name; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef ProgramFileReceiverClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // cancel
         ////////////////////////////////////////////////////////////
         virtual bool cancel(); 
         
         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // describe_outcome
         ////////////////////////////////////////////////////////////
         static void describe_outcome(
            std::ostream &out, client_type::outcome_type outcome);
         
      protected:
         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, 
            Csi::Messaging::Message *message);
         
         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_session_failure();

         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready();
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         ProgramFileReceiverClient *client;
         
         ////////////////////////////////////////////////////////////
         // out_file_name
         ////////////////////////////////////////////////////////////
         StrAsc out_file_name;

         ////////////////////////////////////////////////////////////
         // output
         ////////////////////////////////////////////////////////////
         FILE *output;
         
         ////////////////////////////////////////////////////////////
         // received_bytes
         ////////////////////////////////////////////////////////////
         uint4 received_bytes;

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
         // tran_no
         ////////////////////////////////////////////////////////////
         uint4 tran_no;
      };
   };
};

#endif
