/* Cora.Device.DeviceBase.h

   Copyright (C) 2000, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 07 June 2000
   Last Change: Tuesday 01 February 2011
   Last Commit: $Date: 2011-02-02 11:42:30 -0600 (Wed, 02 Feb 2011) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_Device_DeviceBase_h
#define Cora_Device_DeviceBase_h


#include "Cora.ClientBase.h"
#include "Cora.Device.Defs.h"


namespace Cora
{
   namespace Device
   {
      ////////////////////////////////////////////////////////////
      // class DeviceBase
      //
      // Defines a base class on which to build client classes that access device functionality on
      // the cora server. This class can optionally execute the logon protocol defined by base class
      // ClientBase. It will also execute the open device session transaction to open a session with
      // the specified device.
      ////////////////////////////////////////////////////////////
      class DeviceBase: public ClientBase
      {
      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // device_name
         //
         // The name of the device that we should connect to after logon
         ////////////////////////////////////////////////////////////
         StrUni device_name;

         ////////////////////////////////////////////////////////////
         // device_access_level
         //
         // Specifies the client's current access level on the device
         // session.  This value will be updated as the server sends access
         // level announcement messages. 
         ////////////////////////////////////////////////////////////
         uint4 device_access_level;
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         DeviceBase();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~DeviceBase();

         //@group properties access methods
         ////////////////////////////////////////////////////////////
         // set_device_name
         ////////////////////////////////////////////////////////////
         void set_device_name(StrUni const &device_name_);

         ////////////////////////////////////////////////////////////
         // get_device_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_device_name() const { return device_name; }

         ////////////////////////////////////////////////////////////
         // get_device_access_level
         ////////////////////////////////////////////////////////////
         uint4 get_device_access_level() const
         { return device_access_level; }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // start (new connection)
         ////////////////////////////////////////////////////////////
         virtual void start(router_handle &router);

         ////////////////////////////////////////////////////////////
         // start (use existing connection)
         ////////////////////////////////////////////////////////////
         virtual void start(ClientBase *other_client);
         
         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // format_devicebase_failure
         ////////////////////////////////////////////////////////////
         enum devicebase_failure_type
         {
            devicebase_failure_unknown,
            devicebase_failure_logon,
            devicebase_failure_session,
            devicebase_failure_invalid_device_name,
            devicebase_failure_unsupported,
            devicebase_failure_security,
         };
         static void format_devicebase_failure(
            std::ostream &out, devicebase_failure_type failure);

      protected:
         ////////////////////////////////////////////////////////////
         // onNetSesBroken
         ////////////////////////////////////////////////////////////
         virtual void onNetSesBroken(
            Csi::Messaging::Router *rtr,
            uint4 session_no,
            uint4 reason,
            char const *why);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         //@group events to derived classes
         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         //
         // Called when the session with the device has been opened and everything is ready.
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready() = 0;

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(devicebase_failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_session_failure()
         { on_devicebase_failure(devicebase_failure_session); }
               
         //@endgroup

         //@group event notifications overloaded from ClientBase
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
         //@endgroup

      protected:
         ////////////////////////////////////////////////////////////
         // device_session
         //
         // Handle to the device session
         ////////////////////////////////////////////////////////////
         uint4 device_session;
      };
   };
};


#endif
