/* Cora.Device.CollectAreaResetter.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 09 July 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_CollectAreaResetter_h
#define Cora_Device_CollectAreaResetter_h

#include "Cora.Device.DeviceBase.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class CollectAreaResetter;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class CollectAreaResetterClient
      ////////////////////////////////////////////////////////////
      class CollectAreaResetterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_area_name = 2,
            outcome_server_security_blocked = 3,
            outcome_invalid_device_name = 4,
            outcome_unsupported = 5,
            outcome_invalid_logon = 6,
            outcome_session_failed = 7,
         };
         virtual void on_complete(
            CollectAreaResetter *resetter,
            outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class CollectAreaResetter
      //
      // Defines a component that executes the Device Collect Arae Reset transaction.  In order to
      // use this component, an application must provide an object that is derived from class
      // CollectAreaResetterClient (also known as CollectAreaResetter::client_type).  It should
      // create an instance of CollectAreaResetter and set the appropriate properties including
      // set_device_name() and set_collect_area_name().  It should then call start().  When the
      // server transaction is complete, the client's on_complete() methods will be invoked.  
      ////////////////////////////////////////////////////////////
      class CollectAreaResetter:
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
         CollectAreaResetterClient *client;

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
         CollectAreaResetter():
            state(state_standby),
            client(0)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~CollectAreaResetter()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // set_collect_area_name
         ////////////////////////////////////////////////////////////
         void set_collect_area_name(StrUni const &collect_area_name_)
         {
            if(state == state_standby)
               collect_area_name = collect_area_name_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_collect_area_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_collect_area_name() const
         { return collect_area_name; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef CollectAreaResetterClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other);

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
      };
   };
};


#endif
