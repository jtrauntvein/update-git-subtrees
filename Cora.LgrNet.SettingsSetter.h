/* Cora.LgrNet.SettingsSetter.h

   Copyright (C) 2002, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 19 December 2002
   Last Change: Monday 21 October 2019
   Last Commit: $Date: 2019-10-21 08:28:25 -0600 (Mon, 21 Oct 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_SettingsSetter_h
#define Cora_LgrNet_SettingsSetter_h

#include "Cora.ClientBase.h"
#include "Cora.Setting.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace LgrNet
   {
      class SettingsSetter;


      /**
       * Defines the interface that the application must implement in order to use the
       * SettingsSetter component type.
       */
      class SettingsSetterClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the component when the transaction is complete.
          *
          * @param sender Specifies the component that is calling this method.
          *
          * @param outcome Specifies the an code that describes the outcome.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_some_errors = 2,
            outcome_invalid_logon = 3,
            outcome_session_failed = 4,
            outcome_security_blocked = 5,
            outcome_unsupported = 6,
            outcome_network_locked = 7,
         };
         virtual void on_complete(SettingsSetter *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines a component that can be used to set one or more LgrNet level settings.  In order to
       * use this component, the application must provide an object that is derived from class
       * SettingsSetterClient.  It should then create an instance of this class, add one or more
       * settings, and call one of the two versions of start().  When the transaction is complete,
       * the component will call the client's on_complete() method.
       */
      class SettingsSetter:
         public ClientBase,
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
          * Adds a setting to the set of values that should be sent to the LoggerNet server.
          *
          * @param setting Specifies the setting to add.
          */
         typedef Csi::SharedPtr<Setting> setting_handle;
         void add_setting(setting_handle setting);

         /**
          * Clears the list of settings to transmit.
          */
         void clear_settings();

         /**
          * @return Returns the iterator to the first setting to be sent.
          */
         typedef std::list<setting_handle> settings_type;
         typedef settings_type::iterator iterator;
         typedef settings_type::const_iterator const_iterator;
         iterator begin() { return settings.begin(); }
         const_iterator begin() const { return settings.begin(); }

         /**
          * @return Returns the iterator beyond the last setting to be sent.
          */
         iterator end() { return settings.end(); }
         const_iterator end() const { return settings.end(); }

         /**
          * @return Returns the number of settings to be sent.
          */
         settings_type::size_type size() const { return settings.size(); }

         /**
          * Starts the connection to the LoggerNet server.
          *
          * @param client_ Specifies the application's client object.
          *
          * @param router Specifies a messaging router that has not yet been connected to the
          * server.
          *
          * @param other_component Specifies a component that has a connection that can be shared
          * with this component.
          */
         typedef SettingsSetterClient client_type;
         void start(client_type *client_, router_handle &router);
         void start(client_type *client_, ClientBase *other_component);

         /**
          * Closes the sessions and restores this component to a standby state.
          */
         virtual void finish();

         /**
          * Overloads the base class version to handle asynch events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Writes a description of the specified outcome code to the specified stream.
          *
          * @param out Specifies the output stream.
          *
          * @param outcome Specifies the outcome code.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);
         
      protected:
         /**
          * Overloads the base class version to start the server transaction.
          */
         virtual void on_corabase_ready();

         /**
          * Overloads the base class version to handle a failure report.
          */
         virtual void on_corabase_failure(corabase_failure_type failure);
         virtual void on_corabase_session_failure()
         { on_corabase_failure(corabase_failure_session); }

         /**
          * Overloads the base class version to handle incoming messages.
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);

      private:
         /**
          * Specifies the client object.
          */
         client_type *client;

         /**
          * Specifies the state of this compenent.
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
