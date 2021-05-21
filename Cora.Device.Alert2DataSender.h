/* Cora.Device.Alert2DataSender.h

   Copyright (C) 2016, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 14 July 2016
   Last Change: Thursday 14 July 2016
   Last Commit: $Date: 2016-07-19 17:14:28 -0600 (Tue, 19 Jul 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_Alert2DataSender_h
#define Cora_Device_Alert2DataSender_h
#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"
#include "Csi.InstanceValidator.h"


namespace Cora
{
   namespace Device
   {
      class Alert2DataSender;


      /**
       * Defines the application interface to the data sender component.  The application  must
       * provide an object that implements this interface.
       */
      class Alert2DataSenderClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when some of the data has been transmitted to the server.
          *
          * @param sender Specifies the component that is calling this message.
          *
          * @param bytes_sent Specifies the number of bytes that have been sent.
          *
          * @param total_size Specifies the total number of bytes that need to be sent. 
          */
         virtual void on_fragment_sent(
            Alert2DataSender *sender,
            int8 bytes_sent,
            int8 total_size)
         { }

         /**
          * Called when the send operation has been completed.
          *
          * @param sender Specifies the component that is invoking this method.
          *
          * @param outcome Specifies the outcome of the transaction.
          */
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_failure_logon = 2,
            outcome_failure_session = 3,
            outcome_failure_security = 4,
            outcome_failure_unsupported = 5,
            outcome_failure_invalid_device_name = 6,
            outcome_failure_source = 7
         };
         virtual void on_complete(Alert2DataSender *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines the base class for an object that will act as the source of ALERT2 data to send
       * using the Alert2DataSender component.
       */
      class Alert2DataSenderSourceBase
      {
      public:
         /**
          * Destructor
          */
         virtual ~Alert2DataSenderSourceBase()
         { }

         /**
          * Called to obtain the next fragment of data to transmit to the server transaction.
          *
          * @return Returns the number of bytes that were actually written. 
          *
          * @param sender Specifies the component making this call.
          *
          * @param buff Specifies the buffer to which the data should be written.
          *
          * @param buff_len Specifies the maximum number of bytes to read.
          */
         virtual uint4 get_next_fragment(
            Alert2DataSender *sender, void *buff, uint4 buff_len) = 0;

         /**
          * @return Returns the total number of bytes
          */
         virtual int8 get_total_size(Alert2DataSender *sender)  = 0;
      };


      /**
       * Defines a data source that reads its data from a file.
       */
      class Alert2DataSenderFileSource: public Alert2DataSenderSourceBase
      {
      public:
         /**
          * @param file_name Specifies the name of the file to read.
          */
         Alert2DataSenderFileSource(StrAsc const &file_name);

         /**
          * Destructor
          */
         virtual ~Alert2DataSenderFileSource();

         /**
          * Overloads the base class to read from the file.
          */
         virtual uint4 get_next_fragment(
            Alert2DataSender *sender, void *buff, uint4 buff_len);

         /**
          * Overloads the bse class version to return the file size.
          */
         virtual int8 get_total_size(Alert2DataSender *sender);

      private:
         /**
          * Specifies the file that is being read.
          */
         FILE *input;
      };


      /**
       * Defines a component that can be used to send data to an ALERT2 base device.  In order to
       * use this component, the application must provide an object that implements the
       * Alert2DataSenderClient interface and it must also provide an object that implements the
       * Alert2DataSenderSourceBase component.  It must then create an instance of this class, set
       * its properties including device_name and source, and call one of the two versions of
       * start.  As data is sent to the server, the client's on_fragment_sent() method will be
       * called.  When the transfer is complete, the client's on_complete() method will be called.
       */
      class Alert2DataSender: public DeviceBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the client for this component.
          */
         Alert2DataSenderClient *client;

         /**
          * Specifies the source for this component.
          */
         Csi::SharedPtr<Alert2DataSenderSourceBase> source;

         /**
          * Specifies the total number of bytes that have been sent.
          */
         int8 bytes_sent;

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
          * Used to buffer bytes that will eb sent.
          */
         byte tx_buff[1024];

         /**
          * Specifies the transaction number that will be used.
          */
         uint4 tran_no;

      public:
         /**
          * Constructor
          */
         Alert2DataSender():
            client(0),
            bytes_sent(0),
            state(state_standby)
         { }

         /**
          * Destructor
          */
         virtual ~Alert2DataSender()
         { finish(); }

         /**
          * @param source_ Specifies the source used for this transaction.
          */
         typedef Alert2DataSenderSourceBase source_type;
         typedef Csi::SharedPtr<source_type> source_handle;
         void set_source(source_handle source_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            source = source_;
         }

         /**
          * Overloads the base class version to release our own resources.
          */
         virtual void finish()
         {
            client = 0;
            source.clear();
            state = state_standby;
            bytes_sent = 0;
            DeviceBase::finish();
         }

         /**
          * Called to start the server transaction.
          *
          * @param client_ Specifies the application object that will receive notification on
          * completion.
          *
          * @param router Specifies a router for a newly established connection.
          *
          * @param other_component Specifies the component with which we should share the connection
          * and logon information.
          */
         typedef Alert2DataSenderClient client_type;
         void start(client_type *client_, router_handle router)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            if(state != state_standby)
               throw exc_invalid_state();
            if(source == 0)
               throw std::invalid_argument("source not specified");
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }
         void start(client_type *client_, ClientBase *other_component)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            if(state != state_standby)
               throw exc_invalid_state();
            if(source == 0)
               throw std::invalid_argument("source not specified");
            client = client_;
            state = state_delegate;
            DeviceBase::start(other_component);
         }

         /**
          * Overloads the base class version to start the send process.
          */
         virtual void on_devicebase_ready();

         /**
          * Overloads the base class version to handle any failure.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         /**
          * Handles the session failure.
          */
         virtual void on_devicebase_session_failure()
         { on_devicebase_failure(devicebase_failure_session); }

         /**
          * Overloads the message handler.
          */
         virtual void onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message);

         /**
          * Overloads the event handlers.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Formats the specified outcome code to the specified stream.
          */
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);
         
      private:
         /**
          * Sends the next fragment to the server.
          */
         void send_next_fragment();
      };
   };
};


#endif
