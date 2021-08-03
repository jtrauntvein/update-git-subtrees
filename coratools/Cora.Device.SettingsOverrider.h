/* Cora.Device.SettingsOverrider.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 01 August 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_Device_SettingsOverrider_h
#define Cora_Device_SettingsOverrider_h


#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "Cora.Setting.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class SettingsOverrider;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class SettingsOverriderClient
      //
      // Declares the interface that should be implemented by a client to an SettingsOverrider
      // object.
      ////////////////////////////////////////////////////////////
      class SettingsOverriderClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when the server device settings override transaction has
         // been started successfully. If a failure occurs while starting the
         // transaction, on_failure() will be called instead.
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            SettingsOverrider *overrider) = 0;

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called when a failure has prevented the server transaction from starting properly.
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
         };
         virtual void on_failure(
            SettingsOverrider *overrider,
            failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_override_started
         //
         // Called after an override attempt for a setting has occurred. The
         // resp_code parameter will indicate outcome of attempt.
         ////////////////////////////////////////////////////////////
         enum resp_code_type
         {
            resp_overridden = 1,
            resp_unsupported_setting = 2,
            resp_invalid_format = 3,
            resp_setting_not_overriddable = 4,
         };
         virtual void on_override_started(
            SettingsOverrider *overrider,
            uint4 setting_id,
            resp_code_type resp_code)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class SettingsOverrider
      //
      // Defines a concrete class that encapsulates the cora server device
      // settings override transaction. In order to use this class, a client
      // must derive from class SettingsOverriderClient. The client should
      // create an instance of this class and then call set_xxx() methods to
      // adjust properties like device name. Finally, it shouild invoke
      // start().
      //
      // If the transaction is successfully started, the client's on_start()
      // method will be invoked. At this point, the client can begin calling
      // override() with various setting objects to cause those to be
      // overridden. For each of these calls, the client's
      // on_override_started() method will be called back. This can happen any
      // number of times while this object is in an active state.
      //
      // If, at any time after start() is called, an unrecoverable failure
      // occurs, the client's on_failure() method will be invoked and this
      // object will be placed back into a standby state.
      ////////////////////////////////////////////////////////////
      class SettingsOverrider: public DeviceBase, public Csi::EvReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SettingsOverrider();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SettingsOverrider();

         ////////////////////////////////////////////////////////////
         // start
         //
         // This method should be invoked whilke this object is in a standby
         // state. Following base class initialisation, the settings override
         // transaction will be started with no initial overrides. If the
         // transaction is started successfully, the client's on_started()
         // method will be invoked. It can then invoke override() as many times
         // as desired. The transaction (and associated overrides) will remain
         // active until finish() is called or the server session is lost.
         ////////////////////////////////////////////////////////////
         typedef SettingsOverriderClient client_type;
         void start(
            client_type *client,
            router_handle &router);
         void start(
            client_type *client,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // override_setting
         //
         // This method can be invoked by the client following an on_started()
         // notification. For each invocation of this method, the client will
         // receive one on_override_started() notification.
         ////////////////////////////////////////////////////////////
         void override_setting(Setting const &the_setting);

         ////////////////////////////////////////////////////////////
         // finish
         //
         // This method should be invoked when the client wants to stop the
         // override transaction. It will place this object back into a standby
         // state and cancel any overrides that may have taken place since
         // start() was invoked.
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(
            std::ostream &out, client_type::failure_type failure);

      protected:
         //@group DeviceBase overloaded methods
         ////////// on_devicebase_ready
         virtual void on_devicebase_ready();

         ////////// on_devicebase_failure
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////// on_devicebase_session_failure
         virtual void on_devicebase_session_failure();

         ////////// onNetMessage
         virtual void onNetMessage(Csi::Messaging::Router *rtr,
                                   Csi::Messaging::Message *msg);
         //@endgroup

         ////////// receive
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
      private:
         ////////// client
         SettingsOverriderClient *client;

         ////////// state
         enum state_type
         {
            state_standby,
            state_delegate,
            state_before_active,
            state_active,
         } state;

         ////////// override_tran_no
         uint4 override_tran_no;
      };
   };
};

#endif
