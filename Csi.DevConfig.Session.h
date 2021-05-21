/* Csi.DevConfig.Session.h

   Copyright (C) 2003, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 19 December 2003
   Last Change: Monday 03 October 2011
   Last Commit: $Date: 2011-10-05 15:49:00 -0600 (Wed, 05 Oct 2011) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_Session_h
#define Csi_DevConfig_Session_h

#include <list>
#include "Csi.DevConfig.SessionBase.h"
#include "OneShot.h"


namespace Csi
{
   namespace DevConfig
   {
      //@group class forward declarations
      class Session;
      //@endgroup

      
      ////////////////////////////////////////////////////////////
      // class SessionDriverBase
      //
      // Defines a base class for objects that can serve as low level drivers
      // for objects of class Session.  These drivers are active objects that
      // provide low level communication services for the session.
      ////////////////////////////////////////////////////////////
      class SessionDriverBase
      {
      protected:
         ////////////////////////////////////////////////////////////
         // session
         //
         // Reference to the session 
         ////////////////////////////////////////////////////////////
         Session *session;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SessionDriverBase():
            session(0)
         { }
         
         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SessionDriverBase()
         { }

         ////////////////////////////////////////////////////////////
         // set_session
         ////////////////////////////////////////////////////////////
         void set_session(Session *session_)
         { session = session_; }
         
         ////////////////////////////////////////////////////////////
         // start_open
         //
         // Must be overloaded to implement the open operation.  Once the link
         // is opened, the session's on_driver_open() method should be invoked.
         // This method may be re-invoked while the driver is opending.  In
         // this case, this method should be a no-op.
         ////////////////////////////////////////////////////////////
         virtual void start_open(Session *session) = 0;

         ////////////////////////////////////////////////////////////
         // is_open
         //
         // Must be overloaded to indicate whether the driver is in an opened
         // state. 
         ////////////////////////////////////////////////////////////
         virtual bool is_open(Session *session) = 0;

         ////////////////////////////////////////////////////////////
         // close
         //
         // Must be overloaded to implement the link close operation.  
         ////////////////////////////////////////////////////////////
         virtual void close(Session *session) = 0;

         ////////////////////////////////////////////////////////////
         // send
         //
         // Must be overloaded to transmit the characters in the specified
         // buffer on the link.  When characters are received from the link,
         // the driver should invoke the session's on_serial_data() method. 
         ////////////////////////////////////////////////////////////
         virtual void send(
            Session *session, void const *buff, uint4 buff_len) = 0;
      };


      namespace SessionHelpers
      {
         ////////////////////////////////////////////////////////////
         // class Transaction
         ////////////////////////////////////////////////////////////
         class Transaction
         {
         public:
            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            TransactionClient *client;

            ////////////////////////////////////////////////////////////
            // command
            ////////////////////////////////////////////////////////////
            SharedPtr<Message> command;

            ////////////////////////////////////////////////////////////
            // max_retry_count
            ////////////////////////////////////////////////////////////
            uint4 max_retry_count;

            ////////////////////////////////////////////////////////////
            // timeout_interval
            ////////////////////////////////////////////////////////////
            uint4 timeout_interval;

            ////////////////////////////////////////////////////////////
            // retry_count
            ////////////////////////////////////////////////////////////
            uint4 retry_count;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            Transaction(
               TransactionClient *client_,
               SharedPtr<Message> &command_,
               uint4 max_retry_count_,
               uint4 timeout_interval_):
               client(client_),
               command(command_),
               max_retry_count(max_retry_count_),
               timeout_interval(timeout_interval_),
               retry_count(0)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class TerminalBase
         //
         // Defines an interface to an object that is able to receive input and
         // send output to the device.  
         ////////////////////////////////////////////////////////////
         class TerminalBase: public InstanceValidator
         {
         public:
            ////////////////////////////////////////////////////////////
            // receive_data
            ////////////////////////////////////////////////////////////
            virtual void receive_data(
               void const *buff,
               uint4 buff_len) = 0;

            ////////////////////////////////////////////////////////////
            // on_driver_failure
            ////////////////////////////////////////////////////////////
            virtual void on_driver_failure() = 0;
         };
      };
      
      
      ////////////////////////////////////////////////////////////
      // class Session
      //
      // This class defines a session object that uses the DevConfig low level
      // protocol to carry out messages.  In order to use this component, an
      // application must provide a driver object derived from class
      // SessionDriverBase in the constructor.
      ////////////////////////////////////////////////////////////
      class Session:
         public SessionBase,
         public OneShotClient,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Session(
            SharedPtr<SessionDriverBase> driver_,
            SharedPtr<OneShot> timer_,
            uint4 start_retry_count_ = 0xFFFFFFFF,
            uint4 start_timeout_ = 0xFFFFFFFF);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Session();

         ////////////////////////////////////////////////////////////
         // add_transaction
         ////////////////////////////////////////////////////////////
         virtual void add_transaction(
            TransactionClient *client,
            message_handle command,
            uint4 max_retry_count,
            uint4 timeout_interval,
            byte tran_no = 0);

         ////////////////////////////////////////////////////////////
         // suspend
         //
         // Suspends this session by closing the driver and cancelling the session timer.  This
         // condition can be backed out by adding a transaction or when send_terminal_data() is
         // invoked.    This method will throw an exception if the session is currently servicing
         // one or more transactions.  
         ////////////////////////////////////////////////////////////
         virtual void suspend();

         ////////////////////////////////////////////////////////////
         // on_driver_open
         //
         // Called when the driver has been successfully opened and is ready to
         // send and receive data.
         ////////////////////////////////////////////////////////////
         virtual void on_driver_open();

         ////////////////////////////////////////////////////////////
         // on_driver_data
         //
         // Called when the driver has received data.
         ////////////////////////////////////////////////////////////
         virtual void on_driver_data(
            void const *buff,
            uint4 buff_len);

         ////////////////////////////////////////////////////////////
         // on_driver_failure
         //
         // Called when the driver has experienced a link or a resource
         // failure. 
         ////////////////////////////////////////////////////////////
         virtual void on_driver_failure();

         ////////////////////////////////////////////////////////////
         // onOneShotFired
         ////////////////////////////////////////////////////////////
         virtual void onOneShotFired(uint4 id);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(SharedPtr<Event> &ev);

         ////////////////////////////////////////////////////////////
         // send_terminal_data
         ////////////////////////////////////////////////////////////
         void send_terminal_data(
            SessionHelpers::TerminalBase *terminal_,
            void const *buff,
            uint4 buff_len);

      private:
         ////////////////////////////////////////////////////////////
         // send_next_message
         ////////////////////////////////////////////////////////////
         void send_next_message();

         ////////////////////////////////////////////////////////////
         // on_message_read
         ////////////////////////////////////////////////////////////
         void on_message_read(message_handle &message);
         
      protected:
         ////////////////////////////////////////////////////////////
         // driver
         ////////////////////////////////////////////////////////////
         SharedPtr<SessionDriverBase> driver;

         ////////////////////////////////////////////////////////////
         // timer
         ////////////////////////////////////////////////////////////
         SharedPtr<OneShot> timer;
         
         ////////////////////////////////////////////////////////////
         // transactions
         //
         // The queue of pending transactions.
         ////////////////////////////////////////////////////////////
         typedef SharedPtr<SessionHelpers::Transaction> tran_handle;
         typedef std::list<tran_handle> transactions_type;
         transactions_type transactions;

         ////////////////////////////////////////////////////////////
         // session_timer
         //
         // Used to identify a timer that is armed when a transaction is ended
         // unless that transaction is a control transaction that would close
         // the session.  That timer will be disarmed when the next transaction
         // is started.  If the timer ever fires, a control command with a
         // reset watchdog timer action will be automatically added so that the
         // session can be kept alive.  
         ////////////////////////////////////////////////////////////
         uint4 session_timer;

         ////////////////////////////////////////////////////////////
         // last_tran_no
         //
         // Keeps track of the last transaction number that was used.  
         ////////////////////////////////////////////////////////////
         byte last_tran_no;

         ////////////////////////////////////////////////////////////
         // input_buffer
         //
         // Buffers up characters as they are received and decoded from the
         // driver.  
         ////////////////////////////////////////////////////////////
         StrBin input_buffer;

         ////////////////////////////////////////////////////////////
         // should_unquote_next
         //
         // Maintains the state of the decoding algorithm between fragments
         // received by the driver. 
         ////////////////////////////////////////////////////////////
         bool should_unquote_next;

         ////////////////////////////////////////////////////////////
         // tran_timer
         //
         // Identifies a watch dog timer for retrying or failing the current
         // transaction.  This timer is set when a command is sent and will be
         // disarmed when the response is received.  If it fires before the
         // response is received, the transaction's retry count will be
         // incremented and retried or failed depedning on the comparison with
         // the transactions max_retry_count and its incremented retry count. 
         ////////////////////////////////////////////////////////////
         uint4 tran_timer;

         ////////////////////////////////////////////////////////////
         // terminal
         //
         // Reference to a client object that is able to handle input and
         // output directly from the device. 
         ////////////////////////////////////////////////////////////
         SessionHelpers::TerminalBase *terminal;

         ////////////////////////////////////////////////////////////
         // term_queue
         //
         // Keeps the terminal bytes that need to be sent while we are waiting for the driver to
         // open. 
         ////////////////////////////////////////////////////////////
         StrBin term_queue;

         ////////////////////////////////////////////////////////////
         // suspended
         ////////////////////////////////////////////////////////////
         bool suspended;

         ////////////////////////////////////////////////////////////
         // output_buffer
         ////////////////////////////////////////////////////////////
         StrBin output_buffer;

         ////////////////////////////////////////////////////////////
         // start_retry_count
         ////////////////////////////////////////////////////////////
         uint4 const start_retry_count;

         ////////////////////////////////////////////////////////////
         // start_timeout
         ////////////////////////////////////////////////////////////
         uint4 const start_timeout;
      };
   };
};


#endif
