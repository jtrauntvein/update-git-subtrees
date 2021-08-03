/* Cora.PbRouter.SettingsSetter.h

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 10 May 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_PbRouter_SettingsSetter_h
#define Cora_PbRouter_SettingsSetter_h

#include "Cora.PbRouter.PbRouterBase.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace PbRouter
   {
      //@group class forward declarations
      class SettingsSetter;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class SettingsSetterClient
      ////////////////////////////////////////////////////////////
      class SettingsSetterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_invalid_router_id = 3,
            outcome_server_permission_denied = 4,
            outcome_server_session_failed = 5,
            outcome_communication_failed = 6,
            outcome_communication_disabled = 7,
            outcome_unsupported = 8,
            outcome_unreachable = 9,
            outcome_setting_read_only = 10,
            outcome_not_enough_space = 11,
            outcome_invalid_name_or_value = 12,
            outcome_node_permission_denied = 13,
         };
         virtual void on_complete(
            SettingsSetter *setter,
            outcome_type outcome,
            uint4 settings_applied) = 0;
      };

      
      ////////////////////////////////////////////////////////////
      // class SettingsSetter
      //
      // This class defines a component that is able to set one or more PakBus settings at a time.
      // It should be run against one of the PakBus port objects in the server's network map but can
      // set the setting for any PakBus device known to the router.
      //
      // In order to use this component, the application can create an instance of this class, set
      // attributes including router address, pakbus address, and setting values, and call one of
      // the start() methods with a pointer to a client-derived object.
      //
      // Once the server transaction is complete, the component will invoke the client's
      // on_complete() method and thus report the outcome of the transaction. 
      ////////////////////////////////////////////////////////////
      class SettingsSetter:
         public PbRouterBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // pakbus_address
         ////////////////////////////////////////////////////////////
         uint2 pakbus_address;

         ////////////////////////////////////////////////////////////
         // settings
         ////////////////////////////////////////////////////////////
         typedef std::pair<StrAsc, StrAsc> setting_type;
         typedef std::list<setting_type> settings_type;
         settings_type settings;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SettingsSetter();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SettingsSetter();

         ////////////////////////////////////////////////////////////
         // get_pakbus_address
         ////////////////////////////////////////////////////////////
         uint2 get_pakbus_address() const { return pakbus_address; }

         ////////////////////////////////////////////////////////////
         // set_pakbus_address
         ////////////////////////////////////////////////////////////
         void set_pakbus_address(uint2 pakbus_address_);

         ////////////////////////////////////////////////////////////
         // add_setting
         ////////////////////////////////////////////////////////////
         void add_setting(
            StrAsc const &setting_name,
            StrAsc const &setting_value);

         ////////////////////////////////////////////////////////////
         // clear_settings
         ////////////////////////////////////////////////////////////
         void clear_settings();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef SettingsSetterClient client_type;
         virtual void start(
            client_type *client,
            router_handle &router);
         virtual void start(
            client_type *client,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

      protected:
         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_ready();

         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_failure(pbrouterbase_failure_type);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router,
            Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;
      };
   };
};


#endif
