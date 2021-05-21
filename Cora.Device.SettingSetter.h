/* Cora.Device.SettingSetter.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 01 August 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_SettingSetter_h
#define Cora_Device_SettingSetter_h


#include "Cora.Device.DeviceBase.h"
#include "Cora.Setting.h"
#include "Csi.InstanceValidator.h"
#include "CsiEvents.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class SettingSetter;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class SettingSetterClient
      //
      // Defines the notification interface for a client to a device
      // setting setter object.
      ////////////////////////////////////////////////////////////
      class SettingSetterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //////////////////////////////////////////////////////////// 
         enum resp_code_type
         {
            resp_unknown = 0,
            resp_success = 1,
            resp_invalid_logon = 2,
            resp_invalid_device_name = 3,
            resp_session_failed = 4,
            resp_security_blocked = 5,
            resp_unsupported = 6,
            resp_unsupported_setting = 7,
            resp_invalid_setting_value = 8,
            resp_setting_read_only = 9,
            resp_network_locked = 10,
         };
         virtual void on_complete(SettingSetter *setter,
                                  resp_code_type resp_code) = 0;
      };

      ////////////////////////////////////////////////////////////
      // class SettingSetter
      //
      // Defines a concrete class that can be used to set a device setting
      // on the cora server. In order to use this class, a client must be
      // derived from class SettingSetterClient which defines the
      // notification interface.
      //
      // To use this class, a client should create an instance of it, set
      // the appropriate properties including the device name and setting
      // value, and then invoke start(). When the server transaction is
      // complete, the client will receive a notification through ots
      // on_complete() method which will provide an indicator regarding the
      // success of the transaction. Once this notification has been sent,
      // this object will be returned to a standby state and can possibly
      // be used to set another setting if desired.
      ////////////////////////////////////////////////////////////
      class SettingSetter: public DeviceBase, public Csi::EvReceiver
      {
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
         //
         // Sets the setting property which contains the value (and setting
         // identifier) that will be written to the server.
         //////////////////////////////////////////////////////////// 
         void set_the_setting(Csi::SharedPtr<Setting> the_setting_);
         void set_the_setting(Setting *the_setting_)
         { set_the_setting(Csi::SharedPtr<Setting>(the_setting_)); }

         ////////////////////////////////////////////////////////////
         // get_setting_identifier
         //////////////////////////////////////////////////////////// 
         uint4 get_setting_identifier() const { return the_setting->get_identifier(); }

         ////////////////////////////////////////////////////////////
         // start
         //
         // This method is invoked to initiate the connection to the serve
         // (if required) and to start the server set device setting
         // transaction. The client will receive notification when the
         // server transaction has been completed through its on_complete()
         // method.
         //////////////////////////////////////////////////////////// 
         void start(
            SettingSetterClient *client,
            router_handle &router);
         void start(
            SettingSetterClient *client,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         //
         // Releases the server sessions and places this object back into a
         // standby state. If this method is invoked while in an active
         // state (waiting for a server set device setting transaction),
         // the transaction will be completed even though the on_complete()
         // notification will be cancelled.
         //////////////////////////////////////////////////////////// 
         void finish();

      protected:
         //@group DeviceBase overloaded methods
         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         //////////////////////////////////////////////////////////// 
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_devicebase_session_failure();

         ////////////////////////////////////////////////////////////
         // onNetMessage
         //////////////////////////////////////////////////////////// 
         virtual void onNetMessage(Csi::Messaging::Router *rtr,
                                   Csi::Messaging::Message *msg);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         //////////////////////////////////////////////////////////// 
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // client
         //////////////////////////////////////////////////////////// 
         SettingSetterClient *client;

         ////////////////////////////////////////////////////////////
         // state
         //////////////////////////////////////////////////////////// 
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
         } state;

         ////////////////////////////////////////////////////////////
         // the_setting
         //////////////////////////////////////////////////////////// 
         Csi::SharedPtr<Setting> the_setting;
      };
   };
};

#endif
