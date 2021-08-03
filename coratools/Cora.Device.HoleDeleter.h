/* Cora.Device.HoleDeleter.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 31 July 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_Device_HoleDeleter_h
#define Cora_Device_HoleDeleter_h


#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "CsiEvents.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class HoleDeleter;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class HoleDeleterClient
      //
      // Declares the interface for a client object to class HoleDeleter.
      ////////////////////////////////////////////////////////////
      class HoleDeleterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //////////////////////////////////////////////////////////// 
         // Called when the hole delete transaction has been completed.
         enum resp_code_type
         {
            resp_unknown = 0,
            resp_succeeded = 1,
            resp_unsupported = 2,
            resp_invalid_logon = 3,
            resp_security_blocked = 4,
            resp_invalid_device_name = 5,
            resp_session_lost = 6,
         };
         virtual void on_complete(HoleDeleter *deleter, resp_code_type resp_code) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class HoleDeleter
      //
      // Defines a high level client class that is responsible for deleting
      // holes associated with a device. The client is expected to implement
      // the HoleDeleterClient interface in order to use this class.
      //
      // The client can use this class by creating an instance of it (usually
      // on the heap), setting the appropriate properties (like device name),
      // and calling start(). When the server transaction has been completed,
      // the HoleDeleter object will invoke the client's on_complete()
      // method. Once this method has been invoked, the HoleDeleter object will
      // return to a standby state.
      ////////////////////////////////////////////////////////////
      class HoleDeleter:
         public Csi::EvReceiver,
         public Cora::Device::DeviceBase
      {
      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // table_name
         //
         // Controls which holes will be deleted when the delete_mode property
         // is set to delete_holes_for_table.
         //////////////////////////////////////////////////////////// 
         StrUni table_name;

         ////////////////////////////////////////////////////////////
         // newest_record_no
         ////////////////////////////////////////////////////////////
         uint4 newest_record_no;

         ////////////////////////////////////////////////////////////
         // send_newest_record_no
         ////////////////////////////////////////////////////////////
         bool send_newest_record_no;
         //@endgroup

         ////////////////////////////////////////////////////////////
         // client
         //////////////////////////////////////////////////////////// 
         HoleDeleterClient *client;

         ////////////////////////////////////////////////////////////
         // state
         //////////////////////////////////////////////////////////// 
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
         } state;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         HoleDeleter():
            newest_record_no(0),
            send_newest_record_no(false),
            client(0),
            state(state_standby)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~HoleDeleter()
         { finish(); }

         //@group property modifiers
         ////////////////////////////////////////////////////////////
         // set_table_name
         //
         // Sets both the delete_mode and table_name properties
         //////////////////////////////////////////////////////////// 
         void set_table_name(StrUni const &table_name_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            table_name = table_name_;
         }

         ////////////////////////////////////////////////////////////
         // set_newest_record_no
         ////////////////////////////////////////////////////////////
         void set_newest_record_no(
            uint4 newest_record_no_,
            bool send_newest_record_no_ = true)
         {
            if(state == state_standby)
            {
               newest_record_no = newest_record_no_;
               send_newest_record_no = send_newest_record_no_;
            }
            else
               throw exc_invalid_state();
         }
            
         //@endgroup

         ////////////////////////////////////////////////////////////
         // start
         //
         // This method is invoked to start the server hole deletion
         // transaction. Once the transaction has been completed, the client
         // object's on_complete() method will be invoked and this object will
         // return to a standby state. The client can cancel this notification
         // by invoking finish() or by deleting this object (which will also
         // cause finish() to be invoked.
         ////////////////////////////////////////////////////////////
         typedef HoleDeleterClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         //
         // Causes this object to cancel the client on_complete() notification
         // and also releases any server session resources specifially used by
         // this object. Note that this will not cause the server transaction
         // to be rolled back if this object is in an active state. It will
         // simply prevent the notification when the transaction completes.
         //////////////////////////////////////////////////////////// 
         void finish();

      protected:
         //@groups methods overloaded from class DeviceBase

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

         ////////////////////////////////////////////////////////////
         // onNetMessage
         //////////////////////////////////////////////////////////// 
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *message);
         
      private:
      };
   };
};

#endif
