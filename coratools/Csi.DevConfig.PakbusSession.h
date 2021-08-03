/* Csi.DevConfig.PakbusSession.h

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 07 May 2012
   Last Change: Tuesday 08 May 2012
   Last Commit: $Date: 2012-05-08 13:51:31 -0600 (Tue, 08 May 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_PakbusSession_h
#define Csi_DevConfig_PakbusSession_h


#include "Csi.DevConfig.SessionBase.h"
#include "Csi.DevConfig.LibraryManager.h"
#include "Csi.DevConfig.Setting.h"
#include "Csi.PakBus.Router.h"
#include "Csi.PakBus.TranGetSettings.h"
#include "Csi.PakBus.TranSetSettings.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace PakbusSessionHelpers
      {
         /**
          * Defines a structure that maintains pending transaction information for a client
          * requested device configuration protocol transaction.
          */
         class Transaction
         {
         public:
            /**
             * Specifies the object that will get notified when this transaction is complete.
             */
            TransactionClient *client;

            /**
             * Specifies the command message for this transaction.
             */
            typedef SharedPtr<Message> command_type;
            command_type command;

            /**
             * Specifies the maximum number of times that the command should be retried.
             */
            uint4 max_retry_count;

            /**
             * Specifies the extra amount of time that should be used for this transaction.
             */
            uint4 extra_timeout;

            /**
             * Specifies that this transaction has been started.
             */
            bool started;

            /**
             * default constructor
             */
            Transaction():
               client(0),
               max_retry_count(3),
               extra_timeout(1000),
               started(false)
            { }

            /**
             * initialisation constructor
             *
             * @param client_  Specifies the client that will receive notification when complete.
             * @param command_ Specifies the command to be sent.
             * @param max_retry_count_ Specifies the number of times that the command should be
             * retried.
             * @param extra_timeout_ Specifies the extra amount of time in milliseconds the this
             * command should wait for a response.
             */
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

            /**
             * Copy constructor
             *
             * @param other the other transaction object to copy.
             */
            Transaction(Transaction const &other):
               client(other.client),
               command(other.command),
               max_retry_count(other.max_retry_count),
               extra_timeout(other.extra_timeout),
               started(other.started)
            { }

            /**
             * destructor
             */
            ~Transaction()
            { }

            /**
             * copy operator
             *
             * @param other The transaction to copy
             */
            Transaction &operator =(Transaction const &other)
            {
               client = other.client;
               command = other.command;
               max_retry_count = other.max_retry_count;
               started = other.started;
               extra_timeout = other.extra_timeout;
               return *this;
            }
         };
      };


      /**
       * This class defines a device configuration session object that uses an
       * application pakbus router (as opposed to the PakBus router in the
       * LoggerNet server) to work with device settings.  This class also
       * implements a layer that will emulate devconfig protocol with the
       * PakCtrl string based settings.
       */
      class PakbusSession:
         public SessionBase,
         public EventReceiver,
         PakBus::TranGetSettingsClient,
         PakBus::TranSetSettingsClient
      {
      public:
         /**
          * Constructs a session with a supplied PakBus router, library
          * manager, and PakBus address.
          *
          * @param library_  A reference to a device configuration library
          * manager.
          * @param router_  A reference to the PakBus router that we will use
          * for communication.
          * @param node_address  The address of the device with which we will
          * be communicating.
          */
         PakbusSession(
            SharedPtr<LibraryManager> &library_,
            SharedPtr<PakBus::Router> &router_,
            uint2 node_address_);

         /**
          * destructor
          */
         virtual ~PakbusSession();

         /**
          * Overloads the base class's version to add a device configuration
          * transaction .
          *
          * @param client   The object that will receive notification when the
          * transaction is complete.
          * @param command  The DevConfig message that is to be sent.
          * @param max_retry_count  The maximum number of times that the
          * command should be retried.
          * @param timeout_interval  The amount of time in milliseconds to wait
          * for a response.
          * @param tran_no  The transaction number.
          */
         virtual void add_transaction(
            TransactionClient *client,
            message_handle command,
            uint4 max_retry_count,
            uint4 timeout_interval,
            byte tran_no = 0);

         /**
          * Overloaded to indicate whether the device can be reset to factory
          * defaults.
          */
         virtual bool supports_reset();

         /**
          * Responsible for mapping a model number reported in the string
          * settings to the model numbers that should be used in the device
          * descriptions.
          */
         virtual void map_model_no(StrAsc &model_no);

         /**
          * Handles CSI events.
          */
         virtual void receive(SharedPtr<Event> &ev);

         /**
          * Handles the event where the string settings have been read.
          */
         virtual void on_complete(
            PakBus::TranGetSettings *getter,
            TranGetSettingsClient::outcome_type outcome,
            StrAsc const &content);

         /**
          * Handles the event where the string settings have been set.
          */
         virtual void on_complete(
            PakBus::TranSetSettings *setter,
            TranSetSettingsClient::outcome_type outcome,
            uint4 failed_offset);

         typedef PakbusSessionHelpers::Transaction transaction_type;
         typedef SharedPtr<transaction_type> tran_handle;

         /**
          * @return the reference to the PakBus router.
          */
         SharedPtr<PakBus::Router> &get_router()
         { return router; }

         /**
          * @return the timer object associated with the PakBus router.
          */
         SharedPtr<OneShot> &get_timer()
         { return router->get_timer(); }

         /**
          * @return the address of the destination device.
          */
         uint2 get_node_address() const
         { return node_address; }

         /**
          * Called to report an error for this session.
          */
         void do_on_error();

      private:
         /**
          * Performs the next operation that needs to be carried out.
          */
         void do_next_transaction();

         /**
          * Interprets the get settings command.
          *
          * @param transaction  the client transaction.
          */
         void do_get_settings(tran_handle &transaction);

         /**
          * Interprets the set settings command.
          *
          * @param transaction  the client transaction.
          */
         void do_set_settings(tran_handle &transaction);

         /**
          * Interprets the control transaction.
          *
          * @param transaction represents the devconfig transaction.
          */
         void do_start_control(tran_handle &transaction);

         /**
          * Interprets the get setting fragment transaction.
          *
          * @param transaction represents the state of the devconfig
          * transaction.
          */
         void do_get_setting_fragment(tran_handle &transaction);

         /**
          * Interprets the set setting fragment transaction.
          *
          * @param transaction represents the state of the devconfig
          * transaction.
          */
         void do_set_setting_fragment(tran_handle &transaction);

         /**
          * Starts the process of committing setting changes.
          *
          * @param transaction represents the state of the devconfig
          * transaction.
          */
         void do_start_commit(tran_handle &transaction);

         /**
          * Starts the process of closing the session.
          *
          * @param transaction represents the state of the devconfig
          * transaction
          * @param outcome  Specifies the outcome of the transaction.
          */
         void do_close_session(tran_handle &transaction, byte outcome);

         /**
          * Starts the process of reverting to factory defaults.
          *
          * @param transaction represents the state of the devconfig
          * transaction.
          */
         void do_revert(tran_handle &transaction);

         /**
          * Starts the process of refreshing the setting values from the
          * device.
          *
          * @param transaction represents the state of the transaction.
          */
         void do_refresh_session(tran_handle &transaction);

      private:
         /**
          * Keeps track of the transactions that are currently pending.
          */
         typedef std::list<tran_handle> transactions_type;
         transactions_type transactions;

         /**
          * Set to true if the session has determined, based upon strings read
          * from the device that the device supports the devconfig protocol.
          * This flag will determine whether the responses to devconfig
          * commands are simulated or directed to the device.
          */
         bool use_binary_transactions;

         /**
          * caches the set of string based settings that were read from the
          * device.
          */
         typedef SharedPtr<Setting> setting_handle;
         typedef std::pair<bool, setting_handle> setting_type;
         typedef std::list<setting_type> cached_settings_type;
         cached_settings_type cached_settings;

         /**
          * manages the collection of device descriptions.
          */
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

         /**
          * Caches the device description for this session.
          */
         SharedPtr<DeviceDesc> device_desc;

         /**
          * Reference to the PakBus router that we will use for communication.
          */
         SharedPtr<PakBus::Router> router;

         /**
          * Specifies the PakBus address of the device.
          */
         uint2 node_address;

         /**
          * Used to keep the link with the device alive.
          */
         SharedPtr<PakBus::PakBusTran> comm_manager;
      };
   };
};


#endif
