/* Cora.Device.HoleLister.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 26 July 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_Device_HoleLister_h
#define Cora_Device_HoleLister_h


#include "Csi.InstanceValidator.h"
#include "Cora.Device.DeviceBase.h"
#include "CsiEvents.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward definitions
      class HoleLister;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class HoleListerClient
      //
      // defines the interface expected from a client to a hole lister object
      //////////////////////////////////////////////////////////// 
      class HoleListerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called by the hole lister after it has been successfully started
         // and the initial hole list has been transmitted.
         //////////////////////////////////////////////////////////// 
         virtual void on_started(HoleLister *lister) = 0;

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called by the hole lister at any time after its start() method has
         // been invoked to report a failure condition.
         //////////////////////////////////////////////////////////// 
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_device_name = 1,
            failure_invalid_logon = 2,
            failure_not_supported = 3,
            failure_security_blocked = 4,
            failure_session_failed = 5,
         };
         virtual void on_failure(
            HoleLister *lister,
            failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_hole_added
         //
         // Called when a hole added event has been detected
         //////////////////////////////////////////////////////////// 
         virtual void on_hole_added(
            HoleLister *lister,
            StrUni const &table_name,
            uint4 begin_record_no,
            uint4 end_record_no)
         { }

         ////////////////////////////////////////////////////////////
         // on_hole_collected
         //
         // Called when a hole (or a portion thereof) has been collected
         //////////////////////////////////////////////////////////// 
         virtual void on_hole_collected(
            HoleLister *lister,
            StrUni const &table_name,
            uint4 begin_record_no,
            uint4 end_record_no)
         { }

         ////////////////////////////////////////////////////////////
         // on_hole_uncollectable
         //
         // Called when a hole has become uncollectable
         //////////////////////////////////////////////////////////// 
         virtual void on_hole_uncollectable(
            HoleLister *lister,
            StrUni const &table_name,
            uint4 begin_record_no,
            uint4 end_record_no)
         { }

         ////////////////////////////////////////////////////////////
         // on_collection_started
         //
         // Called when the server has begun to collect the specified hole
         //////////////////////////////////////////////////////////// 
         virtual void on_collection_started(
            HoleLister *lister,
            StrUni const &table_name,
            uint4 begin_record_no,
            uint4 end_record_no)
         { }

         ////////////////////////////////////////////////////////////
         // on_collection_ended
         ////////////////////////////////////////////////////////////
         virtual void on_collection_ended(
            HoleLister *lister,
            StrUni const &table_name,
            uint4 begin_record_no,
            uint4 end_record_no)
         { }
      };

      ////////////////////////////////////////////////////////////
      // class HoleLister
      //
      // Defines an object that is able to connect to a device object on the
      // server and enumerate the holes that might exist on that datalogger
      // device. Note that this function is only supported on devices that
      // support data advise (BMP1 based dataloggers).
      //
      // A client can use this class be creating an instance of it, invoking
      // methods to set properties, and then invoking start(). If all goes
      // well, the lister object will begin sending notifications to bring the
      // client up to date and will then invoke the client's on_started()
      // method. Following that, notification methods will be invoked whenever
      // a change in hole collection status is detected. This will continue
      // until the session is broken or the client invokes finish().
      //
      // If a failure occurs at any time after start() is invoked, the lister
      // object will invoke the client's on_failure() method and will return to
      // a standby (startable) state.
      ////////////////////////////////////////////////////////////
      class HoleLister: public DeviceBase, public Csi::EvReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         HoleLister();

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~HoleLister();

         ////////////////////////////////////////////////////////////
         // start
         //////////////////////////////////////////////////////////// 
         typedef HoleListerClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         //////////////////////////////////////////////////////////// 
         void finish();

      protected:
         //@group methods derived from class DeviceBase
         ////////////////////////////////////////////////////////////
         // onNetMessage
         //////////////////////////////////////////////////////////// 
         virtual void onNetMessage(Csi::Messaging::Router *rtr,
                                   Csi::Messaging::Message *msg);

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
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         //////////////////////////////////////////////////////////// 
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // on_start_ack
         //////////////////////////////////////////////////////////// 
         void on_start_ack(Csi::Messaging::Message *message);
         
         ////////////////////////////////////////////////////////////
         // on_advise_not
         //////////////////////////////////////////////////////////// 
         void on_advise_not(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_stopped_not
         //////////////////////////////////////////////////////////// 
         void on_stopped_not(Csi::Messaging::Message *message);
         
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
         HoleListerClient *client;
      };
   };
};

#endif
