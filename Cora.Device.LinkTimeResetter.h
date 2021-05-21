/* Cora.Device.LinkTimeResetter.h

   Copyright (C) 2008, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 07 January 2008
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_Device_LinkTimeResetter_h
#define Cora_Device_LinkTimeResetter_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      ////////////////////////////////////////////////////////////
      // class LinkTimeResetterClient
      ////////////////////////////////////////////////////////////
      class LinkTimeResetter;
      class LinkTimeResetterClient: public Csi::InstanceValidator
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
            outcome_invalid_security  = 3,
            outcome_invalid_device_name = 4,
            outcome_session_lost = 5,
            outcome_unsupported = 6
         };
         virtual void on_complete(
            LinkTimeResetter *resetter,
            outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class LinkTimeResetter
      ////////////////////////////////////////////////////////////
      class LinkTimeResetter:
         public DeviceBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         LinkTimeResetter();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~LinkTimeResetter()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef LinkTimeResetterClient client_type;
         void start(client_type *client_, router_handle router);
         void start(client_type *client_, ClientBase *other_client);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // format_outcome
         ////////////////////////////////////////////////////////////
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);
         
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
         virtual void on_devicebase_session_failure()
         { on_devicebase_failure(devicebase_failure_session); }

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
      protected:
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
