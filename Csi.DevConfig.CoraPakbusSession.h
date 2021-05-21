/* Csi.DevConfig.CoraPakbusSession.h

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 09 January 2004
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_CoraPakbusSession_h
#define Csi_DevConfig_CoraPakbusSession_h

#include <list>
#include <map>
#include "Csi.DevConfig.SessionBase.h"
#include "Csi.DevConfig.LibraryManager.h"
#include "Csi.DevConfig.Setting.h"
#include "Cora.PbRouter.CommResourceManager.h"
#include "Cora.PbRouter.SettingsGetter.h"
#include "Cora.PbRouter.SettingsSetter.h"
#include "Cora.PbRouter.PakctrlMessageSender.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace CoraPakbusSessionHelpers
      {
         ////////////////////////////////////////////////////////////
         // class Transaction
         //
         // Defines a structure that maintains pending transaction information
         // for the cora PakBus based session.  
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
            typedef SharedPtr<Message> command_type;
            command_type command;

            ////////////////////////////////////////////////////////////
            // max_retry_count
            ////////////////////////////////////////////////////////////
            uint4 max_retry_count;

            ////////////////////////////////////////////////////////////
            // extra_timeout
            ////////////////////////////////////////////////////////////
            uint4 extra_timeout;

            ////////////////////////////////////////////////////////////
            // sender
            ////////////////////////////////////////////////////////////
            SharedPtr<Cora::PbRouter::PakctrlMessageSender> sender;

            ////////////////////////////////////////////////////////////
            // started
            ////////////////////////////////////////////////////////////
            bool started;

            ////////////////////////////////////////////////////////////
            // default constructor
            ////////////////////////////////////////////////////////////
            Transaction():
               client(0),
               max_retry_count(3),
               extra_timeout(1000),
               started(false)
            { }

            ////////////////////////////////////////////////////////////
            // initialisation constructor
            ////////////////////////////////////////////////////////////
            Transaction(
               TransactionClient *client_,
               command_type &command_,
               uint4 max_retry_count_,
               uint4 extra_timeout_):
               client(client_),
               command(command_),
               max_retry_count(max_retry_count_),
               extra_timeout(extra_timeout_),
               started(false)
            { }

            ////////////////////////////////////////////////////////////
            // copy constructor
            ////////////////////////////////////////////////////////////
            Transaction(Transaction const &other):
               client(other.client),
               command(other.command),
               max_retry_count(other.max_retry_count),
               extra_timeout(other.extra_timeout),
               started(other.started),
               sender(other.sender)
            { }

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            ~Transaction()
            { }

            ////////////////////////////////////////////////////////////
            // copy operator
            ////////////////////////////////////////////////////////////
            Transaction &operator =(Transaction const &other)
            {
               client = other.client;
               command = other.command;
               max_retry_count = other.max_retry_count;
               extra_timeout = other.extra_timeout;
               started = other.started;
               sender = other.sender;
               return *this;
            }
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class CoraPakbusSession.h
      //
      // This class defines a session object that uses the server's PbRouter
      // interface to work with device settings.  This component will also fake
      // the DevConfig messages for devices that do not support the new binary
      // settings messages.  
      ////////////////////////////////////////////////////////////
      class CoraPakbusSession:
         public SessionBase,
         public EventReceiver,
         public Cora::PbRouter::CommResourceManagerClient,
         public Cora::PbRouter::SettingsGetterClient,
         public Cora::PbRouter::SettingsSetterClient,
         public Cora::PbRouter::PakctrlMessageSenderClient 
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         CoraPakbusSession(
            SharedPtr<LibraryManager> &library_,
            SharedPtr<Cora::ClientBase> &base_component_,
            StrUni const &pakbus_router_name,
            uint2 node_address);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~CoraPakbusSession();

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
         // supports_reset
         ////////////////////////////////////////////////////////////
         virtual bool supports_reset();

         ////////////////////////////////////////////////////////////
         // map_model_no
         //
         // Responsible for mapping the model number to a value that is
         // expected by the library and application.
         ////////////////////////////////////////////////////////////
         virtual void map_model_no(StrAsc &model_no);

      protected:
         //@group methods overloaded from CommResourceManagerClient
         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         typedef Cora::PbRouter::CommResourceManager comm_manager_type;
         virtual void on_started(
            comm_manager_type *manager);

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            comm_manager_type *manager,
            CommResourceManagerClient::failure_type failure);
         //@endgroup

         //@group Methods overloaded from class SettingsGetterClient
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         typedef Cora::PbRouter::SettingsGetter strings_getter_type;
         virtual void on_complete(
            strings_getter_type *getter,
            SettingsGetterClient::outcome_type outcome,
            SettingsGetterClient::settings_type const &settings);
         //@endgroup

         //@group methods overloaded from class SettingsSetterClient
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         typedef Cora::PbRouter::SettingsSetter strings_setter_type;
         virtual void on_complete(
            strings_setter_type *setter,
            SettingsSetterClient::outcome_type outcome,
            uint4 settings_applied);
         //@endgroup

         //@group methods overloaded from class PakctrlMessageSenderClient
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         typedef Cora::PbRouter::PakctrlMessageSender binary_sender_type;
         virtual void on_complete(
            binary_sender_type *sender,
            PakctrlMessageSenderClient::outcome_type outcome,
            PakctrlMessageSenderClient::response_type &response,
            uint4 round_trip_time);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(SharedPtr<Event> &ev);
         
      private:
         ////////////////////////////////////////////////////////////
         // do_next_transaction
         ////////////////////////////////////////////////////////////
         void do_next_transaction();

         ////////////////////////////////////////////////////////////
         // do_get_settings
         ////////////////////////////////////////////////////////////
         typedef CoraPakbusSessionHelpers::Transaction transaction_type;
         typedef SharedPtr<transaction_type> tran_handle;
         void do_get_settings(tran_handle &transaction);

         ////////////////////////////////////////////////////////////
         // do_set_settings
         ////////////////////////////////////////////////////////////
         void do_set_settings(tran_handle &transaction);

         ////////////////////////////////////////////////////////////
         // do_start_control
         ////////////////////////////////////////////////////////////
         void do_start_control(tran_handle &transaction);

         ////////////////////////////////////////////////////////////
         // do_get_setting_fragment
         ////////////////////////////////////////////////////////////
         void do_get_setting_fragment(tran_handle &transaction);

         ////////////////////////////////////////////////////////////
         // do_set_setting_fragment
         ////////////////////////////////////////////////////////////
         void do_set_setting_fragment(tran_handle &transaction);

         ////////////////////////////////////////////////////////////
         // do_start_commit
         ////////////////////////////////////////////////////////////
         void do_start_commit(tran_handle &transaction);

         ////////////////////////////////////////////////////////////
         // do_close_session
         ////////////////////////////////////////////////////////////
         void do_close_session(tran_handle &transaction, byte outcome);

         ////////////////////////////////////////////////////////////
         // do_revert
         ////////////////////////////////////////////////////////////
         void do_revert(tran_handle &transaction);
         
         ////////////////////////////////////////////////////////////
         // do_refresh_session
         ////////////////////////////////////////////////////////////
         void do_refresh_session(tran_handle &transaction);

         ////////////////////////////////////////////////////////////
         // do_on_error
         ////////////////////////////////////////////////////////////
         void do_on_error();
         
      private:
         ////////////////////////////////////////////////////////////
         // transactions
         ////////////////////////////////////////////////////////////
         typedef std::list<tran_handle> transactions_type;
         transactions_type transactions;

         ////////////////////////////////////////////////////////////
         // comm_manager
         ////////////////////////////////////////////////////////////
         SharedPtr<comm_manager_type> comm_manager;

         ////////////////////////////////////////////////////////////
         // strings_getter
         ////////////////////////////////////////////////////////////
         SharedPtr<strings_getter_type> strings_getter;

         ////////////////////////////////////////////////////////////
         // strings_setter
         ////////////////////////////////////////////////////////////
         SharedPtr<strings_setter_type> strings_setter;

         ////////////////////////////////////////////////////////////
         // use_binary_transactions
         //
         // Set to true if this session has determined, based upon the strings
         // read from the device, whether the device supports the new binary
         // settings transactions.  This flag will determine whether the
         // responses to DevConfig transactions are simulated or directed to
         // the device. 
         ////////////////////////////////////////////////////////////
         bool use_binary_transactions;

         ////////////////////////////////////////////////////////////
         // cached_settings
         //
         // Cache of the setting objects supported by the device.  
         ////////////////////////////////////////////////////////////
         typedef std::pair<bool, SharedPtr<Setting> > setting_type;
         typedef std::list<setting_type> cached_settings_type;
         cached_settings_type cached_settings;

         ////////////////////////////////////////////////////////////
         // library
         ////////////////////////////////////////////////////////////
         SharedPtr<LibraryManager> library;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_loading_strings,
            state_strings_loaded,
            state_saving_strings
         } state;

         ////////////////////////////////////////////////////////////
         // base_component
         ////////////////////////////////////////////////////////////
         SharedPtr<Cora::ClientBase> base_component;

         ////////////////////////////////////////////////////////////
         // device_desc
         ////////////////////////////////////////////////////////////
         SharedPtr<DeviceDesc> device_desc;
      };
   };
};


#endif
