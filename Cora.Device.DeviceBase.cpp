/* Cora.Device.DeviceBase.cpp

   Copyright (C) 2000, 2007 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 07 June 2000
   Last Change: Thursday 29 November 2007
   Last Commit: $Date: 2007-11-29 15:04:32 -0600 (Thu, 29 Nov 2007) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.DeviceBase.h"
#include "Cora.LgrNet.Defs.h"
#include "Cora.Sec2.Defs.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      ////////////////////////////////////////////////////////////
      // class DeviceBase definitions
      ////////////////////////////////////////////////////////////
      DeviceBase::DeviceBase():
         device_session(0),
         device_access_level(Cora::Sec2::AccessLevels::level_root)
      { }


      DeviceBase::~DeviceBase()
      { finish(); }

      
      void DeviceBase::set_device_name(StrUni const &device_name_)
      {
         if(state == corabase_state_standby)
            device_name = device_name_;
         else
            throw exc_invalid_state();
      } // set_device_name


      void DeviceBase::start(router_handle &router)
      {
         if(state == corabase_state_standby)
         {
            if(device_name.length() > 0)
               ClientBase::start(router);
            else
               throw std::invalid_argument("Invalid device name");
         }
         else
            throw exc_invalid_state();
      } // start


      void DeviceBase::start(ClientBase *other_client)
      {
         if(state == corabase_state_standby)
         {
            if(device_name.length() > 0)
               ClientBase::start(other_client);
            else
               throw std::invalid_argument("Invalid device name");
         }
         else
            throw exc_invalid_state();
      } // start


      void DeviceBase::finish()
      {
         if(device_session != 0 && router.get_rep())
         {
            router->closeSession(device_session);
            device_session = 0;
         }
         ClientBase::finish();
      } // finish


      void DeviceBase::onNetSesBroken(
         Csi::Messaging::Router *rtr,
         uint4 session_no,
         uint4 reason,
         char const *why)
      {
         if(session_no == device_session)
         {
            device_session = 0;
            if(reason == Csi::Messaging::Messages::session_closed_no_object)
               on_devicebase_failure(devicebase_failure_invalid_device_name);
            else
               on_devicebase_session_failure();
         }
         else
            ClientBase::onNetSesBroken(rtr,session_no,reason,why);
      } // onNetSesBroken


      void DeviceBase::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(msg->getMsgType() == Messages::announce_access_level)
         {
            msg->reset();
            msg->readUInt4(device_access_level);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage


      void DeviceBase::on_corabase_ready()
      {
         // send a message to open the device session. Since this has no acknowledgement, we will
         // assume that the session was opened.
         Csi::Messaging::Message open_cmd(
            net_session,
            LgrNet::Messages::open_device_session_cmd);
         
         device_session = router->openSession(this);
         open_cmd.addWStr(device_name);
         open_cmd.addUInt4(device_session);
         router->sendMessage(&open_cmd);
         on_devicebase_ready();
      } // on_corabase_ready


      void DeviceBase::on_corabase_failure(corabase_failure_type failure)
      {
         devicebase_failure_type device_failure;
         switch(failure)
         {
         case corabase_failure_logon:
            device_failure = devicebase_failure_logon;
            break;
            
         case corabase_failure_session:
            device_failure = devicebase_failure_session;
            break;
            
         case corabase_failure_unsupported:
            device_failure = devicebase_failure_unsupported;
            break;
            
         case corabase_failure_security:
            device_failure = devicebase_failure_security;
            break;

         default:
            device_failure = devicebase_failure_unknown;
            break;
         }
         on_devicebase_failure(device_failure);
      } // on_corabase_failure


      void DeviceBase::on_corabase_session_failure()
      { 
         // once the device session is opened, we will not need the net session. A derived class
         // might want to overload this however in case it does need the net session 
      } // on_corabase_session_failure


      void DeviceBase::format_devicebase_failure(
         std::ostream &out, devicebase_failure_type failure)
      {
         using namespace DeviceBaseStrings;
         switch(failure)
         {
         case devicebase_failure_unknown:
            describe_failure(out, corabase_failure_unknown);
            break;
            
         case devicebase_failure_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case devicebase_failure_session:
            describe_failure(out, corabase_failure_session);
            break;
            
         case devicebase_failure_invalid_device_name:
            out << my_strings[strid_invalid_device_name];
            break;
            
         case devicebase_failure_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         case devicebase_failure_security:
            describe_failure(out, corabase_failure_security);
            break;
         }
      } // format_devicebase_failure
   };
};
