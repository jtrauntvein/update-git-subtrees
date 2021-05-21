/* Cora.LgrNet.DeviceDefaultSettingsLister.h

   Copyright (C) 2003, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 25 April 2003
   Last Change: Monday 04 October 2010
   Last Commit: $Date: 2010-10-04 15:06:44 -0600 (Mon, 04 Oct 2010) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_DeviceDefaultSettingsLister_h
#define Cora_LgrNet_DeviceDefaultSettingsLister_h

#include "Cora.ClientBase.h"
#include "Csi.Events.h"
#include "Cora.SettingFactory.h"
#include <list>
#include <vector>
#include <iterator>


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class DeviceDefaultSettingsLister;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class DeviceDefaultSettingsListerClient
      ////////////////////////////////////////////////////////////
      class DeviceDefaultSettingsListerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // class setting_status_type
         ////////////////////////////////////////////////////////////
         struct setting_status_type
         {
            ////////////////////////////////////////////////////////////
            // setting_id
            ////////////////////////////////////////////////////////////
            uint4 setting_id;

            ////////////////////////////////////////////////////////////
            // status
            ////////////////////////////////////////////////////////////
            enum status_type
            {
               setting_known = 1,
               setting_ignored = 2,
               setting_unpredictable = 3
            } status;

            ////////////////////////////////////////////////////////////
            // setting
            //
            // Reference to the setting object.  Will be a null reference if the status is not set
            // to setting_known.
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<Setting> setting;
         };
         
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_session_broken = 3,
            outcome_unsupported = 4,
            outcome_server_security_blocked = 5, 
            outcome_invalid_device_type = 6,
         };
         typedef std::list<setting_status_type> settings_type;
         virtual void on_complete(
            DeviceDefaultSettingsLister *lister,
            outcome_type outcome,
            settings_type const &settings) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class DeviceDefaultSettingsLister
      ////////////////////////////////////////////////////////////
      class DeviceDefaultSettingsLister:
         public ClientBase,
         public Csi::EventReceiver
      {
      public:
         typedef std::vector<DevTypeCode> context_type;
         typedef DeviceDefaultSettingsListerClient client_type;

      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // factory
         //
         // Specifies the policy of how settings will be allocated.  If the factory returns a null
         // pointer for a particular setting ID, that setting will not be represented in the
         // results in on_complete().
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<SettingFactory> factory;

         ////////////////////////////////////////////////////////////
         // context
         ////////////////////////////////////////////////////////////
         context_type context;
         //@endgroup

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
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         DeviceDefaultSettingsLister();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~DeviceDefaultSettingsLister();

         ////////////////////////////////////////////////////////////
         // set_factory
         ////////////////////////////////////////////////////////////
         void set_factory(Csi::SharedPtr<SettingFactory> &factory_)
         {
            if(state == state_standby)
               factory = factory_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_factory
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<SettingFactory> &get_factory()
         { return factory; }

         ////////////////////////////////////////////////////////////
         // set_context
         ////////////////////////////////////////////////////////////
         template <class iterator>
         void set_context(iterator begin, iterator end)
         {
            if(state == state_standby)
               std::copy(begin,end,std::back_inserter(context));
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_context
         ////////////////////////////////////////////////////////////
         context_type const &get_context() const
         { return context; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

      protected:
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
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &event);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg); 
      };
   };
};


#endif
