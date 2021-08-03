/* Cora.Device.CollectAreaSettingsOverrider.h

   Copyright (C) 2006, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 12 May 2006
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_Device_CollectAreaSettingsOverrider_h
#define Cora_Device_CollectAreaSettingsOverrider_h

#include "Cora.Device.DeviceBase.h"
#include "Cora.Setting.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class CollectAreaSettingsOverrider;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class CollectAreaSettingsOverriderClient
      ////////////////////////////////////////////////////////////
      class CollectAreaSettingsOverriderClient:
         public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called after the override transaction has been established with the
         // server.  Once this method is invoked, the client can use the
         // override() method of the overrider component to effect setting
         // overrides for the collect area.
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            CollectAreaSettingsOverrider *overrider) = 0;

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called when the override transaction could not be started or
         // sustained.
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_logon = 1,
            failure_invalid_device_name = 2,
            failure_security_blocked = 3,
            failure_unsupported = 4,
            failure_session_failed = 5,
            failure_another_in_progress = 6,
            failure_invalid_collect_area_name = 7,
            failure_collect_area_deleted = 8,
            failure_device_deleted = 9
         };
         virtual void on_failure(
            CollectAreaSettingsOverrider *overrider,
            failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_override_started
         //
         // Called when an attempt to override a collect area setting has
         // started or could be started.
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_overriden = 1,
            outcome_unsupported_setting = 2,
            outcome_invalid_format = 3,
            outcome_setting_not_overridable = 4
         };
         virtual void on_override_started(
            CollectAreaSettingsOverrider *overrider,
            uint4 setting_id,
            outcome_type outcome)
         { } 
      };


      ////////////////////////////////////////////////////////////
      // class CollectAreaSettingsOverrider
      //
      // Defines a component that can be used to override the values of collect
      // area settings in the loggernet server.  In order to use this
      // component, the application must provide an object derived from class
      // CollectAreaSettingsOverriderClient (also known as
      // CollectAreaSettingsOverrider::client_type) to receive notifications
      // from the component.  The application should then create an instance of
      // this class, invoke various methods including set_device_name() and
      // set_collect_area_name() to set up object properties, and then invoke
      // one of the two versions of start().
      //
      // Once the component has started the collect area settings override
      // transaction with the server, it will invoke the client's on_started()
      // method.  At this point, the application can override settings for the
      // collect area by invoking the component's override() method.  For each
      // call to override, the client's on_override_started() method will be
      // invoked by the component.   If, at any time, the server transaction
      // fails, the client will be notified through its on_failure() method.
      //
      // The server transaction can be stopped (and the component returned to a
      // standby state) by the application calling the finish() method.  It
      // will also be stopped when the component is deleted while in an active
      // state.  Once the settings override transaction is stopped, all
      // settings that had been overriden will be returned to their normal
      // states.  
      ////////////////////////////////////////////////////////////
      class CollectAreaSettingsOverrider:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // collect_area_name
         ////////////////////////////////////////////////////////////
         StrUni collect_area_name;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         CollectAreaSettingsOverriderClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_before_active,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // override_tran_no
         ////////////////////////////////////////////////////////////
         uint4 override_tran_no;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         CollectAreaSettingsOverrider():
            state(state_standby),
            client(0)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~CollectAreaSettingsOverrider()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // set_collect_area_name
         ////////////////////////////////////////////////////////////
         void set_collect_area_name(StrUni const &collect_area_name_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            collect_area_name = collect_area_name_;
         }

         ////////////////////////////////////////////////////////////
         // get_collect_area_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_collect_area_name() const
         { return collect_area_name; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef CollectAreaSettingsOverriderClient client_type;
         void start(
            client_type *client_,
            router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }

         void start(
            client_type *client_,
            ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(other_component);
         }

         ////////////////////////////////////////////////////////////
         // override_setting
         //
         // This method must be invoked by the application after the client's
         // on_started() method has been called.  For each invocation of this
         // method, the client will receive one on_override_started()
         // call-back. 
         ////////////////////////////////////////////////////////////
         void override_setting(Setting const &the_setting);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(
            devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_session_failure();

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router,
            Csi::Messaging::Message *message);
      };
   };
};


#endif
