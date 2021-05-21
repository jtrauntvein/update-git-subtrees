/* Cora.Device.CollectArea.SettingSetter.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 02 November 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_CollectArea_SettingSetter_h
#define Cora_Device_CollectArea_SettingSetter_h

#include "Cora.Device.DeviceBase.h"
#include "Cora.Setting.h"
#include "Csi.InstanceValidator.h"
#include "CsiEvents.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         //@group class forward declarations
         class SettingSetter;
         //@endgroup


         ////////////////////////////////////////////////////////////
         // class SettingSetterClient
         //
         // Defines the interface that is expected from an object that is a
         // client to a SettingSetter process.
         ////////////////////////////////////////////////////////////
         class SettingSetterClient: public Csi::InstanceValidator
         {
         public:
            ////////////////////////////////////////////////////////////
            // on_complete
            ////////////////////////////////////////////////////////////-0 
            enum outcome_type
            {
               outcome_unknown = 0,
               outcome_success = 1,
               outcome_invalid_logon = 2,
               outcome_invalid_device_name = 3,
               outcome_session_failed = 4,
               outcome_security_blocked = 5,
               outcome_unsupported = 6,
               outcome_unsupported_setting = 7,
               outcome_invalid_setting_value = 8,
               outcome_setting_read_only = 9,
               outcome_invalid_collect_area_name = 10,
               outcome_network_locked = 11,
            };
            virtual void on_complete(
               SettingSetter *setter,
               outcome_type resp_code) = 0; 
         };


         ////////////////////////////////////////////////////////////
         // class SettingSetter
         //
         // Defines an object that is designed to set the value of a
         // collect area setting in behalf of a client.
         //
         // Once an object of this class has been created, the application
         // should set the appropriate properties of this class as well as
         // its base classes (set_device_name(), set_collect_area_name(),
         // set_the_setting(), etc). Once these properties have been set,
         // the start() method should be invoked. This will cause the
         // object to start the server transaction. Once the transaction is
         // complete, the object will invoke the client's on_complete()
         // method with a response code indicating whether the transaction
         // succeeded. At this point, the object will be in a standby state
         // and its properties can be changed and the object restarted.
         ////////////////////////////////////////////////////////////
         class SettingSetter: public DeviceBase, public Csi::EvReceiver
         {
         private:
            //@group properties
            ////////////////////////////////////////////////////////////
            // collect_area_name
            //////////////////////////////////////////////////////////// 
            StrUni collect_area_name;

            ////////////////////////////////////////////////////////////
            // the_setting
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
            // set_collect_area_name
            //////////////////////////////////////////////////////////// 
            void set_collect_area_name(StrUni const &collect_area_name_);

            ////////////////////////////////////////////////////////////
            // get_collect_area_name
            //////////////////////////////////////////////////////////// 
            StrUni const &get_collect_area_name() const { return collect_area_name; }

            ////////////////////////////////////////////////////////////
            // set_the_setting
            //////////////////////////////////////////////////////////// 
            void set_the_setting(Csi::SharedPtr<Setting> the_setting_);
            void set_the_setting(Setting *the_setting_)
            { set_the_setting(Csi::SharedPtr<Setting>(the_setting_)); }

            ////////////////////////////////////////////////////////////
            // get_the_setting
            //////////////////////////////////////////////////////////// 
            Csi::SharedPtr<Setting> &get_the_setting() { return the_setting; }

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

         protected:
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
            virtual void onNetMessage(
               Csi::Messaging::Router *rtr,
               Csi::Messaging::Message *msg);

            ////////////////////////////////////////////////////////////
            // receive
            //////////////////////////////////////////////////////////// 
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         private:
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
            // client
            //////////////////////////////////////////////////////////// 
            SettingSetterClient *client;
         };
      };
   };
};

#endif
