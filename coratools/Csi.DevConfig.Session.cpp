/* Csi.DevConfig.Session.cpp

   Copyright (C) 2003, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 20 December 2003
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.Session.h"
#include "Csi.PakBus.SerialDecode.h"
#include "Csi.Utils.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // session_timer_interval
         //
         // Specifies the interval at which the session should send refresh commands when nothing
         // else is happening on the transaction level.
         ////////////////////////////////////////////////////////////
         uint4 const session_timer_interval = 15000;

         
         ////////////////////////////////////////////////////////////
         // class event_tran_failed
         ////////////////////////////////////////////////////////////
         class event_tran_failed: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // transaction
            ////////////////////////////////////////////////////////////
            typedef SharedPtr<SessionHelpers::Transaction> tran_type;
            tran_type tran;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef TransactionClient::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               Session *session,
               tran_type &tran,
               failure_type failure)
            {
               try{(new event_tran_failed(session,tran,failure))->post(); }
               catch(Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_tran_failed(
               Session *session,
               tran_type &tran_,
               failure_type failure_):
               Event(event_id,session),
               tran(tran_),
               failure(failure_)
            { }
         };


         uint4 const event_tran_failed::event_id =
         Event::registerType("Csi::DevConfig::Session::event_tran_failed");


         ////////////////////////////////////////////////////////////
         // class event_tran_complete
         ////////////////////////////////////////////////////////////
         class event_tran_complete: public Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // tran
            ////////////////////////////////////////////////////////////
            typedef SharedPtr<SessionHelpers::Transaction> tran_type;
            tran_type tran;

            ////////////////////////////////////////////////////////////
            // response
            ////////////////////////////////////////////////////////////
            typedef SharedPtr<Message> response_type;
            response_type response;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               Session *session,
               tran_type &tran,
               response_type &response)
            {
               try{(new event_tran_complete(session,tran,response))->post();}
               catch(Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_tran_complete(
               Session *session,
               tran_type &tran_,
               response_type &response_):
               Event(event_id,session),
               tran(tran_),
               response(response_)
            { }
         };


         uint4 const event_tran_complete::event_id =
         Event::registerType("Csi::DevConfig::Session::event_tran_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class Session definitions
      ////////////////////////////////////////////////////////////
      Session::Session(
         SharedPtr<SessionDriverBase> driver_,
         SharedPtr<OneShot> timer_,
         uint4 start_retry_count_,
         uint4 start_timeout_):
         driver(driver_),
         timer(timer_),
         session_timer(0),
         should_unquote_next(false),
         last_tran_no(128),
         terminal(0),
         suspended(false),
         start_retry_count(start_retry_count_),
         start_timeout(start_timeout_)
      { driver->set_session(this); }


      Session::~Session()
      {
         if(session_timer)
            timer->disarm(session_timer);
      } // destructor
      
      
      void Session::add_transaction(
         TransactionClient *client,
         message_handle command,
         uint4 max_retry_count,
         uint4 timeout_interval,
         byte tran_no)
      {
         bool should_start_now = transactions.empty();
         if(session_timer)
            timer->disarm(session_timer);
         if(tran_no == 0)
         {
            if(++last_tran_no == 0)
               last_tran_no = 1;
            tran_no = last_tran_no;
         }
         if(command != 0)
         {
            command->set_tran_no(tran_no);
            command->addUInt2(
               calcSigNullifier(
                  calcSigFor(command->getMsg(),command->length())));
         }
         transactions.push_back(
            new SessionHelpers::Transaction(
               client,
               command,
               max_retry_count,
               timeout_interval));
         if(should_start_now)
            send_next_message(); 
      } // add_transaction


      void Session::suspend()
      {
         if(!transactions.empty())
            throw MsgExcept("DevConfig Session cannot be suspended with pending transactions");
         if(session_timer)
            timer->disarm(session_timer);
         driver->close(this);
         suspended = true;
      } // suspend

      
      void Session::on_driver_open()
      {
         // initialise the low level state
         input_buffer.cut(0);
         should_unquote_next = false;

         // We want to add an initial transaction to start the session.
         if(!suspended && start_retry_count != 0)
         {
            message_handle cmd(new Message);
            uint4 retry_count(start_retry_count);
            uint4 timeout(start_timeout);
            
            cmd->set_message_type(Messages::control_cmd);
            cmd->addUInt2(0);
            cmd->addByte(ControlCodes::action_refresh_timer);
            cmd->addUInt2(
               calcSigNullifier(
                  calcSigFor(cmd->getMsg(),cmd->length())));
            if(retry_count == 0xFFFFFFFF)
               retry_count = 375;
            if(timeout == 0xFFFFFFFF)
               timeout = 40;
            transactions.push_front(
               new SessionHelpers::Transaction(
                  0, cmd, retry_count, timeout));
         }
         if(term_queue.length() > 0)
         {
            driver->send(this, term_queue.getContents(), (uint4)term_queue.length());
            term_queue.cut(0);
         }
         suspended = false;

         // send the next message
         send_next_message();
      } // on_driver_open

      
      void Session::on_driver_data(
         void const *buff,
         uint4 buff_len)
      {
         using namespace Csi::PakBus::SerialDecode;
         uint4 start_offset = 0;
         decode_outcome_type outcome;
         uint4 decode_len;
         byte const *byte_buff = static_cast<byte const *>(buff);

         if(terminal != 0)
         {
            using namespace SessionHelpers;
            if(TerminalBase::is_valid_instance(terminal))
               terminal->receive_data(buff,buff_len);
            else
               terminal = 0;
         }
         while(start_offset < buff_len)
         {
            decode_len = decode_quoted_data(
               input_buffer,
               should_unquote_next,
               outcome,
               byte_buff + start_offset,
               buff_len - start_offset);
            start_offset += decode_len;
            if(outcome == decode_synch_found)
            {
               // the presence of a serial synch byte indicates the possibility of a valid message
               // but we must first check the length of the message.  It must accomodate at least
               // the header and a signature nullifier
               if(input_buffer.length() >= Message::header_len + 2)
               {
                  uint2 buff_sig = calcSigFor(
                     input_buffer.getContents(),
                     input_buffer.length());
                  if(buff_sig == 0)
                  {
                     // finally the first byte of the message must indicate the protocol type
                     if(input_buffer[0] == Message::packet_type_byte)
                     {
                        message_handle message(
                           new Message(
                              input_buffer.getContents(),
                              (uint4)input_buffer.length() - 2));
                        on_message_read(message);
                     }
                     input_buffer.cut(0);
                  }
                  else
                     input_buffer.cut(0);
               }
               else
                  input_buffer.cut(0);
            }
            else if(outcome != decode_synch_not_found)
               input_buffer.cut(0);
         }
      } // on_driver_data

      
      void Session::on_driver_failure()
      {
         transactions_type temp(transactions);

         transactions.clear();
         if(session_timer)
            timer->disarm(session_timer);
         if(tran_timer)
            timer->disarm(tran_timer);
         if(terminal && SessionHelpers::TerminalBase::is_valid_instance(terminal))
            terminal->on_driver_failure();
         terminal = 0;
         while(!temp.empty())
         {
            event_tran_failed::cpost(
               this,
               temp.front(),
               TransactionClient::failure_link_failed);
            temp.pop_front();
         }
         suspended = false;
         term_queue.cut(0);
      } // on_driver_failure

      
      void Session::onOneShotFired(uint4 id)
      {
         if(id == session_timer)
         {
            session_timer = 0;
            if(transactions.empty())
            {
               message_handle cmd(new Message);
               cmd->set_message_type(Messages::control_cmd);
               cmd->addUInt2(0);
               cmd->addByte(ControlCodes::action_refresh_timer);
               add_transaction(0,cmd,3,1000);
            }
         }
         else if(id == tran_timer)
         {
            tran_timer = 0;
            if(!transactions.empty())
            {
               SharedPtr<SessionHelpers::Transaction> &tran = transactions.front();
               if(++tran->retry_count > tran->max_retry_count)
               {
                  event_tran_failed::cpost(
                     this,
                     tran,
                     TransactionClient::failure_timed_out);
                  transactions.pop_front();
                  if(transactions.empty())
                     session_timer = timer->arm(this,session_timer_interval);
               }
               send_next_message();
            }
         }
      } // onOneShotFired


      void Session::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == event_tran_failed::event_id)
         {
            event_tran_failed *event = static_cast<event_tran_failed *>(ev.get_rep());
            if(TransactionClient::is_valid_instance(event->tran->client))
            {
               event->tran->client->on_failure(
                  event->tran->command,
                  event->failure);
            }
         }
         else if(ev->getType() == event_tran_complete::event_id)
         {
            event_tran_complete *event = static_cast<event_tran_complete *>(ev.get_rep());
            if(TransactionClient::is_valid_instance(event->tran->client))
            {
               event->tran->client->on_complete(
                  event->tran->command,
                  event->response);
            }
         }
      } // receive


      void Session::send_terminal_data(
         SessionHelpers::TerminalBase *terminal_,
         void const *buff,
         uint4 buff_len)
      {
         if(transactions.empty())
         {
            // send the data
            if(SessionHelpers::TerminalBase::is_valid_instance(terminal_))
            {
               // if the session timer is active, we need to get the device into a state where it
               // can send and receive terminal data.  we will do this by transmitting a cancel
               // control command
               if(session_timer)
               {
                  message_handle cancel_cmd(new Message);
                  cancel_cmd->set_message_type(Messages::control_cmd);
                  cancel_cmd->addUInt2(0); // security code
                  cancel_cmd->addByte(ControlCodes::action_cancel_without_reboot);
                  add_transaction(0,cancel_cmd,3,1000);
               }

               // remember the terminal object so data can be sent to it
               terminal = terminal_;
               if(driver->is_open(this))
                  driver->send(this, buff, buff_len);
               else
               {
                  driver->start_open(this);
                  term_queue.append(buff,buff_len);
               }
            }
         }
         else
            throw MsgExcept("Cannot do terminal data while transactions are active");
      } // send_terminal_data

      
      void Session::send_next_message()
      {
         if(!transactions.empty())
         {
            if(driver->is_open(this))
            {
               // encode the command and send it through the driver
               using namespace Csi::PakBus::SerialDecode;
               SharedPtr<Message> &command(transactions.front()->command);
               if(command != 0)
               {
                  byte const *message_src = reinterpret_cast<byte const *>(
                     command->getMsg());
                  
                  output_buffer.cut(0);
                  output_buffer.reserve(command->length() + 10);
                  output_buffer.append(&synch_byte, 1);
                  for(uint4 i = 0; i < command->length(); ++i)
                  {
                     byte temp = message_src[i];
                     if(temp == synch_byte || temp == quote_byte)
                     {
                        output_buffer.append(&quote_byte, 1);
                        temp += 0x20;
                     }
                     output_buffer.append(&temp,1);
                  }
                  output_buffer.append(&synch_byte, 1);
                  driver->send(this, output_buffer.getContents(), (uint4)output_buffer.length());
               }
               
               // set the transaction timer
               tran_timer = timer->arm(
                  this,
                  transactions.front()->timeout_interval); 
            }
            else
               driver->start_open(this);
         }
      } // send_next_message


      void Session::on_message_read(message_handle &message)
      {
         // in order to be processed, the message transaction number and type should be related to
         // the last command that was sent.
         if(!transactions.empty())
         {
            tran_handle &tran = transactions.front();
            if(tran->command == 0 ||
               (message->get_tran_no() == tran->command->get_tran_no() &&
                (message->get_message_type() & 0x7f) == tran->command->get_message_type()))
            {
               timer->disarm(tran_timer);
               event_tran_complete::cpost(this,tran,message);
               transactions.pop_front(); 
               if(!transactions.empty())
                  send_next_message();
               else
               {
                  bool set_session_timer = true;
                  if(message->get_message_type() == Messages::control_ack)
                  {
                     byte outcome;
                     message->reset();
                     outcome = message->readByte();
                     if(outcome != ControlCodes::outcome_session_timer_reset &&
                        outcome != ControlCodes::outcome_reverted_to_defaults)
                        set_session_timer = false;
                  }
                  if(set_session_timer)
                     session_timer= timer->arm(this,session_timer_interval);
               }
            }
         }
      } // on_message_read
   };
};


