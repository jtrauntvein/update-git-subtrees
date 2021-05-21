/* Cora.LgrNet.FileRetriever.h

   Copyright (C) 2009, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 07 January 2009
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_LgrNet_FileRetriever_h
#define Cora_LgrNet_FileRetriever_h

#include "Cora.ClientBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace LgrNet
   {
      // @group: class forward declarations
      class FileRetriever;
      // @endgroup


      ////////////////////////////////////////////////////////////
      // class FileRetrieverClient
      ////////////////////////////////////////////////////////////
      class FileRetrieverClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_unsupported = 3,
            outcome_server_security_blocked = 4,
            outcome_session_failed = 5,
            outcome_invalid_file_name = 6,
            outcome_server_file_open_failed = 7,
            outcome_local_file_open_failed = 8
         };
         virtual void on_complete(
            FileRetriever *retriever, outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class FileRetriever
      //
      // Defines a component that is able to read a file from the LoggerNet
      // server and store it locally.  In order to use this component, the
      // application must provide an object that is derived from class
      // FileRetrieverClient.  It should construct an object of this class and
      // set properties including the remote file name and local file name.
      // Once this is done, the application should invoke one of the two
      // start() methods.  When the transfer is complete, the client's
      // on_complete() method will be invoked. 
      ////////////////////////////////////////////////////////////
      class FileRetriever:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         FileRetrieverClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // output
         ////////////////////////////////////////////////////////////
         FILE *output;

         ////////////////////////////////////////////////////////////
         // remote_file_name
         ////////////////////////////////////////////////////////////
         StrAsc remote_file_name;

         ////////////////////////////////////////////////////////////
         // local_file_name
         ////////////////////////////////////////////////////////////
         StrAsc local_file_name;

         ////////////////////////////////////////////////////////////
         // file_size
         ////////////////////////////////////////////////////////////
         int8 file_size;

         ////////////////////////////////////////////////////////////
         // file_modified_date
         ////////////////////////////////////////////////////////////
         int8 file_modified_date;

         ////////////////////////////////////////////////////////////
         // rx_buff
         ////////////////////////////////////////////////////////////
         StrBin rx_buff;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         FileRetriever():
            state(state_standby),
            client(0),
            output(0)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~FileRetriever()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // get_remote_file_name
         ////////////////////////////////////////////////////////////
         StrAsc const &get_remote_file_name() const
         { return remote_file_name; }

         ////////////////////////////////////////////////////////////
         // set_remote_file_name
         ////////////////////////////////////////////////////////////
         void set_remote_file_name(StrAsc const &name)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            remote_file_name = name;
         }

         ////////////////////////////////////////////////////////////
         // get_local_file_name
         ////////////////////////////////////////////////////////////
         StrAsc const &get_local_file_name() const
         { return local_file_name; }

         ////////////////////////////////////////////////////////////
         // set_local_file_name
         ////////////////////////////////////////////////////////////
         void set_local_file_name(StrAsc const &name)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            local_file_name = name;
         }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef FileRetrieverClient client_type;
         void start(client_type *client_, router_handle router);
         void start(client_type *client, ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // format_outcome
         ////////////////////////////////////////////////////////////
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      protected:
         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure()
         { on_corabase_failure(corabase_failure_session); }

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);
      };
   };
};


#endif
