/* Cora.PbRouter.SettingsGetter.h

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 08 May 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_PbRouter_SettingsGetter_h
#define Cora_PbRouter_SettingsGetter_h

#include "Cora.PbRouter.PbRouterBase.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace PbRouter
   {
      //@group class forward declarations
      class SettingsGetter;
      //@endgroup


      class SettingsGetterClient: public Csi::InstanceValidator
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
            outcome_unreachable = 8,
            outcome_unsupported = 9,
         };
         typedef std::pair<StrAsc, StrAsc> setting_type;
         typedef std::list<setting_type> settings_type;
         virtual void on_complete(
            SettingsGetter *getter,
            outcome_type outcome,
            settings_type const &settings) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class SettingsGetter
      //
      // This class defines a component that can retrieve some or all of the settings from a PakBus
      // device known by the router.
      //
      // In order to use this component, an application can instantiate an instance of this class
      // directly and must then set the appropriate properties including set_pakbus_router_id() and
      // set_pakbus_address().  It should then invoke one of the two versions of start() and pass
      // into this method a client object that is derived from class SettingsGetterClient.  This
      // client object will receive an on_complete() notification when the server transaction has
      // been completed.
      ////////////////////////////////////////////////////////////
      class SettingsGetter:
         public PbRouterBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // pakbus_address
         ////////////////////////////////////////////////////////////
         uint2 pakbus_address;
         
         ////////////////////////////////////////////////////////////
         // setting_names
         ////////////////////////////////////////////////////////////
         typedef std::list<StrAsc> setting_names_type;
         setting_names_type setting_names;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SettingsGetter();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SettingsGetter();

         ////////////////////////////////////////////////////////////
         // set_pakbus_address
         ////////////////////////////////////////////////////////////
         void set_pakbus_address(uint2 pakbus_address_);

         ////////////////////////////////////////////////////////////
         // get_pakbus_address
         ////////////////////////////////////////////////////////////
         uint2 get_pakbus_address() const { return pakbus_address; }
         
         ////////////////////////////////////////////////////////////
         // add_setting_name
         ////////////////////////////////////////////////////////////
         void add_setting_name(StrAsc const &setting_name);

         ////////////////////////////////////////////////////////////
         // clear_setting_names
         ////////////////////////////////////////////////////////////
         void clear_setting_names();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef SettingsGetterClient client_type;
         void start(
            client_type *client,
            router_handle &router);
         void start(
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
         virtual void on_pbrouterbase_failure(pbrouterbase_failure_type failure);

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
