/* Cora.Device.CollectArea.FsAreaCreator.h

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 20 December 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_CollectArea_FsAreaCreator_h
#define Cora_Device_CollectArea_FsAreaCreator_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         //@group class forward declarations
         class FsAreaCreator;
         //@endgroup


         ////////////////////////////////////////////////////////////
         // class FsAreaCreatorClient
         ////////////////////////////////////////////////////////////
         class FsAreaCreatorClient: public Csi::InstanceValidator
         {
         public:
            ////////////////////////////////////////////////////////////
            // on_started
            //
            // Called when the collect area has been created.  
            ////////////////////////////////////////////////////////////
            virtual void on_started(
               FsAreaCreator *creator,
               StrUni const &collect_area_name)
            { }

            ////////////////////////////////////////////////////////////
            // on_failure
            //
            // Called when the transaction has ended.  This can happen the device is deleted or the
            // server is shut down.  This method will also be invoked if the area could not be
            // created or the transaction failed for any other reason.
            ////////////////////////////////////////////////////////////
            enum failure_type
            {
               failure_unknown = 0,
               failure_invalid_logon = 1,
               failure_session_failed = 2,
               failure_security_blocked = 3,
               failure_unsupported = 4,
               failure_invalid_device_name = 5,
               failure_invalid_area_name = 6,
               failure_invalid_area_id = 7,
            };
            virtual void on_failure(
               FsAreaCreator *creator,
               failure_type failure) = 0;
         };


         ////////////////////////////////////////////////////////////
         // class FsAreaCreator
         //
         // Defines a coratools component that can create a classic logger final storage area on a
         // given device and maintain its presence if created as a temporary.
         //
         // In order to use this component, an application must provide an object derived from class
         // FsAreaCreatorClient, create an instance of this class and invoke appropriate methods
         // such as set_device_name(), set_collect_area_name(), and etc.  When the setup is
         // finished, the application can call one of the two versions of start().
         //
         // If the area could be created by the server, the component will invoke the client's
         // on_started() method and will then enter a state where it is maintaining its session with
         // the server device.  This is needed because the area can be created as a temporary area
         // that is destroyed when the session that created it is lost.  The component can be
         // brought out of this state by deleting it, or by calling finish().  The component will
         // also automatically leave the maintenance state when the device is deleted or the server
         // is shut down.  If one of the other terminating conditions occurs, the client's
         // on_failure() method will be invoked.
         ////////////////////////////////////////////////////////////
         class FsAreaCreator:
            public DeviceBase,
            public Csi::EventReceiver
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            FsAreaCreator();

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~FsAreaCreator();

            ////////////////////////////////////////////////////////////
            // set_collect_area_name
            ////////////////////////////////////////////////////////////
            void set_collect_area_name(StrUni const &collect_area_name);

            ////////////////////////////////////////////////////////////
            // get_collect_area_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_collect_area_name() const
            { return collect_area_name; }

            ////////////////////////////////////////////////////////////
            // set_is_permanent
            ////////////////////////////////////////////////////////////
            void set_is_permanent(bool is_permanent);

            ////////////////////////////////////////////////////////////
            // get_is_permanent
            ////////////////////////////////////////////////////////////
            bool get_is_permanent() const
            { return is_permanent; }

            ////////////////////////////////////////////////////////////
            // set_area_id
            ////////////////////////////////////////////////////////////
            void set_area_id(uint4 area_id_);

            ////////////////////////////////////////////////////////////
            // get_area_id
            ////////////////////////////////////////////////////////////
            uint4 get_area_id() const
            { return area_id; }

            ////////////////////////////////////////////////////////////
            // set_make_unique_name
            ////////////////////////////////////////////////////////////
            void set_make_unique_name(bool make_unique_name);

            ////////////////////////////////////////////////////////////
            // get_make_unique_name
            ////////////////////////////////////////////////////////////
            bool get_make_unique_name() const
            { return make_unique_name; }

            ////////////////////////////////////////////////////////////
            // start
            ////////////////////////////////////////////////////////////
            typedef FsAreaCreatorClient client_type;
            void start(
               client_type *client_,
               router_handle &router_);
            void start(
               client_type *client_,
               ClientBase *other_component);

            ////////////////////////////////////////////////////////////
            // finish
            ////////////////////////////////////////////////////////////
            virtual void finish();

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
               state_active,
            } state;

            ////////////////////////////////////////////////////////////
            // collect_area_name
            ////////////////////////////////////////////////////////////
            StrUni collect_area_name;

            ////////////////////////////////////////////////////////////
            // is_permanent
            ////////////////////////////////////////////////////////////
            bool is_permanent;

            ////////////////////////////////////////////////////////////
            // area_id
            ////////////////////////////////////////////////////////////
            uint4 area_id;

            ////////////////////////////////////////////////////////////
            // make_unique_name
            ////////////////////////////////////////////////////////////
            bool make_unique_name;
         };
      };
   };
};


#endif
