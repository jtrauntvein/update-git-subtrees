/* Cora.Device.SettingsEnumerator.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_SettingsEnumerator_h
#define Cora_Device_SettingsEnumerator_h


#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "Cora.SettingFactory.h"
#include "Cora.SettingHandler.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class SettingsEnumerator;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class SettingsEnumeratorClient
      //
      // Declares the interface for a client to a SettingsEnumerator object.
      //////////////////////////////////////////////////////////// 
      class SettingsEnumeratorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called after the settings enumerate (or settings get transaction) has been started and
         // the initial set of settings has been recieved.
         //////////////////////////////////////////////////////////// 
         virtual void on_started(SettingsEnumerator *enumerator)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called if the settings enumeration has failed for any reason
         //////////////////////////////////////////////////////////// 
         enum failure_type
         {
            failure_unknown  = 0,
            failure_connection_failed = 1,
            failure_invalid_logon = 2,
            failure_server_security_blocked = 3,
            failure_device_name_invalid = 4,
         };
         virtual void on_failure(
            SettingsEnumerator *enumerator,
            failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_setting_changed
         //
         // Called when a setting has been changed. This method can also be invoked several times
         // when the transaction is first coming up.
         //////////////////////////////////////////////////////////// 
         virtual void on_setting_changed(
            SettingsEnumerator *enumerator,
            Csi::SharedPtr<Setting> &setting)
         { }

         ////////////////////////////////////////////////////////////
         // on_setting_overridden
         //
         // called when a setting has been overridden
         //////////////////////////////////////////////////////////// 
         virtual void on_setting_overridden(
            SettingsEnumerator *enumerator,
            Csi::SharedPtr<Setting> &setting)
         { }

         ////////////////////////////////////////////////////////////
         // on_override_stopped
         //
         // Called when notification that a settings override transaction has been cancelled for the
         // device has been received.
         //////////////////////////////////////////////////////////// 
         virtual void on_override_stopped(
            SettingsEnumerator *enumerator)
         { }
      };

      ////////////////////////////////////////////////////////////
      // class SettingsEnumerator
      //
      // Defines an object that encapsulates device settings for all versions of the server. A
      // client should use this class by creating an instance of it, setting the appropriate
      // properties, and calling start().
      //
      // While the transaction is starting, the client will recieve several settings changed events
      // (one for each setting supported by the device) and possibly some settings overridden events
      // (if an override transaction is active on the server and the server version is greater than
      // or equal to 1.2.1. Finally, the client will recieve a on_settings_enum_started()
      // event. From that point on, further changed and overriden events will be posted when they
      // are detected.
      //
      // To stop the transaction, the client should invoke the finish() method. This will cause all
      // sessions and resources claimed by this object to be released.
      //////////////////////////////////////////////////////////// 
      class SettingsEnumerator:
         public DeviceBase,
         public Csi::EvReceiver,
         public SettingHandler
      {
      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // setting_factory
         //
         // Responsible for allocating setting objects as notifications regarding changes and
         // overrides are recieved. This is defined as a property so that a client can change the
         // classes of settings that are allocated for its own purpose or it can limit the set of
         // setting identifiers that are supported.
         //
         // This property is set by default to be an instance of Cora::Device::DeviceSettingsFactory
         // by the constructor. it can be changed by calling set_setting_factory()
         //////////////////////////////////////////////////////////// 
         Csi::SharedPtr<SettingFactory> setting_factory;
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         SettingsEnumerator();
         
         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~SettingsEnumerator();

         ////////////////////////////////////////////////////////////
         // set_setting_factory
         //
         // Allows the client to specify a setting factory that is different from the default type
         // (Cora::Device::DeviceSettingFactory) used. This method will not succeed if the object is
         // not in a standby state.
         //////////////////////////////////////////////////////////// 
         void set_setting_factory(Csi::SharedPtr<SettingFactory> factory);
         void set_setting_factory(SettingFactory *factory);
         
         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef SettingsEnumeratorClient client_type;
         void start(
            SettingsEnumeratorClient *client,
            router_handle &router);
         void start(
            SettingsEnumeratorClient *client,
            ClientBase *othera_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // is_setting_ignored
         //
         // Returns true if the specified identifier is one of the settings that should be ignored.
         // If the component is in a standby state an exc_invalid_state object will be thrown. 
         ////////////////////////////////////////////////////////////
         bool is_setting_ignored(uint4 setting_id) const;

         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(
            std::ostream &out, client_type::failure_type failure);

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         //////////////////////////////////////////////////////////// 
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         //////////////////////////////////////////////////////////// 
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_devicebase_session_failure();

         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         //////////////////////////////////////////////////////////// 
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // on_setting_read
         //////////////////////////////////////////////////////////// 
         virtual void on_setting_read(
            Csi::SharedPtr<Setting> &setting,
            uint4 context_token);

      private:
         ////////////////////////////////////////////////////////////
         // on_settings_enum_status_not
         ////////////////////////////////////////////////////////////
         void on_settings_enum_status_not(Csi::Messaging::Message *message);
         
         ////////////////////////////////////////////////////////////
         // on_settings_get_not
         //////////////////////////////////////////////////////////// 
         void on_settings_enum_not(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_settings_get_not
         //////////////////////////////////////////////////////////// 
         void on_settings_enum_override_not(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_settings_get_not
         //////////////////////////////////////////////////////////// 
         void on_settings_enum_override_stop_not(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_settings_get_not
         //////////////////////////////////////////////////////////// 
         void on_settings_enum_stopped_not(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_settings_get_not
         //////////////////////////////////////////////////////////// 
         void on_settings_get_not(Csi::Messaging::Message *message);
         
      private:
         ////////////////////////////////////////////////////////////
         // state
         //
         // Records the current state of this object
         //////////////////////////////////////////////////////////// 
         enum state_type
         {
            state_standby,
            state_delegate,
            state_before_active,
            state_active,
         } state;

         ////////////////////////////////////////////////////////////
         // client
         //
         // The object that will receive event notifications
         //////////////////////////////////////////////////////////// 
         SettingsEnumeratorClient *client;

         ////////////////////////////////////////////////////////////
         // ignored_settings
         //
         // The list of setting identifiers that should be marked as ignored.  
         ////////////////////////////////////////////////////////////
         std::map<uint4, bool> ignored_settings;
      }; 
   };
};

#endif
