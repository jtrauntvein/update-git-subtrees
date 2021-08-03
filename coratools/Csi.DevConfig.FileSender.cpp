/* Csi.DevConfig.FileSender.cpp

   Copyright (C) 2008, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 11 April 2008
   Last Change: Tuesday 19 September 2017
   Last Commit: $Date: 2017-09-19 10:49:29 -0600 (Tue, 19 Sep 2017) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.FileSender.h"
#include "coratools.strings.h"
#include "Csi.MsgExcept.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         uint4 const max_payload = 990;
      };

      
      ////////////////////////////////////////////////////////////
      // class FileSender definitions
      ////////////////////////////////////////////////////////////
      FileSender::FileSender():
         security_code(0),
         state(state_standby),
         transaction_no(0),
         current_offset(0),
         last_fragment_sent(false)
      { }


      FileSender::~FileSender()
      {
         session.clear();
      } // destructor


      void FileSender::start(
         StrAsc const &file_name_,
         Csi::SharedPtr<SessionBase> session_,
         uint2 security_code_)
      {
         // initialise the transaction state
         if(state != state_standby)
            throw MsgExcept("file sender already started");
         session = session_;
         security_code = security_code_;
         file_name = file_name_;
         current_offset = 0;
         transaction_no = 0;
         state = state_active;

         // before we form the first message, we need to get the first buffer of data from the
         // application.  The max size of this buffer will depend upon the size of the file name.
         // Because many devices will be writing to flash, the code is much more efficient if the
         // fragment length and offsets are multiples of two.  Because of this, we will always
         // constrain the max fragment length to be a multiple of two.
         if(file_name.length() > max_payload)
            throw std::invalid_argument("File name is too long");
         
         // we can now form the first message
         message_handle cmd(new Message);
         uint4 fragment_len = max_payload - (uint4)file_name.length();

         if(fragment_len % 2 != 0 && fragment_len > 0)
            --fragment_len;
         send_buffer.cut(0);
         get_next_fragment(
            send_buffer, last_fragment_sent, fragment_len, 0);
         cmd->set_message_type(Messages::send_file_cmd);
         cmd->addUInt2(security_code);
         cmd->addUInt4(current_offset);
         cmd->addBool(last_fragment_sent);
         cmd->addAsciiZ(file_name.c_str());
         if(send_buffer.length() > 0)
            cmd->addBytes(send_buffer.getContents(), (uint4)send_buffer.length());
         session->add_transaction(this, cmd, 3, 3000);
      } // start


      void FileSender::describe_outcome(std::ostream &out, outcome_type outcome, StrAsc const &explanation)
      {
         using namespace FileSenderStrings;
         switch(outcome)
         {
         case outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case outcome_invalid_file_name:
            out << my_strings[strid_outcome_invalid_file_name]; 
            break;
            
         case outcome_link_failed:
            out << my_strings[strid_outcome_link_failed];
            break;
            
         case outcome_timed_out:
            out << my_strings[strid_outcome_timed_out];
            break;
            
         case outcome_invalid_fragment_len:
            out << my_strings[strid_outcome_invalid_fragment_len];
            break;

         case outcome_corrupt:
            out << my_strings[strid_outcome_corrupt];
            break;

         case outcome_incompatible:
            if(explanation.length())
               out << explanation;
            else
               out << my_strings[strid_outcome_incompatible];
            break;
            
         default:
            out << my_strings[strid_outcome_unknown];
            break;
         }
      } // describe_outcome

      
      void FileSender::on_complete(
         message_handle &command, message_handle &response)
      {
         try
         {
            if(state == state_active && response->get_message_type() == Messages::send_file_ack)
            {
               // read the response message parameters
               byte outcome = response->readByte();
               uint4 offset = response->readUInt4();
               uint2 wait_for_next = response->readUInt2();
               StrAsc explanation;
               
               if(outcome == 1)
               {
                  // the device is waiting for the next fragment.  We will only proceed if the offset
                  // specified matches the last offset sent.  This may be a delayed response to a
                  // retried command and can be ignored.
                  if(offset == current_offset)
                  {
                     transaction_no = command->get_tran_no();
                     if(wait_for_next == 0)
                        send_next_command();
                     else
                     {
                        state = state_wait_for_continue;
                        session->add_transaction(
                           this,
                           message_handle(),
                           1,
                           static_cast<uint4>(wait_for_next) * 1000,
                           transaction_no);
                     }
                  } 
               }
               else if(outcome == 2)
                  do_on_complete(outcome_success, false, 0);
               else if(outcome == 3)
                  do_on_complete(outcome_success, true, wait_for_next);
               else if(outcome == 6)
                  do_on_complete(outcome_corrupt, false, 0);
               else if(outcome == 7)
               {
                  if(response->whatsLeft())
                     response->readAsciiZ(explanation);
                  do_on_complete(outcome_incompatible, false, 0, explanation);
               }
               else
               {
                  // we need to map the device outcome code into our own outcome
                  outcome_type client_outcome = outcome_unknown;
                  if(outcome == 5)
                     client_outcome = outcome_invalid_file_name;
                  do_on_complete(client_outcome);
               }
            }
            else if(state == state_wait_for_continue && response->get_message_type() == Messages::send_file_continue_cmd)
            {
               state = state_active;
               send_next_command();
            }
            else
               do_on_complete(outcome_timed_out);
         }
         catch(std::exception &)
         { do_on_complete(outcome_timed_out); }
      } // on_complete


      void FileSender::on_failure(
         message_handle &command, failure_type failure)
      {
         if(failure == failure_timed_out && state == state_wait_for_continue)
         {
            state = state_active;
            send_next_command();
         }
         else
         {
            outcome_type outcome = outcome_unknown;
            switch(failure)
            {
            case failure_link_failed:
               outcome = outcome_link_failed;
               break;
               
            case failure_timed_out:
               outcome = outcome_timed_out;
               break;
            }
            do_on_complete(outcome);
         }
      } // on_failure


      void FileSender::send_next_command()
      {
         if(!last_fragment_sent)
         {
            message_handle cmd(new Message);
            uint4 fragment_len = max_payload;

            if(fragment_len % 2 != 0)
               --fragment_len;
            current_offset += (uint4)send_buffer.length();
            send_buffer.cut(0);
            cmd->set_message_type(Messages::send_file_cmd);
            cmd->addUInt2(0);   // add security code
            cmd->addUInt4(current_offset);
            get_next_fragment(send_buffer, last_fragment_sent, fragment_len, current_offset);
            cmd->addBool(last_fragment_sent);
            cmd->addBytes(send_buffer.getContents(), (uint4)send_buffer.length());
            session->add_transaction(this, cmd, 3, 3000, transaction_no);
         }
         else
            do_on_complete(outcome_success, false, 0);
      } // send_next_command


      void FileSender::do_on_complete(
         outcome_type outcome, bool will_reset, uint2 wait_after, StrAsc const &explanation)
      {
         state = state_standby;
         session.clear();
         security_code = 0;
         on_complete(outcome, will_reset, wait_after, explanation);
      } // do_on_complete
   };
};

