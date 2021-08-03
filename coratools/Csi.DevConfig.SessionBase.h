/* Csi.DevConfig.SessionBase.h

   Copyright (C) 2003, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 17 December 2003
   Last Change: Wednesday 24 January 2018
   Last Commit: $Date: 2018-01-25 10:51:49 -0600 (Thu, 25 Jan 2018) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SessionBase_h
#define Csi_DevConfig_SessionBase_h

#include "Csi.DevConfig.Message.h"
#include "Csi.InstanceValidator.h"
#include <list>


namespace Csi
{
   namespace DevConfig
   {
      /**
       * Defines the application interface to a devconfig session.   An application object must
       * extend this class.
       */
      class TransactionClient: public InstanceValidator
      {
      public:
         /**
          * Called when a specified transaction has been carried out.
          *
          * @param command Specifies the command message that was sent.
          *
          * @param response Specifies the response message that was received.
          */
         typedef SharedPtr<Message> message_handle;
         virtual void on_complete(
            message_handle &command,
            message_handle &response) = 0;

         /**
          * Called when the session, after a specified number of retries or because of a low level
          * link failed, could not carry out the specified command.
          *
          * @param command Specifies the command that failed.
          *
          * @param failure Specifies the reason for the failure.
          */
         enum failure_type
         {
            failure_unknown = 0,
            failure_link_failed = 1,
            failure_timed_out = 2,
         };
         virtual void on_failure(
            message_handle &command,
            failure_type failure) = 0;

         /**
          * Called when the specified command message is about to be sent by the session object.
          *
          * @param message Specifies the message that is to be sent.
          */
         virtual void on_sending_command(
            message_handle &message)
         { }
      };


      /**
       * Defines a base class for all objects that support or emulate devconfig protocol sessions.
       */
      class SessionBase
      {
      public:
         /**
          * Destructor
          */
         virtual ~SessionBase()
         { }

         /**
          * Must be overloaded to implement that details of adding a new devconfig transaction.
          *
          * @param client Specifies the application object that will receive completion event
          * notifications.
          *
          * @param command Specifies the command to be sent.
          *
          * @param max_retry_count Specifies the maximum number of times that the command should be
          * retried.
          *
          * @param extra_timeout_interval Specifies the amount of time that should be applied for
          * each try.
          *
          * @param tran_no Specifies the transaction number that should be used.
          */
         typedef SharedPtr<Message> message_handle;
         virtual void add_transaction(
            TransactionClient *client,
            message_handle command,
            uint4 max_retry_count,
            uint4 extra_timeout_interval,
            byte tran_no = 0) = 0;

         /**
          * @return Can be overloaded to indicate that this driver supports the reset operation.
          */
         virtual bool supports_reset()
         { return true; }

         /**
          * Can be overloaded to put the session in a suspended state meaing that the communication
          * medium has been released and any timers shut down.  The session should be able to be
          * pulled out of a suspended state when new transactions are added.
          */
         virtual void suspend()
         { }

         /**
          * @return Returns true if the session is an emulation.
          */
         virtual bool is_emulation() const
         { return false; }

         /**
          * @return Returns any warnings received while trying to commit.
          */
         typedef std::list<StrAsc> commit_warnings_type;
         commit_warnings_type const &get_commit_warnings() const
         { return commit_warnings; }

      protected:
         /**
          * Specifies the list of warnings accoumulated while trying to commit.
          */
         commit_warnings_type commit_warnings;
      };
   };
};


#endif
