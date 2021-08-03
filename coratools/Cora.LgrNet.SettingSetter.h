/* Cora.LgrNet.SettingSetter.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 13 September 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_LgrNet_SettingSetter_h
#define Cora_LgrNet_SettingSetter_h

#include "Cora.ClientBase.h"
#include "Cora.Setting.h"
#include "CsiEvents.h"
#include "Csi.InstanceValidator.h"

namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class SettingSetter;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class SettingSetterClient
      //
      // Defines the client interface for a SettingSetter object
      ////////////////////////////////////////////////////////////
      class SettingSetterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called after the set setting attempt has completed. The outcome
         // parameter will indicate whether the attempt succeeded.
         //////////////////////////////////////////////////////////// 
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_session_failed = 2,
            outcome_invalid_logon = 3,
            outcome_server_security_blocked = 4,
            outcome_unsupported_transaction = 5,
            outcome_unsupported_setting = 6,
            outcome_invalid_value = 7,
            outcome_read_only = 8,
            outcome_network_locked = 9,
         };
         virtual void on_complete(
            SettingSetter *setter,
            outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class SettingSetter
      //
      // Defines an object that can set one LgrNet setting at a time on
      // cora server. An application can use this class by providing a
      // SettingSetterClient derived client object, a router, and a setting
      // value. The application can optionally provide a default network
      // session which, if used, will prevent the logon protocol from being
      // executed.
      //
      // After a SettingSetter object has been created (while it is in a
      // standby mode) an application can invoke its set_logon_name(),
      // set_logon_password, and set_the_setting() property set
      // methods. Following this, the application should invoke start() to
      // commit the setting change. When the server transaction is over,
      // the setting setter will invoke the client's on_complete() method
      // with the appropriate outcome code.
      ////////////////////////////////////////////////////////////
      class SettingSetter:
         public ClientBase,
         public Csi::EvReceiver
      {
      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // the_setting
         //
         // The setting value that should be sent to the server. This
         // property must be set through method set_the_setting().
         //////////////////////////////////////////////////////////// 
         Csi::SharedPtr<Setting> the_setting;
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         SettingSetter();

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~SettingSetter();

         ////////////////////////////////////////////////////////////
         // set_the_setting
         //////////////////////////////////////////////////////////// 
         typedef Csi::SharedPtr<Setting> setting_handle;
         void set_the_setting(setting_handle &the_setting_);

         ////////////////////////////////////////////////////////////
         // start
         //////////////////////////////////////////////////////////// 
         void start(
            SettingSetterClient *client_,
            router_handle &router);
         void start(
            SettingSetterClient *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         //////////////////////////////////////////////////////////// 
         void finish();

      private:
         //@group ClientBase overloaded methods
         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         //////////////////////////////////////////////////////////// 
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_corabase_session_failure();

         ////////////////////////////////////////////////////////////
         // onNetMessage
         //////////////////////////////////////////////////////////// 
         virtual void onNetMessage(
            Csi::Messaging::Router *router,
            Csi::Messaging::Message *message);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         //////////////////////////////////////////////////////////// 
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // client
         //////////////////////////////////////////////////////////// 
         typedef SettingSetterClient client_type;
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
