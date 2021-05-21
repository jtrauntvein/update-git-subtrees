/* Cora.Device.AlohaMessagesLogQuery.h

   Copyright (C) 2021, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Saturday 06 February 2021
   Last Change: Wednesday 10 February 2021
   Last Commit: $Date: 2021-02-11 09:18:20 -0600 (Thu, 11 Feb 2021) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_AlohaMessagesLogQuery_h
#define Cora_Device_AlohaMessagesLogQuery_h
#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"
#include <deque>
#include <limits>
#ifdef max
#undef max
#undef min
#endif


namespace Cora
{
   namespace Device
   {
      class AlohaMessagesLogQuery;


      /**
       * Defines the interface that must be implemented by an application object that uses the
       * AlohaMessagesLogQuery component type.
       */
      class AlohaMessagesLogQueryClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the server transaction has been started.
          *
          * @param sender Specifies the component that called this method.
          */
         virtual void on_started(AlohaMessagesLogQuery *sender)
         { }

         /**
          * Called when one or messages have been received from the server for the application to
          * process.
          *
          * @return Returns true if the acknowledgement should be sent immediately to the server.
          * If false is returned, the application must call send_ack() manually when it is ready for
          * more.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param messages Specifies the messages that have been received.
          */
         typedef std::pair<Csi::LgrDate, StrAsc> message_type;
         typedef std::deque<message_type> messages_type;
         virtual bool on_messages(AlohaMessagesLogQuery *sender, messages_type const &messages) = 0;

         /**
          * Called when the transaction has been completed
          *
          * @param sender Specifies the component reporting the failure.
          *
          * @param outcome Specifies a reason why the transaction has completed.
          */
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_failure_session = 2,
            outcome_failure_logon = 3,
            outcome_failure_invalid_device_name = 4,
            outcome_failure_unsupported = 5,
            outcome_failure_security = 6,
            outcome_failure_logging_disabled = 7,
            outcome_failure_invalid_time_range = 8,
            outcome_failure_invalid_predicate = 9,
            outcome_failure_aborted = 10
         };
         virtual void on_complete(AlohaMessagesLogQuery *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines a component type that can be used to query the logs created by an AlohaReceiver
       * device type in LoggerNet.  In order to use this component, the application must provide an
       * object that extends class AlohaMessagesLogQueryClient.  It should then create an instance
       * of this component, set its parameters including device name, begin_stamp, end_stamp, and
       * predicate, and then call one of the two versions of start().  When the server transaction
       * has been started, the client's on_started() method will be called.  Thereafter, the
       * component will call the client's on_messages() method when messages have been received from
       * the server.  If the client immediately returns true to that method, the component will
       * acknowledge those messages to the server immediately after on_messages() has returned.
       * Otherwise, the application must call send_ack() when it is ready to handle more messages.
       * This will continue until the server transaction has been completed or an error occurs.  At
       * this point, the component will call the client's on_complete() method.
       */
      class AlohaMessagesLogQuery:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the client for this component.
          */
         AlohaMessagesLogQueryClient *client;

         /**
          * Specifies the current transaction number.
          */
         uint4 tran_no;

         /**
          * Specifies the last sequence number.
          */
         uint4 last_sequence_no;

         /**
          * Specifies the messages waiting to be processed by the application.
          */
         typedef AlohaMessagesLogQueryClient::messages_type messages_type;
         messages_type messages;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
            state_needs_ack
         } state;

         /**
          * Specifies the begin time stamp for the range of records.
          */
         Csi::LgrDate begin_stamp;

         /**
          * Specifies the end time stamp for the range of records.
          */
         Csi::LgrDate end_stamp;

         /**
          * Specifies the predicate expression.
          */
         StrAsc predicate;

      public:
         /**
          * Constructor
          */
         AlohaMessagesLogQuery():
            client(0),
            tran_no(0),
            state(state_standby),
            last_sequence_no(0),
            end_stamp(std::numeric_limits<int8>::max())
         { }

         /**
          * Destructor
          */
         virtual ~AlohaMessagesLogQuery()
         { finish(); }

         /**
          * @return the begin stamp.
          */
         Csi::LgrDate const &get_begin_stamp() const
         { return begin_stamp; }

         /**
          * @param value Specifies the bgin stamp.
          */
         void set_begin_stamp(Csi::LgrDate const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            begin_stamp = value;
         }

         /**
          * @return Returns the end time stamp.
          */
         Csi::LgrDate const &get_end_stamp() const
         { return end_stamp; }

         /**
          * @param value Specifies the end stamp.
          */
         void set_end_stamp(Csi::LgrDate const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            end_stamp = value;
         }

         /**
          * @return Returns the predicate expression
          */
         StrAsc const &get_predicate() const
         { return predicate; }

         /**
          * @param value Specifies the predicate expression.
          */
         void set_predicate(StrAsc const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            predicate = value;
         }

         /**
          * @return Returns the last sequence number read from the last notificn.
          */
         uint4 get_last_sequence_no() const
         { return last_sequence_no; }

         /**
          * Called to start the connection to the server and the server transaction.
          *
          * @param client_ Specifies the application object that will receive messages from this
          * component.
          *
          * @param router Specifies a router that has been created but not connected to the server.
          *
          * @param other_client Specifies a component that has been connected and that can share its
          * connection with this component.
          */
         typedef AlohaMessagesLogQueryClient client_type;
         void start(client_type *client_, router_handle router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }
         void start(client_type *client_, ClientBase *other_client)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(other_client);
         }

         /**
          * Overrides the base class version to release the transaction and any resources.
          */
         virtual void finish() override
         {
            state = state_standby;
            client = 0;
            messages.clear();
            DeviceBase::finish();
         }

         /**
          * Formats the outcome code to the specified stream.
          *
          * @param out Specifies the stream.
          *
          * @param outcome Specifies the outcome code.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

         /**
          * Overrides the base class version to handle asynch events.
          */
         virtual void receive(event_handle &ev) override;

         /**
          * Called by the application to acknowledge the last batch of messages.
          * @param sequence_no Optionally specifies the sequence number that will be sent in the ack
          * to the server.  If set to a value of 0xffffffff (the default), the last sequence number
          * that was received from the server will be sent.
          */
         void send_ack(uint4 sequence_no = 0xffffffff);

      protected:
         /**
          * Overrides the base class version to handle the ready notification.
          */
         virtual void on_devicebase_ready() override;

         /**
          * Overridez the base class version to handle the failure notification.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure) override;

         /**
          * Overrides the base class version to handle inbound messages.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message) override;
      };
   };
};


#endif
