/* Csi.DevConfig.SettingsManager.h

   Copyright (C) 2003, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 18 December 2003
   Last Change: Wednesday 21 March 2018
   Last Commit: $Date: 2018-03-21 11:26:54 -0600 (Wed, 21 Mar 2018) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingsManager_h
#define Csi_DevConfig_SettingsManager_h

#include "Csi.DevConfig.Setting.h"
#include "Csi.DevConfig.SessionBase.h"
#include "Csi.DevConfig.LibraryManager.h"
#include "Csi.Expression.TokenFactory.h"
#include <set>


namespace Csi
{
   namespace DevConfig
   {
      /**
       * Defines the interface that must be implemented by an application object that uses the
       * SettingsManager component.
       */
      class SettingsManager;
      class SettingsManagerClient: public InstanceValidator
      {
      public:
         /**
          * Called when there has been a failure in loading settings from the device.
          *
          * @param manager Specifies the component sending this event.
          *
          * @param failure Specifies the type of failure that has occurred.
          */
         enum load_failure_type
         {
            load_unknown_error = 0,
            load_unknown_device_type = 1,
            load_unknown_catalog = 2,
            load_comms_failure = 3,
            load_security_failure = 4,
            load_locked_failure = 5,
            load_invalid_device_desc_failure = 6
         };
         virtual void on_load_failure(
            SettingsManager *manager, load_failure_type failure)
         { }

         /**
          * Writes a description of the specified failure to the specified stream.
          *
          * @param out Specifies the stream to which the description will be written.
          *
          * @param failure Specifies the failure to describe.
          */
         static void describe_load_failure(std::ostream &out, load_failure_type failure);

         /**
          * Called when the component has first been started and the initial set of setting values
          * have been loaded.  Also called when a client has been added to an already started
          * manager.
          *
          * @param manager Specifies the manager sending this event.
          */
         virtual void on_started(
            SettingsManager *manager)
         { }

         /**
          * Called when a control transaction to revert settings to factory defaults has completed.
          *
          * @param manager Specifies the component sending this event.
          */
         virtual void on_settings_reverted(
            SettingsManager *manager)
         { }

         /**
          * Called when an attempt to commit changes haas failed.
          *
          * @param manager Specifies the component sending this event.
          *
          * @param failure Specifies the type of failure that occurred.
          */
         enum commit_failure_type
         {
            commit_unknown_error = 0,
            commit_security_failure = 1,
            commit_comms_failure = 2,
            commit_locked_failure = 3,
            commit_setting_not_recognised = 4,
            commit_setting_malformed = 5,
            commit_setting_read_only = 6,
            commit_out_of_memory = 7,
            commit_system_error = 8
         };
         virtual void on_commit_failure(
            SettingsManager *manager, commit_failure_type falure)
         { }

         /**
          * Writes a description of the specified commit failure to the specified stream.
          *
          * @param out Specifies the stream to which the failure will be written.
          *
          * @param failure Specifies the failure to format.
          */
         static void describe_commit_failure(std::ostream &out, commit_failure_type failure);

         /**
          * Called when settings have been successfully committed or cancelled.
          *
          * @param manager Specifies the device reporting this event.
          *
          * @param cancelled Set to true if the settings were cancelled.
          */
         virtual void on_settings_committed(
            SettingsManager *manager,
            bool cancelled)
         { }

         /**
          * Called when a wifi scan has been completed.
          *
          * @param sender Specifies the settings manager sending this event.
          */
         virtual void on_wifi_scan_complete(SettingsManager *sender)
         { }
      };


      /**
       * Defines an object that manages the list of settings for a given device as well as the state
       * of a devconfig transaction with the device.  This object will work with a session object to
       * obtain the list of settings and to manage that list in behalf of the application.
       */
      class SettingsManager: public TransactionClient
      {
      public:
         /**
          * Constructor
          *
          * @param library_ Specifies the library manager that keeps track of
          * available device descriptions.
          *
          * @param for_factory_ Set to true if the settings should be loaded in a context where
          * certain marked settings should be considered read/write.
          */
         SettingsManager(
            SharedPtr<LibraryManager> library_,
            bool for_factory_ = false);

         /**
          * Destructor
          */
         virtual ~SettingsManager();

         /**
          * Adds an application object that will receive state notifications from this manager.
          *
          * @param client Specifies the client to add.
          */
         typedef SettingsManagerClient client_type;
         void add_client(client_type *client);

         /**
          * Removes a client from the list of application delegates.
          */
         void remove_client(client_type *client);

         /**
          * Removes all clients from the list of application delegates.
          */
         void remove_all_clients()
         { clients.clear(); }
         
         /**
          * Starts the devconfig session with the device using the specified session and logger
          * security code.
          *
          * @param session_ Specifies the session object for this manager.
          *
          * @param security_code_ Specifies the security code that should be used.
          */
         virtual void start(
            SharedPtr<SessionBase> session_,
            uint2 security_code_ = 0);

         /**
          * @return Returns true if there are settings that must be committed.
          */
         virtual bool needs_to_commit();

         /**
          * Sets the flag that indicates that settings must be committed.
          */
         void set_needs_to_commit();
         
         /**
          * Starts the process of sending any pending setting changes and committing these changes
         U* to the device.
         */
         virtual void start_commit_changes();

         /**
          * @return Returns true if the device supports factory defaults.
          */
         bool can_revert_to_defaults();

         /**
          * Starts the process of requesting factory defaults from the device.
          */
         virtual void start_revert_to_defaults();

         /**
          * Starts the process of cancelling the session and/or any pending changes.
          *
          * @param should_reboot Set to true if the device should perform a reboot when the control
          * transaction is complete.
          */
         virtual void start_cancel_changes(bool should_reboot = true);

         /**
          * Starts a control transaction to scan for wifi networks.
          */
         virtual void start_scan_wifi();

         /**
          * Starts the process of renewing all setting values from the device.
          */
         virtual void start_refresh();

         /**
          * @return Returns the device type code.
          */
         uint2 get_device_type() const
         { return device_type; }

         /**
          * @return Returns the major version number that was reported by the device.
          */
         byte get_major_version() const
         { return major_version; }

         /**
          * @return Returns the model number from the device description.
          *
          * @param use_presented=false Set to true if the user facing model should be returned.
          */
         StrAsc const &get_model_no(bool use_presented = false) const;

         /**
          * @return Returns the minor version number reported by the device.
          */
         byte get_minor_version() const
         { return minor_version; }

         /**
          * @return Returns a reference to the library of device descriptions.
          */
         SharedPtr<LibraryManager> &get_library()
         { return library; }

         /**
          * @return Returns the setting catalog was chosen based upon the device's reported major
          * version.
          */
         SharedPtr<SettingCatalog>  &get_catalog()
         { return catalog; }

         /**
          * @return Returns the iterator to the start of the collection of settings.
          */
         typedef SharedPtr<Setting> value_type;
         typedef std::list<value_type> settings_type;
         typedef settings_type::iterator iterator;
         typedef settings_type::const_iterator const_iterator;
         iterator begin()
         { return settings.begin(); }
         const_iterator begin() const
         { return settings.begin(); }

         /**
          * @return Returns the iterator beyond the end of the settings collection.
          */
         iterator end()
         { return settings.end(); }
         const_iterator end() const
         { return settings.end(); }

         /**
          * @return Returns true if there are no settings.
          */
         bool empty() const
         { return settings.empty(); }

         /**
          * @return Returns the number of settings.
          */
         typedef settings_type::size_type size_type;
         size_type size() const
         { return settings.size(); }

         /**
          * @return Returns the iterator to the first setting that matches the parameter criteria.
          *
          * @param setting_name Specifies the name of the setting to search for.
          *
          * @param setting_id Specifies the identifier for the setting.
          *
          * @param common_name Specifies the common name for the setting.
          */
         iterator find_setting(StrAsc const &setting_name);
         iterator find_setting(uint2 setting_id);
         iterator find_setting_common(StrAsc const &common_name);

         /**
          * @return Returns the device description.
          */
         LibraryManager::value_type &get_device_desc()
         { return device_desc; }

         /**
          * @return Returns a copy of the setting identified by either its name or setting
          * identifier.  Returns null if no setting was found to clone.
          *
          * @param setting_name Specifies the name of the setting to find.
          *
          * @param setting_id Specifies the identifier for the setting to find.
          */
         Setting *clone_setting(StrAsc const &setting_name);
         Setting *clone_setting(uint2 setting_id);

         /**
          * @return Returns the setting reference for the specified identifier or null if there is
          * no such setting.
          */
         value_type get_setting(uint2 setting_id);

         /**
          * Copies the components from the specified setting to that managed by this manager.
          *
          * @param setting Specifies the setting to copy.
          */
         void copy_setting(Setting *setting);

         /**
          * @returns Returns the description text for the setting and component associated with the
          * specified names
          *
          * @param setting_name Specifies the name of the setting to find.
          *
          * @param comp_name="" Specifies the name of the component to find.  Will use the first
          * component if this value is an empty string.
          */
         StrAsc const &get_component_desc_text(
            StrAsc const &setting_name,
            StrAsc const &comp_name = "");

         /**
          * @return Returns true if the internal state of this manager indicates that settings have
          * been loaded and the session is active.
          */
         bool is_loaded() const;

         /**
          * @return Returns the session used by this manager.
          */
         SharedPtr<SessionBase> get_session()
         { return session; }

         /**
          * @return Returns the security code.
          */
         uint2 get_security_code() const
         { return security_code; }

         /**
          * @return Returns true if any of the settings specify tab names.
          */
         bool should_use_tabs() const;

         /**
          * @return Returns true if the device description indicates that factory defaults are
          * supported.
          */
         bool supports_factory_defaults() const;

         /**
          * @return Returns the list of wanring messages returned after a commit attempt.
          */
         typedef std::list<StrAsc> commit_warnings_type;
         commit_warnings_type const &get_commit_warnings() const
         { return commit_warnings; }

         /**
          * Evaluates the validation expressions for the setting components.  Any resulting messages
          * will be posted to the errors list.
          *
          * @param errors Specifies the collection of error messages that were generated.
          */
         typedef commit_warnings_type validate_errors_type;
         bool validate(validate_errors_type &errors);

         /**
          * @return Returns true if there is a cancel operation pending.
          */
         bool get_cancel_pending() const;
         
      protected:
         // @group: methods overloaded from TransactionClient

         /**
          * Overloads the base class version to handle the completion of a transaction.
          */
         virtual void on_complete(
            message_handle &command,
            message_handle &response);

         /**
          * Overloads the base class version to handle the failure of a transaction.
          */
         virtual void on_failure(
            message_handle &command,
            failure_type failure);

         /**
          * Overloads the base class version to handle the sending event.
          */
         virtual void on_sending_command(
            message_handle &command);
         
         // @endgroup:

      private:
         // @group: DevConfig protocol message handlers

         /**
          * Handles the get settings acknowledgement message.
          */
         void on_get_settings_ack(
            message_handle &command,
            message_handle &response);

         /**
          * Handles the get setting fragment acknowledgement.
          */
         void on_get_setting_fragment_ack(
            message_handle &command,
            message_handle &response);

         /**
          * Handles the set setting acknowledgement.
          */
         void on_set_settings_ack(
            message_handle &command,
            message_handle &response);

         /**
          * Handles the set setting fragment acknowledgement.
          */
         void on_set_setting_fragment_ack(
            message_handle &command,
            message_handle &response);

         /**
          * Handles the control acknowledgement message.
          */
         void on_control_ack(
            message_handle &command,
            message_handle &response);
         
         // @endgroup:

         /**
          * Performs the work associated with a load completion event.
          */
         void do_load_complete();

         /**
          * Performs the work of loading the next fragment.
          */
         void do_next_load_fragment();

         /**
          * Performs the work of reporting a load failure.
          */
         typedef client_type::load_failure_type load_failure_type;
         void do_load_error(load_failure_type failure);

         /**
          * Starts the next commit transaction.
          */
         void do_next_commit();
         
         /**
          * Performs the work of reporting completion of a commit or cancel.
          */
         void do_commit_complete(bool cancelled);

         /**
          * Performs the work of reporting a commit failure.
          */
         typedef client_type::commit_failure_type commit_failure_type;
         void do_commit_failure(commit_failure_type failure);

         /**
          * Performs the work of rolling back changes for a reboot.
          */
         void do_rollback_for_reboot();

         /**
          * Performs the work of requesting a reboot.
          */
         void do_reboot();

         /**
          * Reports that wifi scanning has been started.
          */
         void do_report_wifi_scan();

      protected:
         /**
          * Specifies the current state of this manager.
          */
         enum state_type
         {
            state_standby,
            state_loading_settings,
            state_saving_settings,
            state_reverting_to_defaults,
            state_cancelling,
            state_active,
            state_reboot_rollback,
            state_reboot_commit
         } state;

         /**
          * Specifies the collecion of application objects that will receive notifications from this
          * component.
          */
         typedef std::list<client_type *> clients_type;
         clients_type clients;

         /**
          * Reference to the devconfig session object.
          */
         SharedPtr<SessionBase> session;

         /**
          * Reference to the object that manages the collection of device descriptions.
          */
         SharedPtr<LibraryManager> library;

         /**
          * Reference to the setting catalog used for this device's major version.
          */
         SharedPtr<SettingCatalog> catalog;

         /**
          * Reference to the device description.
          */
         LibraryManager::value_type device_desc;

         /**
          * Specifies the device type code reported by the device.
          */
         uint2 device_type;

         /**
          * Specifies the major version that was reported by the device.
          */
         byte major_version;

         /**
          * Specifies the minor version that was reported by the device.
          */
         byte minor_version;

         /**
          * Specifies the collection of settings that have been loaded from the device.
          */
         settings_type settings;

         /**
          * Specifies the collection of settings that must be sent to the device during a commit
          * operation.  These will be streamed out in one or more set setting and set setting
          * fragment command messages.
          */
         typedef SharedPtr<SettingDesc> setting_desc_handle;
         typedef std::pair<setting_desc_handle, message_handle> queue_element;
         typedef std::list<queue_element> queue_type;
         queue_type commit_queue;

         /**
          * Specifies the collection of partial setting fragments that we are waiting to complete.
          */
         queue_type get_queue;

         /**
          * Set to true if changes have been made that require commits.
          */
         bool marked_as_changed;

         /**
          * Specifies the security code.
          */
         uint2 security_code;

         /**
          * Specifies the collection of warning messages accumulated during the commit.
          */
         commit_warnings_type commit_warnings;

         /**
          * Specifies the expression token factory used for validation expressions.
          */
         SharedPtr<Expression::TokenFactory> validate_token_factory;

         /**
          * Set to true if specially marked settings should be considered read/write.
          */
         bool for_factory;

         /**
          * Specifies the collection of setting identifiers that have been received.
          */
         std::set<uint2> received_identifiers;
      };
   };
};


#endif
