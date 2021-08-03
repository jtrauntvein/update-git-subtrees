/* Csi.DevConfig.FileSender.h

   Copyright (C) 2008, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 10 April 2008
   Last Change: Friday 12 October 2018
   Last Commit: $Date: 2018-10-19 14:32:53 -0600 (Fri, 19 Oct 2018) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_DevConfig_FileSender_h
#define Csi_DevConfig_FileSender_h

#include "Csi.DevConfig.SessionBase.h"


namespace Csi
{
   namespace DevConfig
   {
      /**
       * Defines a component that helps the application implement the client side of the DevConfig
       * file send transaction.  The content of the file is obtained through calls to virtual
       * methods.  In order to use this component, the application must provide its own derived
       * class that overloads the methods for reporting status as well as obtaining file data.
       */
      class FileSender:
         public TransactionClient
      {
      public:
         /**
          * Constructor
          */
         FileSender();

         /**
          * Destructor
          */
         virtual ~FileSender();

         /**
          * Called to start the file send transaction.
          *
          * @param file_name_ Specifies the name of the file to be sent.
          *
          * @param session_ Specifies the communication session with the device.
          *
          * @param security_code_ Specifies the device security code.
          */
         virtual void start(
            StrAsc const &file_name_,
            Csi::SharedPtr<SessionBase> session_,
            uint2 security_code_ = 0);

         // Lists the possible outcomes from attempting to send a file.u
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_file_name = 3,
            outcome_link_failed = 4,
            outcome_timed_out = 5,
            outcome_invalid_fragment_len = 6,
            outcome_corrupt = 7,
            outcome_incompatible = 8
         };

      protected:
         /**
          * Must be overloaded to fill the send buffer with up to max_len bytes of data at the
          * specified starting offset.  If the buffer is filled with the last fragment, the
          * application must set the last_fragment parameter to true.
          *
          * @param send_buffer Specifies the buffer to which the packet data must be written.
          *
          * @param last_fragment The application must set this to true if this is the last fragment
          * of the file to be sent.
          *
          * @param max_len Specifies the maximum number of bytes that the application can write to
          * the send_buffer.
          *
          * @param offset Specifies the offset into the file.
          */
         virtual void get_next_fragment(
            StrBin &send_buffer,
            bool &last_fragment,
            uint4 max_len,
            uint4 offset) = 0;

         /**
          * Called after the last fragment sent has been acknowledged by the device.
          *
          * @param outcome Specifies the outcome of the transaction.
          *
          * @param will_reset Set to true if the device will reset as a result of this transaction.
          * If this set to true, the application must wait at least wait_after seconds before
          * attempting further communications.
          *
          * @param wait_after Specifies the number of seconds that the application must wait before
          * the device will be ready to communicate again.
          *
          * @param explanation Specifies the device supplied explanation for incompatibility. 
          */
         virtual void on_complete(
            outcome_type outcome,
            bool will_reset,
            uint2 wait_after,
            StrAsc const &explanation) = 0;

         /**
          * Formats the outcome of the transaction.
          *
          * @param out Specifies the stream to which the formatted outcome will be written.
          *
          * @param outcome Specifies the outcome code to format.
          *
          * @param explanation Specifies the explanation string that was sent by the device.
          */
         static void describe_outcome(std::ostream &out, outcome_type outcome, StrAsc const &explanation = "");

         // @group: Methods overloaded from class TransactionClient

         /**
          * Overloads the base class version to handle the completion of an exchange of messages.
          */
         virtual void on_complete(
            message_handle &command, message_handle &response);

         /**
          * Overloads the base class version to handle a failure.
          */
         virtual void on_failure(
            message_handle &command, failure_type failure);
         
         // @endgroup:

         /**
          * Performs the work of reporting completion to the application.
          */
         void do_on_complete(
            outcome_type outcome,
            bool will_reset = false,
            uint2 wait_after = 0,
            StrAsc const &explanation = "");
         
      private:
         /**
          * Prepares to send the next command.
          */
         void send_next_command();

      protected:
         /**
          * Specifies the name of the file to be sent.
          */
         StrAsc file_name;
         
         /**
          * Specifies the communication session with the device.
          */
         Csi::SharedPtr<SessionBase> session;

         /**
          * Specifies the device security code.
          */
         uint2 security_code;

         /**
          * Specifies the buffer that will be used to hold the next fragment to be sent.
          */
         StrBin send_buffer;

         /**
          * Specifies the state of this transaction.
          */
         enum state_type
         {
            state_standby,
            state_active,
            state_wait_for_continue
         } state;

         /**
          * Set to true if the last fragment has been sent.
          */
         bool last_fragment_sent;

         /**
          * Specifies the offset into the file.
          */
         uint4 current_offset;

         /**
          * Specifies the transaction number.
          */
         byte transaction_no;
      };
   };
};


#endif
