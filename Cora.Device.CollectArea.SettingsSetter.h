/* Cora.Device.CollectArea.SettingsSetter.h

   Copyright (C) 2002, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 19 December 2002
   Last Change: Friday 30 August 2019
   Last Commit: $Date: 2019-08-30 16:34:28 -0600 (Fri, 30 Aug 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_CollectArea_SettingsSetter_h
#define Cora_Device_CollectArea_SettingsSetter_h

#include "Cora.Device.DeviceBase.h"
#include "Cora.Setting.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         class SettingsSetter;
         
         
         /**
          * Defines the interface that must be implemented by the application to use this component,
          */
         class SettingsSetterClient: public Csi::InstanceValidator
         {
         public:
            /**
             * Called when the server transaction has been completed.  The application can also poll
             * the component for the outcome of individual settings.
             *
             * @param setter Specifies the component calling this method.
             *
             * @param outcome Specifies the outcome.
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
               outcome_invalid_area_name = 8,
               outcome_network_locked = 9
            };
            virtual void on_complete(SettingsSetter *setter, outcome_type outcome) = 0;
         };
         

         /**
          * Defines a component that can be used to change one or more settings for a device collect
          * area.  In order to use this component, the application must provide an object that
          * extends class SettingsSetterClient.  It should then create an instance of this
          * component, set the device name, set the collect area name, and add one or more setting
          * objects to be sent.  The application should then call one of the two versions of
          * start().  When the server transaction is complete, this component will call the client's
          * on_complete() method.
          */
         class SettingsSetter: public DeviceBase, public Csi::EventReceiver
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
             * @param collect_area_name Specifies the name of the collect area to target.
             */
            void set_collect_area_name(StrUni const &collect_area_name_);

            /**
             * @return Returns the collect area name.
             */
            StrUni const &get_collect_area_name() const
            { return collect_area_name; }

            /**
             * Adds a setting to the set that will be sent to the server.
             *
             * @param setting Specifies the setting to send.
             */
            typedef Csi::SharedPtr<Setting> setting_handle;
            void add_setting(setting_handle setting);

            /**
             * Clears the list of settings to send.
             */
            void clear_settings();

            /**
             * @return Returns an iterator to the start of the settings list.,
             */
            typedef std::list<setting_handle> settings_type;
            typedef settings_type::iterator iterator;
            typedef settings_type::const_iterator const_iterator;
            iterator begin() { return settings.begin(); }
            const_iterator begin() const { return settings.begin(); }

            /**
             * @return Returns an iterator beyond the end of the settings list.
             */
            iterator end() { return settings.end(); }
            const_iterator end() const { return settings.end(); }

            /**
             * @return Returns the number of settings to change.
             */
            settings_type::size_type size() const { return settings.size(); }

            /**
             * Called to start the server transction.
             *
             * @param client_ Specifies the client object provided by the application.
             *
             * @param router Specifies a router that has been newly created and not yet connected.
             *
             * @param other_component Specifies a component that already has a connection to the
             * server that can be shared.
             */
            typedef SettingsSetterClient client_type;
            void start(
               client_type *client_,
               router_handle &router);
            void start(
               client_type *client_,
               ClientBase *other_component);

            /**
             * Called to stop the server transaction and to release any resources.
             */
            virtual void finish();

            /**
             * Called to format the outcome code for this component.
             *
             * @param out Specifies the stream to which the outcome will be formatted.
             *
             * @param outcome Specifies the outcome to format.
             */
            static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

         protected:
            /**
             * Overloads the base class version to handle the event where device session has been
             * cloned and is ready.
             */
            virtual void on_devicebase_ready();

            /**
             * Overloads the base class version to handle a failure of the device session.
             */
            virtual void on_devicebase_failure(devicebase_failure_type failure);
            virtual void on_devicebase_session_failure();

            /**
             * Overloads the base class version to handle an incoming server message.
             */
            virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);

            /**
             * Overloads the base class version to asynchronously deliver the outcome to the client.
             */
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         private:
            /**
             * Specifies the application's client object.
             */
            client_type *client;

            /**
             * Specifies the state of this component.
             */
            enum state_type
            {
               state_standby,
               state_delegate,
               state_active
            } state;

            /**
             * Specifies the collect area name.
             */
            StrUni collect_area_name;

            /**
             * Specifies the collection of settings to send.
             */
            settings_type settings;
         };
      };
   };
};


#endif
