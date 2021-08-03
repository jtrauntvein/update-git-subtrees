/* Cora.Device.ProtocolIdentifier.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 24 July 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_ProtocolIdentifier_h
#define Cora_Device_ProtocolIdentifier_h

#include "Cora.Device.DeviceBase.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class ProtocolIdentifier;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class ProtocolIdentifierClient
      ////////////////////////////////////////////////////////////
      class ProtocolIdentifierClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the server transaction has been completed.  This method must be overloaded
         // by the application in order to receive completion notification.
         //
         // The protocol field should be ignore unless the value of outcome is
         // outcome_protocol_identified or outcome_protocol_and_model_no_identified.  The value of
         // the model_no should be ignored unless the value of outcome os
         // outcome_protocol_and_model_no_identified. 
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_protocol_identified = 1,
            outcome_protocol_and_model_no_identified = 2,
            outcome_no_protocol_identified = 3,
            outcome_communication_disabled = 4,
            outcome_invalid_logon = 5,
            outcome_session_failed = 6,
            outcome_unsupported = 7,
            outcome_server_security_blocked = 8,
            outcome_invalid_device_name = 9,
            outcome_link_failed = 10
         };
         enum protocol_type
         {
            protocol_unknown = 0,
            protocol_classic = 1,
            protocol_bmp1 = 2,
            protocol_bmp3 = 3,
            protocol_pakbus = 4,
         };
         typedef DevTypeCode model_no_type;
         virtual void on_complete(
            ProtocolIdentifier *identifier,
            outcome_type outcome,
            protocol_type protocol,
            model_no_type model_no,
            uint2 pakbus_address) = 0;

         ////////////////////////////////////////////////////////////
         // on_status_notification
         //
         // Called when a status notification has been received from the server transaction.  The
         // application can ignore this notification by not overloading this method. 
         ////////////////////////////////////////////////////////////
         enum status_type
         {
            status_looking_for_command_prompt = 1,
            status_looking_for_bmp1_response = 2,
            status_looking_for_pakbus_response = 3,
         };
         virtual void on_status_notification(
            ProtocolIdentifier *identifier,
            status_type status)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class ProtocolIdentifier
      //
      // Defines a LoggerNet client component that uses the Device Identify Datalogger Protocol
      // transaction to attempt to determine the type of logger associated as a child to the device
      // named by set_device_name().
      //
      // In order to use this component, an application must provide an object that is derived from
      // class ProtocolIdentifierClient (also known as ProtocolIdentifier::client_type).  It must
      // then create an instance of this class (or a subclass of the same) and set the appropriate
      // properties (including set_device_name()) before calling one of the two versions of
      // start().  While the loggernet transaction is working, the client object's
      // on_status_notification() method will be called as the state of the server transaction
      // changes.  When the server transaction has been completed, the client's on_complete() method
      // will be called.  The application can abort these notifications AND the server transaction
      // with the device by either calling finish() or by deleting the component.
      ////////////////////////////////////////////////////////////
      class ProtocolIdentifier:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         ProtocolIdentifierClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
         } state;

         ////////////////////////////////////////////////////////////
         // max_baud_rate
         ////////////////////////////////////////////////////////////
         uint4 max_baud_rate;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ProtocolIdentifier():
            client(0),
            state(state_standby),
            max_baud_rate(9600)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ProtocolIdentifier()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // set_max_baud_rate
         ////////////////////////////////////////////////////////////
         void set_max_baud_rate(uint4 max_baud_rate_)
         {
            if(state == state_standby)
               max_baud_rate = max_baud_rate_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_max_baud_rate
         ////////////////////////////////////////////////////////////
         uint4 get_max_baud_rate() const
         { return max_baud_rate; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef ProtocolIdentifierClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

      protected:
         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(
            devicebase_failure_type failure);

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
      };
   };
};


#endif
