/* Cora.LgrNet.SettingsEnumerator.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 13 September 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2019-09-11 18:20:24 -0600 (Wed, 11 Sep 2019) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_LgrNet_SettingsEnumerator_h
#define Cora_LgrNet_SettingsEnumerator_h


#include "Cora.ClientBase.h"
#include "Csi.InstanceValidator.h"
#include "CsiEvents.h"
#include "Cora.SettingFactory.h"
#include "Cora.SettingHandler.h"


namespace Cora
{
   namespace LgrNet
   {
      class SettingsEnumerator;


      /**
       * Defines the interface that must be implemented by the application in order to receive
       * notifications from the settings enumerator.
       */
      class SettingsEnumeratorClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when all of the initial setting values have been received from the server.
          *
          * @param sender Specifies the component calling this method.
          */
         virtual void on_started(SettingsEnumerator *sender)
         { }
         
         /**
          * Called when the enumerator has failed.
          *
          * @param sender Specifies the component that is calling this method.
          *
          * @param failure Specifies a code that identifies the reason for the failure.
          */
         enum failure_type
         {
            failure_unknown = 0,
            failure_session_failed = 1,
            failure_invalid_logon = 2,
            failure_unsupported = 3,
            failure_server_security_blocked = 4,
         };
         virtual void on_failure(SettingsEnumerator *sender, failure_type failure) = 0;

         /**
          * Called when the initial setting value has been received (on_started will be called
          * afterward) or the setting value has been changed.
          *
          * @param sender Specifies the object that is calling this method.
          *
          * @param setting Specifies the setting that has been changed.
          */
         typedef Csi::SharedPtr<Setting> setting_handle;
         virtual void on_setting_changed(SettingsEnumerator *sender, setting_handle &setting)
         { }
      };


      /**
       * Defines a component that can be be used by the application to read the set of LgrNet
       * settings and to receive notifications when these are changed.
       *
       * In order to use this component, the application must provide an object derived from class
       * SettingsEnumeratorClient.  It should then create an instance of this class and call one of
       * the two start() methods.  As the initial setting values are received from the server, the
       * client's on_setting_changed() method will be called.  When all of the settings have been
       * processed, the client's on_started() method will be called.  Thereafter, if any setting has
       * been changed, the client's on_setting_changed() method will be called.  If the transaction
       * failed, the client's on_failure() method can be called at any time.
       */
      class SettingsEnumerator: public ClientBase, private SettingHandler, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the object used to create setting objects as they are received from the
          * server.
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
          * Sets the setting factory to be used.  The application can use this as a mechanism to
          * limit the settings that can be handled.
          *
          * @param setting_factory_ Specifies the setting factory to be used.
          */
         void set_setting_factory(Csi::SharedPtr<SettingFactory> setting_factory_);

         /**
          * Called to start the server transaction.
          *
          * @param client_ Specifies the application object that will receive notifications.
          *
          * @param router Specifies a messaging router that is newly allocated and not yet connected
          * to the server.
          *
          * @param other_component Specifies an object that already has a server connection that can be
          * shared.
          */
         typedef SettingsEnumeratorClient client_type;
         void start(client_type *client_, router_handle &router);
         void start(client_type *client_, ClientBase *other_component);

         /**
          * Overloads the base class to halt the server transaction.
          */
         virtual void finish();

         /**
          * Formats the failure code to the specified stream.
          *
          * @param out Specifies the stream to which the failure will be formatted.
          *
          * @param failure Specifies the failure code to format.
          */
         static void format_failure(std::ostream &out, client_type::failure_type failure);

         /**
          * Overloads the base class version to handle an incoming message.
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);
         
         /**
          * Overloads the base class version to start the server transaction.
          */
         virtual void on_corabase_ready();

         /**
          * Overloads the base class version to handle a failure.
          */
         virtual void on_corabase_failure(corabase_failure_type failure);
         virtual void on_corabase_session_failure();

         /**
          * Implements an event handler for the case of having received a setting.
          */
         virtual void on_setting_read(Csi::SharedPtr<Setting> &setting, uint4 context_token);

         /**
          * Overloads the base class version to handle an asynchronous event.
          */
         void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         /**
          * Specifies the application client object reference.
          */
         client_type *client;

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
      };
   };
};

#endif
