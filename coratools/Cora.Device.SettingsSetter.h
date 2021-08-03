/* Cora.Device.SettingsSetter.h

   Copyright (C) 2002, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 19 December 2002
   Last Change: Friday 16 March 2018
   Last Commit: $Date: 2019-10-21 07:50:04 -0600 (Mon, 21 Oct 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_SettingsSetter_h
#define Cora_Device_SettingsSetter_h

#include "Cora.Device.DeviceBase.h"
#include "Cora.Setting.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class SettingsSetter;
      //@endgroup


      /**
       * Defines the interface that must be implemented by application objects that use the
       * SettingsSetter component.
       */
      class SettingsSetterClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Handles the event that reports that the settings operation has completed.
          *
          * @param setter Specifies the component reporting this event.
          *
          * @param outcome Specifies the outcome of the operation.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_some_errors = 2,
            outcome_invalid_logon = 3,
            outcome_invalid_device_name = 4,
            outcome_session_failed = 5,
            outcome_security_blocked = 6,
            outcome_unsupported = 7,
            outcome_network_locked = 8
         };
         virtual void on_complete(
            SettingsSetter *setter,
            outcome_type outcome) = 0;
      };


      /**
       * Defines a component that can be used to set the values of one or more device settings in
       * the LoggerNet server.  In order to use this component, an application must provide an
       * object that implements the SettingsSetterClient interface.  It should then create an
       * instance of this component, set its device name, and add one or more setting objects to be
       * set.  It should then call one of the two versions of start().  When the transaction is
       * complete, the client's on_complete() method will be called.
       */
      class SettingsSetter:
         public DeviceBase,
         public Csi::EventReceiver
      {
      public:
         /**
          * Constructor
          */
         SettingsSetter();

         /**
          * Destructor
          */
         virtual ~SettingsSetter();

         /**
          * Adds a setting to the collection that will be sent.
          *
          * @param setting Specifies the setting to be sent.
          */
         typedef Csi::SharedPtr<Setting> setting_handle;
         void add_setting(setting_handle setting);

         /**
          * Clears the list of settings to be sent.
          */
         void clear_settings();

         /**
          * @return Returns an iterator to the first of the set of settings to be sent.
          */
         typedef std::list<setting_handle> settings_type;
         typedef settings_type::iterator iterator;
         typedef settings_type::const_iterator const_iterator;
         iterator begin() { return settings.begin(); }
         const_iterator begin() const { return settings.begin(); }

         /**
          * @return Returns an iterator beyond the end of the list of settings to be sent.
          */
         iterator end() { return settings.end(); }
         const_iterator end() const { return settings.end(); }

         /**
          * @return Returns the number of settings to send.
          */
         settings_type::size_type size() const { return settings.size(); }

         /**
          * Starts the server transaction.
          *
          * @param client_ Specifies the application object that implements the on_complete()
          * method.
          *
          * @param router Specifies a messaging router that has not been previously connected.
          *
          * @param other_component Specifies a component that already has a connection to the
          * server.
          */
         typedef SettingsSetterClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         /**
          * Cancels any pending transactions and returns this component to an idle state.
          */
         virtual void finish();

         /**
          * Writes a description of the specified outcome to the given stream.
          *
          * @param out Specifies the stream to write.
          *
          * @param outcome Specifies the outcome to describe.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

      protected:
         /**
          * Overloads the base class to handle the event where the session with the device is
          * ready.
          */
         virtual void on_devicebase_ready();

         /**
          * Overloads the base class version to handle the event where a failure has occurred.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         /**
          * Overloads the base class version to handle a session failure with the device.
          */
         virtual void on_devicebase_session_failure();

         /**
          * Overloads the base class version to handle an incoming message.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         /**
          * Overloads the base class version to handle asynchonous events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         /**
          * Specifies the application client object.
          */
         client_type *client;

         /**
          * Specifies the current state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Specifies the collection of settings to send.
          */
         settings_type settings;
      };
   };
};


#endif
