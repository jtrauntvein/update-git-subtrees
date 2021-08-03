/* Cora.LgrNet.BackupCreator.h

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 22 December 2004
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2019-10-30 16:17:59 -0600 (Wed, 30 Oct 2019) $ 
   Last Changed by: $Author: amortenson $

*/

#ifndef Cora_LgrNet_BackupCreator_h
#define Cora_LgrNet_BackupCreator_h

#include "Cora.ClientBase.h"
#include "Csi.Events.h"
#include <list>
#include <algorithm>
#include <iterator>


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class BackupCreator;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class BackupCreatorClient
      ////////////////////////////////////////////////////////////
      class BackupCreatorClient: public Csi::InstanceValidator
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
            outcome_session_broken = 3,
            outcome_server_security_blocked = 4,
            outcome_unsupported = 5,
            outcome_invalid_file_name = 6,
            outcome_no_resources = 7
         };
         virtual void on_complete(
            BackupCreator *creator,
            outcome_type outcome,
            StrAsc const &file_name) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class BackupCreator
      //
      // Defines a component that drives the LgrNet Create Backup File
      // transaction.  In order to use this component, an application must
      // provide an object derived from class BackupCreatorClient which will
      // receive the completion event notification.  It should then create an
      // instance of class BackupCreator, set its properties as appropriate
      // (there are no required properties), and invoke one of the versions of
      // start().  When the server transaction is complete, the client's
      // on_complete() method will be invoked.
      ////////////////////////////////////////////////////////////
      class BackupCreator:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // file_name
         //
         // Specifies the name of the file that will be created.  The path of
         // this name should be relative to the SERVERS file system and not the
         // client's file system.
         ////////////////////////////////////////////////////////////
         StrAsc file_name;

         ////////////////////////////////////////////////////////////
         // include_table_files
         //
         // Set to true if the server should back up its cache table files as
         // well as the configuration files.
         ////////////////////////////////////////////////////////////
         bool include_table_files;

         ////////////////////////////////////////////////////////////
         // additional_files
         ////////////////////////////////////////////////////////////
         typedef std::list<StrAsc> additional_files_type;
         additional_files_type additional_files;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         BackupCreatorClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         BackupCreator():
            include_table_files(false),
            client(0),
            state(state_standby)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~BackupCreator()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // set_file_name
         ////////////////////////////////////////////////////////////
         void set_file_name(StrAsc const &file_name_)
         {
            if(state == state_standby)
               file_name = file_name_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_file_name
         ////////////////////////////////////////////////////////////
         StrAsc const &get_file_name() const
         { return file_name; }

         ////////////////////////////////////////////////////////////
         // set_include_table_files
         ////////////////////////////////////////////////////////////
         void set_include_table_files(bool include_table_files_)
         {
            if(state == state_standby)
               include_table_files = include_table_files_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_include_table_files
         ////////////////////////////////////////////////////////////
         bool get_include_table_files() const
         { return include_table_files; }

         ////////////////////////////////////////////////////////////
         // clear_additional_files
         ////////////////////////////////////////////////////////////
         void clear_additional_files()
         {
            if(state == state_standby)
               additional_files.clear();
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // add_additional_file
         ////////////////////////////////////////////////////////////
         void add_additional_file(StrAsc const &additional_file_)
         {
            if(state == state_standby)
               additional_files.push_back(additional_file_);
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // add_additional_files
         //
         // Adds a collection of file names passed in as an iterator range.
         // The iterators when dereferenced will be expected to be values of
         // type StrAsc (or something that can be converted directly to
         // StrAsc). 
         ////////////////////////////////////////////////////////////
         template <class it>
         void add_additional_files(it begin, it end)
         {
            if(state == state_standby)
            {
               std::copy(
                  begin,
                  end,
                  std::back_inserter(additional_files)); 
            }
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef BackupCreatorClient client_type;
         void start(
            client_type *client_,
            router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(router);
         }


         ////////////////////////////////////////////////////////////
         // start (from other components connection)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(other_component);            
         }

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         {
            client = 0;
            state = state_standby;
            ClientBase::finish();
         }

         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure()
         { on_corabase_failure(corabase_failure_session); }
      };
   };
};


#endif
