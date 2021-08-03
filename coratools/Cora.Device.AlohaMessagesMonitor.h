/* Cora.Device.AlohaMessagesMonitor.h

   Copyright (C) 2021, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 02 February 2021
   Last Change: Tuesday 02 February 2021
   Last Commit: $Date: 2021-02-02 16:29:36 -0600 (Tue, 02 Feb 2021) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_AlohaMessagesMonitor_h
#define Cora_Device_AlohaMessagesMonitor_h
#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"
#include <deque>


namespace Cora
{
   namespace Device
   {
      class AlohaMessagesMonitor;

      
      /**
       * Defines the interface that must be implemented by an application object in order to use the
       * AlohaMessagesMonitor component type.
       */
      class AlohaMessagesMonitorClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called to report a failure of the transaction.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param failure Specifies the type of failure.
          */
         enum failure_type
         {
            failure_unknown = 0,
            failure_logon = 1,
            failure_session = 2,
            failure_invalid_device_name = 3,
            failure_unsupported = 4,
            failure_security = 5,
            failure_shut_down = 6
         };
         virtual void on_failure(AlohaMessagesMonitor *sender, failure_type failure) = 0;

         /**
          * Called when the monitor transaction has been started with the server.
          *
          * @param sender Specifies the component calling this method.
          */
         virtual void on_started(AlohaMessagesMonitor *sender)
         { }

         /**
          * Called when one or more messages have been received from the server.
          *
          * @return Return true if the messages should be acknowledged automaticaly.  Returns false
          * if the spplication will later call send_ack() to tell the server to continue to send
          * messages.  Unless the application has some form of asynchronous message processing, the
          * return value should always be true.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param messages Specfies the collection of messages that are available.
          */
         typedef std::pair<Csi::LgrDate, StrAsc> message_type;
         typedef std::deque<message_type> messages_type;
         virtual bool on_messages(AlohaMessagesMonitor *sender, messages_type const &messages) = 0;
      };


      /**
       * Defines a component that can be used to monitor messages received by an AlohaReceiver
       * device in LoggerNet.  In order to use this component, the application must provice an
       * object that implements the AlohaMessageMonitorClient interface.  It should then create an
       * instance of this class, set its device name, and call one of the two versions of start().
       * Once the transaction is started, the component will call the client's on_messages() method
       * when there are messages available from the server.  If the client returns true from this
       * call, the component will immediately send an acknowledgement to the server which will allow
       * the server to send any other messages.  If the client returns false from this call, no
       * acknowledgement will be sent until the application calls this components send_ack()
       * method.  If the transction fails at any time, the client's on_failure() method will be
       * called.
       */
      class AlohaMessagesMonitor: public DeviceBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the client for this component.
          */
         AlohaMessagesMonitorClient *client;

         /**
          * Specifies the collection of messages that have been received.
          */
         typedef AlohaMessagesMonitorClient::messages_type messages_type;
         messages_type messages;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
            state_ack_wait
         } state;

         /**
          * Specifies the transaction number.
          */
         uint4 tran_no;

      public:
         /**
          * Constructor
          */
         AlohaMessagesMonitor():
            client(0),
            state(state_standby),
            tran_no(0)
         { }

         /**
          * Destructor
          */
         virtual ~AlohaMessagesMonitor()
         { finish(); }

         /**
          * Starts this component's connection to loggernet.
          *
          * @param client_ Specifies the client for this component.
          *
          * @param router Specifies a messaging router that has not been connected to the server.
          *
          * @param other_client Specifies an already connected component that can share its server
          * connection.
          */
         typedef AlohaMessagesMonitorClient client_type;
         void start(client_type *client_, router_handle &router)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            if(state != state_standby)
               throw exc_invalid_state();
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }
         void start(client_type *client_, ClientBase *other_client)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            if(state != state_standby)
               throw exc_invalid_state();
            client = client_;
            state = state_delegate;
            DeviceBase::start(other_client);
         }

         /**
          * Releases all resources held by this component and returns it to a statndby state.
          */
         virtual void finish()
         {
            client = 0;
            state = state_standby;
            messages.clear();
            tran_no = 0;
            DeviceBase::finish();
         }

         /**
          * Overrides the base class version to handle asynch events
          */
         virtual void receive(event_handle &ev) override;
         
         /**
          * Must be called by the client after an on_messages() call that returned a value of false.
          */
         void send_ack();

         /**
          * Formats a description of the given failure code to the specified stream.
          */
         static void describe_failure(std::ostream &out, client_type::failure_type failure);

      protected:
         /**
          * Overrides the base class version to actively start the transaction.
          */
         virtual void on_devicebase_ready() override;

         /**
          * Overrides the base class version to handle a failure report.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure) override;

         /**
          * Overrides the base class version to handle a server message.
          */
         virtual void onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message) override;
      };
   };
};


#endif
