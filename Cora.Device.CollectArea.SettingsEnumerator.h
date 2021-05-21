/* Cora.Device.CollectArea.SettingsEnumerator.h

   Copyright (C) 2000, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 31 October 2000
   Last Change: Friday 18 October 2019
   Last Commit: $Date: 2019-10-18 17:46:39 -0600 (Fri, 18 Oct 2019) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_CollectArea_SettingsEnumerator_h
#define Cora_Device_CollectArea_SettingsEnumerator_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "Cora.SettingFactory.h"
#include "Cora.SettingHandler.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         class SettingsEnumerator;

         /**
          * Defines the interface that the application must implement in order to use the
          * SettingsEnumerator component type.
          */
         class SettingsEnumeratorClient: public Csi::InstanceValidator
         {
         public:
            /**
             * Called when the LoggerNet transaction has been started and all of the initial setting
             * values have been received.
             *
             * @param sender Specifies the component that called this method.
             */
            virtual void on_started(SettingsEnumerator *sender)
            { }

            /**
             * Called when the loggernet transaction has failed.
             *
             * @param sender Specifies the component that is calling this method.
             *
             * @param failure Specifies the type of failure.
             */
            enum failure_type
            {
               failure_unknown  = 0,
               failure_connection_failed = 1,
               failure_invalid_logon = 2,
               failure_server_security_blocked = 3,
               failure_device_name_invalid = 4,
               failure_collect_area_name_invalid = 5,
            };
            virtual void on_failure(SettingsEnumerator *sender, failure_type failure) = 0;

            /**
             * Called to report the initial value of a collect area setting and to report when that
             * setting has been changed.
             *
             * @param sender Specifies the component that is calling this method.
             *
             * @param setting Specifies the setting.
             */
            virtual void on_setting_changed(
               SettingsEnumerator *enumerator,
               Csi::SharedPtr<Setting> &setting)
            { }
         };


         /**
          * Defines a component that can be used to monitor the values of settings for a given
          * collect area on a given datalogger.  In order to use this component, the application
          * must provide a client object that is derived from class SettingsEnumeratorClient.  It
          * should then create an instance of this class, set its properties including device name
          * and collect area name, and call one of the two versions of start().  As the server
          * transaction is starting, it will call the client's on_setting_changed() method for each
          * setting that it processes.  It will then call the client's on_started() method.
          * Thereafter, the component will call the client's on_setting_changed() method each time
          * that a change is detected.  If the LoggerNet transaction fails, the client's
          * on_failure() method will be called.
          */
         class SettingsEnumerator:
            public DeviceBase,
            public SettingHandler,
            public Csi::EvReceiver
         {
         private:
            /**
             * Specifies the name of the collect area to monitor.
             */
            StrUni collect_area_name;

            /**
             * Specifies the object that will create settings objects.
             */
            Csi::SharedPtr<SettingFactory> setting_factory;

         public:
            /**
             * Constructor
             */
            SettingsEnumerator();

            /**
             * Destructor
             */
            virtual ~SettingsEnumerator();

            /**
             * @param collect_area_name_ Specifies the name of the collect area to monitor.
             */
            void set_collect_area_name(StrUni const &collect_area_name_);

            /**
             * @return Returns the name of the collect area being monitored.
             */
            StrUni const &get_collect_area_name() const
            { return collect_area_name; }

            /**
             * @param setting_factory_ Specifies the object responsible for creating setting
             * objects.
             */
            void set_setting_factory(Csi::SharedPtr<SettingFactory> setting_factory_);
            void set_setting_factory(SettingFactory *setting_factory_)
            { set_setting_factory(Csi::SharedPtr<SettingFactory>(setting_factory_)); }

            /**
             * @return Returns the object responsible for creating setting objects.
             */
            Csi::SharedPtr<SettingFactory> &get_setting_factory()
            { return setting_factory; }

            /**
             * Called to start the server transaction.
             *
             * @param client_ Specifies the application's client object.
             *
             * @param router Specifies a messaging router that has not yet been connected.
             *
             * @param other_component Specifies another client that can share its connection with
             * this component.
             */
            typedef SettingsEnumeratorClient client_type;
            void start(client_type *client_, router_handle &router);
            void start(client_type *client_, ClientBase *other_component);

            /**
             * Overloads the base class version to release resources for this component and to
             * return it to a standby state.
             */
            virtual void finish();

            /**
             * Formats the specified failure code to the specified stream.
             */
            static void describe_failure(std::ostream &out, client_type::failure_type failure);
            
            /**
             * Overloads the base class version to handle asynch events.
             */
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         protected:
            /**
             * Overloads the base class version to handle an incoming message.
             */
            virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);

            /**
             * Overloads the base class version to handle a device failure.
             */
            virtual void on_devicebase_failure(devicebase_failure_type failure);
            virtual void on_devicebase_session_failure()
            { on_devicebase_failure(devicebase_failure_session); }

            /**
             * Overloads the base class version to handle the server transaction.
             */
            virtual void on_devicebase_ready();

            /**
             * Handles a setting change event.
             */
            virtual void on_setting_read(Csi::SharedPtr<Setting> &setting, uint4 context_token);

         private:
            /**
             * Handles a setting enum notification.
             */
            void on_enum_not(Csi::Messaging::Message *message);

            /**
             * Handles a stopped notification.
             */
            void on_stopped_not(Csi::Messaging::Message *message);

         private:
            /**
             * Specifies the state of this component.
             */
            enum state_type
            {
               state_standby,
               state_delegate,
               state_before_active,
               state_active,
            } state;

            /**
             * Specifies the client object.
             */
            SettingsEnumeratorClient *client;
         };
      };
   };
};

#endif
