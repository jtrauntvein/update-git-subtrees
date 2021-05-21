/* Csi.PakBus.PakBusTran.h

   Copyright (C) 2002, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 18 March 2002
   Last Change: Friday 01 October 2010
   Last Commit: $Date: 2012-11-30 10:08:16 -0600 (Fri, 30 Nov 2012) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_PakBusTran_h
#define Csi_PakBus_PakBusTran_h

#include "Csi.PolySharedPtr.h"
#include "Csi.PakBus.Defs.h"
#include "OneShot.h"
#include <list>
#include <iostream>


namespace Csi
{
   namespace PakBus
   {
      //@group class forward declarations
      class Message;
      class PakCtrlMessage;
      class Bmp5Message;
      class Router;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class PakBusTran
      //
      // Defines an object that maintains the state of a transaction between
      // two PakBus devices. This state includes timing concerns as well as
      // tracking the the identifier for the remote node.  The local node is
      // always assumed to be the node identified by the router (specified in
      // the constructor) as well as the transaction identifier.
      //
      // This class defines virtual methods that define the events that a
      // transaction object can receive from the router including a failure
      // event, notifications that BMP5 or PakCtrl packets have been received,
      // and notification when a PakBus message is about to be sent).  It also
      // defines methods that allow BMP5 or PakCtrl messages to be sent. These
      // methods will fill in the transaction number, priority, destination
      // address, and whatever other fields are appropriate for the
      // transaction.
      ////////////////////////////////////////////////////////////
      class PakBusTran:
         public OneShotClient
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         typedef Priorities::priority_type priority_type;
         typedef Csi::SharedPtr<OneShot> timer_handle;
         PakBusTran(
            Router *router_,
            timer_handle &timer_,
            priority_type priority_,
            uint2 destination_address_);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~PakBusTran();
            
         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called when a failure has occurred in relation to delivering a
         // message associated with this transaction.
         ////////////////////////////////////////////////////////////
         typedef PakCtrl::DeliveryFailure::failure_type failure_type;
         virtual void on_failure(failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_sending_message
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Message> message_handle;
         virtual void on_sending_message(message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_pakctrl_message
         ////////////////////////////////////////////////////////////
         typedef Csi::PolySharedPtr<Message, PakCtrlMessage> pakctrl_message_handle;
         virtual void on_pakctrl_message(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_bmp5_message
         ////////////////////////////////////////////////////////////
         typedef Csi::PolySharedPtr<Message, Bmp5Message> bmp5_message_handle;
         virtual void on_bmp5_message(bmp5_message_handle &message);

         ////////////////////////////////////////////////////////////
         // send_bmp5_message
         ////////////////////////////////////////////////////////////
         void send_bmp5_message(bmp5_message_handle message);

         ////////////////////////////////////////////////////////////
         // send_pakctrl_message
         ////////////////////////////////////////////////////////////
         void send_pakctrl_message(pakctrl_message_handle message);

         ////////////////////////////////////////////////////////////
         // send_message
         ////////////////////////////////////////////////////////////
         void send_message(
            message_handle message,
            ExpectMoreCodes::expect_more_code_type = ExpectMoreCodes::expect_more);

         ////////////////////////////////////////////////////////////
         // get_round_trip_time
         //
         // Returns the time that has elapsed since the last packet was sent on
         // this transaction.
         ////////////////////////////////////////////////////////////
         uint4 get_round_trip_time() const;

         ////////////////////////////////////////////////////////////
         // set_time_out
         //
         // Sets the timeout parameter associated with this transaction.  If
         // the first message on this transaction has already been sent, this
         // method will start the timer for the transaction.  Otherwise, the
         // timer will not be set until the first message is sent.  No timer
         // will ever be set if the specified timeout has a value of zero.
         ////////////////////////////////////////////////////////////
         virtual void set_time_out(uint4 msec_time_out_);

         ////////////////////////////////////////////////////////////
         // reset_time_out
         //
         // Causes the watch dog timer for this transaction to be reset (or
         // set).
         ////////////////////////////////////////////////////////////
         void reset_time_out();

         ////////////////////////////////////////////////////////////
         // clear_time_out
         ////////////////////////////////////////////////////////////
         virtual void clear_time_out();

         ////////////////////////////////////////////////////////////
         // on_close
         //
         // Called when this transaction has been closed
         ////////////////////////////////////////////////////////////
         virtual void on_close();

         ////////////////////////////////////////////////////////////
         // on_new_transaction_id
         //
         // Called when the transaction number associated with this transaction
         // should be changed.  This will also clear the timer and asociated
         // flags for the transaction.
         ////////////////////////////////////////////////////////////
         void on_new_transaction_id(byte transaction_id_);

         ////////////////////////////////////////////////////////////
         // get_transaction_id
         ////////////////////////////////////////////////////////////
         byte get_transaction_id() const { return transaction_id; }

         ////////////////////////////////////////////////////////////
         // get_destination_address
         ////////////////////////////////////////////////////////////
         uint2 get_destination_address() const { return destination_address; }

         ////////////////////////////////////////////////////////////
         // get_started_session
         //
         // Returns true if this transaction sent a message that would have
         // started a session
         ////////////////////////////////////////////////////////////
         bool get_started_session() const { return started_session; }

         ////////////////////////////////////////////////////////////
         // get_priority
         ////////////////////////////////////////////////////////////
         priority_type get_priority() const { return priority; }

         ////////////////////////////////////////////////////////////
         // start
         //
         // Can be optionally overloaded to start the initial messages.  This
         // mechanism is used extensively in the transactions intended for
         // router support.
         ////////////////////////////////////////////////////////////
         virtual void start() { }

         ////////////////////////////////////////////////////////////
         // is_same
         //
         // Returns truw if this transaction is identified by the destination
         // and transaction id
         ////////////////////////////////////////////////////////////
         bool is_same(uint2 destination, byte transaction_id) const
         { return destination == destination_address && transaction_id == this->transaction_id; }

         ////////////////////////////////////////////////////////////
         // on_focus_start
         //
         // Called when this transaction has gained the focus from the router.
         ////////////////////////////////////////////////////////////
         virtual void on_focus_start() { }

         ////////////////////////////////////////////////////////////
         // request_focus
         ////////////////////////////////////////////////////////////
         virtual void request_focus();

         ////////////////////////////////////////////////////////////
         // release_focus
         ////////////////////////////////////////////////////////////
         void release_focus();

         ////////////////////////////////////////////////////////////
         // get_first_message_sent
         ////////////////////////////////////////////////////////////
         bool get_first_message_sent() const
         { return first_message_sent; }

         ////////////////////////////////////////////////////////////
         // get_messages_sent
         ////////////////////////////////////////////////////////////
         uint4 get_messages_sent() const
         { return messages_sent_count; }

         ////////////////////////////////////////////////////////////
         // get_time_out
         //
         // Calculates the time associated with the route and the assigned
         // transaction timeout.
         ////////////////////////////////////////////////////////////
         uint4 get_time_out();

         ////////////////////////////////////////////////////////////
         // get_transaction_description
         //
         // Should be overloaded to fill in a string that describes the
         // transaction intent.
         ////////////////////////////////////////////////////////////
         virtual void get_transaction_description(std::ostream &desc) = 0;

         ////////////////////////////////////////////////////////////
         // on_application_finished
         //
         // Called when the application is done with this transaction.  This
         // default version will call post_close_event() to prepare the
         // transaction to be deleted.
         ////////////////////////////////////////////////////////////
         virtual void on_application_finished()
         {
            release_focus();
            post_close_event();
         }

         ////////////////////////////////////////////////////////////
         // on_router_close
         ////////////////////////////////////////////////////////////
         virtual void on_router_close();

         ////////////////////////////////////////////////////////////
         // is_closing
         ////////////////////////////////////////////////////////////
         bool is_closing() const
         { return closing_id != 0; }

         ////////////////////////////////////////////////////////////
         // is_still_valid
         //
         // This method will be invoked by the router in its maintenance cycle
         // on the transaction that has current focus.  If its return value is
         // false, the router will consider the transaction a failure and
         // report it as such while ending the transaction focus.
         //
         // This base version will validate that the timer is set if the
         // transaction is in a state where it is awaiting the device
         // response.  This base version should generally be incorporated into
         // overloaded methods as a check.  
         ////////////////////////////////////////////////////////////
         virtual bool is_still_valid();

         ////////////////////////////////////////////////////////////
         // will_terminate
         //
         // Should be overloaded in derived classes that are designed to
         // terminate the link to return true.
         ////////////////////////////////////////////////////////////
         virtual bool will_terminate() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // is_special_hello
         ////////////////////////////////////////////////////////////
         virtual bool is_special_hello(PakBusTran *for_other) const
         { return false; }

         ////////////////////////////////////////////////////////////
         // router_sponsored
         ////////////////////////////////////////////////////////////
         virtual bool router_sponsored() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // get_was_preempted
         ////////////////////////////////////////////////////////////
         bool get_was_preempted() const
         { return was_preempted; }

         ////////////////////////////////////////////////////////////
         // set_was_preempted
         ////////////////////////////////////////////////////////////
         void set_was_preempted()
         { was_preempted = true; }

         ////////////////////////////////////////////////////////////
         // get_report_id
         ////////////////////////////////////////////////////////////
         int8 get_report_id() const
         { return report_id; }

      protected:
         ////////////////////////////////////////////////////////////
         // onOneShotFired
         ////////////////////////////////////////////////////////////
         virtual void onOneShotFired(uint4 event_id);

         ////////////////////////////////////////////////////////////
         // post_close_event
         //
         // Prepares this transaction to be closed.
         ////////////////////////////////////////////////////////////
         virtual void post_close_event();
         
      protected:
         ////////////////////////////////////////////////////////////
         // router
         ////////////////////////////////////////////////////////////
         Router *router;

         ////////////////////////////////////////////////////////////
         // timer
         ////////////////////////////////////////////////////////////
         timer_handle timer;
         
         ////////////////////////////////////////////////////////////
         // destination_address
         //
         // Specifies the address of the device that this transaction is with.
         // This value will be assigned to the destination value for all
         // messages sent through this transaction and will be used to filter
         // the source addresses of all incoming messages.
         ////////////////////////////////////////////////////////////
         uint2 destination_address;

         ////////////////////////////////////////////////////////////
         // transaction_id
         //
         // Specifies the transaction number that will be assigned to all
         // messages sent through this transaction.  Also specifies which
         // transaction number will allow this transaction to receive messages.
         ////////////////////////////////////////////////////////////
         byte transaction_id;

         ////////////////////////////////////////////////////////////
         // priority
         //
         // Identifies the priority that should be assigned to all messages
         // sent through this transaction.
         ////////////////////////////////////////////////////////////
         priority_type priority;

         ////////////////////////////////////////////////////////////
         // round_trip_base
         //
         // Keeps track of the time when the first message was sent for this
         // transaction.  This value is used to calculate the round-trip time
         // for the transaction.
         ////////////////////////////////////////////////////////////
         uint4 round_trip_base;

         ////////////////////////////////////////////////////////////
         // msec_time_out
         //
         // Stores the timeout used to set the watch dog timer for the
         // transaction when the first message has been sent.
         ////////////////////////////////////////////////////////////
         uint4 msec_time_out;

         ////////////////////////////////////////////////////////////
         // watch_dog_id
         //
         // Identifies the timer that gets started after the first message is
         // sent.
         ////////////////////////////////////////////////////////////
         uint4 watch_dog_id;

         ////////////////////////////////////////////////////////////
         // first_message_sent
         //
         // Keeps track of whether the first message of the transaction has
         // been sent since the transaction was created or the transaction id
         // re-assigned.
         ////////////////////////////////////////////////////////////
         bool first_message_sent;

         ////////////////////////////////////////////////////////////
         // messages_sent_count
         //
         // Keeps track of the total number of messages that have been sent
         // through this transaction.  This value will not be reset if the
         // transaction is re-assigned a new transaction number.
         ////////////////////////////////////////////////////////////
         uint4 messages_sent_count;

         ////////////////////////////////////////////////////////////
         // pending_messages
         //
         // The list of messages that have been posted through this transaction
         // that have not yet been sent.  This list is added to when
         // send_message() is called and is removewd from when
         // on_sending_message() is called.  The purpose of this queue is to
         // allow messages to be cancelled if the transaction is closed while
         // the messages are pending.
         ////////////////////////////////////////////////////////////
         typedef std::list<message_handle> pending_messages_type;
         pending_messages_type pending_messages;

         ////////////////////////////////////////////////////////////
         // started_session
         //
         // Set to true when on_sending_message() is called for the first time.
         ////////////////////////////////////////////////////////////
         bool started_session;

         ////////////////////////////////////////////////////////////
         // closing_id
         ////////////////////////////////////////////////////////////
         uint4 closing_id;

         ////////////////////////////////////////////////////////////
         // unroutable_id
         //
         // Stores the id for the timer that is used when a message is sent
         // that cannot be routed.
         ////////////////////////////////////////////////////////////
         uint4 unroutable_id;

         ////////////////////////////////////////////////////////////
         // was_preempted
         //
         // Indicates whether this transaction was pre-empted by a hello
         // transaction.  This will be used to prevent future pre-emption. 
         ////////////////////////////////////////////////////////////
         bool was_preempted;

         ////////////////////////////////////////////////////////////
         // report_id
         ////////////////////////////////////////////////////////////
         int8 report_id;
      };
   };
};


#endif

