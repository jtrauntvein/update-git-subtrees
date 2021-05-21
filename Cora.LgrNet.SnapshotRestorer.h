/* Cora.LgrNet.SnapshotRestorer.h

   Copyright (C) 2004, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 23 December 2004
   Last Change: Thursday 01 November 2018
   Last Commit: $Date: 2019-01-28 17:52:35 -0600 (Mon, 28 Jan 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_SnapshotRestorer_h
#define Cora_LgrNet_SnapshotRestorer_h

#include "Cora.ClientBase.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class SnapshotRestorer;
      //@endgroup


      /**
       * Defines the interface that must be used by an application object to use the
       * SnapshotReceiver component.
       */
      class SnapshotRestorerClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the snapshot restoration has been completed.
          *
          * @param sender Specifies the component that is sending this message.
          *
          * @param outcome Specifies the outcome of the server transaction.
          *
          * @param results Specifies the collection of files and their outcomes provided the value
          * of outcome is one or two.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_partial_success = 2,
            outcome_invalid_logon = 3,
            outcome_session_broken = 4,
            outcome_server_security_blocked = 5,
            outcome_unsupported = 6,
            outcome_invalid_file_name = 7,
            outcome_invalid_snapshot_version = 8,
            outcome_corrupt_snapshot = 9,
            outcome_other_transactions = 10,
            outcome_network_locked = 11,
         };
         typedef std::pair<StrAsc, StrAsc> result_type;
         typedef std::list<result_type> results_type; 
         virtual void on_complete(
            SnapshotRestorer *sender,
            outcome_type outcome,
            results_type const &results) = 0;
      };


      /**
       * Defines a component that can be used to restore a backup file to the LgrNet server.  In
       * order to use this compnent, the application must provide an object that inherits from class
       * SnapshotRestorerClient. It must then create an instance of this class, sert its parameters
       * including file_name and, optionally, clear_before_restore and comm_enabled, and must then
       * call one of the two versions of start.  When the server transaction is complete, the
       * component will call the component's on_complete() method.
       */
      class SnapshotRestorer:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the name and path of the file to be restored.  This must be a path that is
          * accessible to the server.
          */
         StrAsc file_name;

         /**
          * Set to true if all of the confifuration and cache files should be deleted before the
          * restore.
          */
         bool clear_before_restore;

         /**
          * Specifies the state of the global commEnabled LgrNet variable following the
          * transaction.  Defaults to use the state that was in the snapshot.
          */
      public:
         enum comm_enabled_state_type
         {
            comm_enabled_snapshot = -1,
            comm_enabled_disabled = 0,
            comm_enabled_enabled = 1
         };
      private:
         comm_enabled_state_type comm_enabled_state;
         
         /**
          * Specifies the application object that will receive notification when the transaction is
          * complete.
          */
         SnapshotRestorerClient *client;

         /**
          * Specifies the current state of this transaction.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

      public:
         /**
          * Constructor
          */
         SnapshotRestorer():
            client(0),
            state(state_standby),
            clear_before_restore(true),
            comm_enabled_state(comm_enabled_snapshot)
         { }

         /**
          * Destructor
          */
         virtual ~SnapshotRestorer()
         { finish(); }

         /**
          * @param file_name_ Sets the file name and path for the snapshot file to that is to be
          * restored.  This must be in a path that can be reached by the server and can include the
          * %a and %w sequences to reference the server's application and working directories.
          */
          void set_file_name(StrAsc const &file_name_)
         { if(state == state_standby)
               file_name = file_name_;
            else throw exc_invalid_state();
         }

         /**
          * @return Returns the name of the file to be restored.
          */
         StrAsc const &get_file_name() const
         { return file_name; }

         /**
          * @param clear_before_restore_ Set to true if the server configuration and cache files
          * should be deleted before the snapshot is restored.
          */
         void set_clear_before_restore(bool clear_before_restore_)
         {
            if(state == state_standby)
               clear_before_restore = clear_before_restore_;
            else
               throw exc_invalid_state();
         }

         /**
          * @return Returns true if the server's configuration and cache files are to be deleted
          * before the restoration.
          */
         bool get_clear_before_restore() const
         { return clear_before_restore; }

         /**
          * @param value Specifies the stae of changing the server's LgrNet commEnabled setting.
          */
         void set_comm_enabled_state(comm_enabled_state_type value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            comm_enabled_state = value;
         }

         /**
          * @return Returns the value of the comm_enabled_state property.
          */
         comm_enabled_state_type get_comm_enabled_state() const
         { return comm_enabled_state; }

         /**
          * Starts the server transaction.
          *
          * @param client_ Specifies the application object that will receive notification of
          * completion.
          *
          * @param router Specifies a newly created messaging router that will make a new connection
          * to the server.
          *
          * @param other_component Specifies a component that already has a connection to the
          * LoggerNet server that this component will share.
          */
         typedef SnapshotRestorerClient client_type;
         void start(
            client_type *client_,
            router_handle router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(file_name.length() == 0)
               throw std::invalid_argument("invalid file name");
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(router);
         }
         void start(
            client_type *client_,
            ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(file_name.length() == 0)
               throw std::invalid_argument("invalid file name");
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(other_component);
         }

         /**
          * Called to cancel the transaction and to return this component to a standby state.
          */
         virtual void finish()
         {
            client = 0;
            state = state_standby;
            ClientBase::finish();
         }

         /**
          * Describes the specified outcome and restore results.
          *
          * @param out Specifies the stream to which the results will be formatted.
          *
          * @param outcome Specifies the outcome code.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

      protected:
         /**
          * Oveeloads the base class version to handle handle and asynchronous messaage.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Overloads the base class version to start  the server transaction.
          */
         virtual void on_corabase_ready();

         /**
          * Overloads the base class version to handle an incoming LgrNet message.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         /**
          * Overloads the base class version to handle a connection failure.
          */
         virtual void on_corabase_failure(corabase_failure_type failure);

         /**
          * Overloads the base class version to handle a connection failure.
          */
         virtual void on_corabase_session_failure()
         { on_corabase_failure(corabase_failure_session); }
      };
   };
};
   

#endif
