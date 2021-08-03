/*  Cora.Device.CollectArea.InlocsAreaCreator.h

    Copyright (C) 2000, 2009 Campbell Scientific, Inc.

    Written By: Tyler Mecham
    Date Begun: Friday 28 September 2001

    Last Changed By: $Author: jon $
    Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
    File Revision Number: $Revision: 7127 $

*/

#ifndef Cora_Device_CollectArea_InlocsAreaCreator_h
#define Cora_Device_CollectArea_InlocsAreaCreator_h


#include "Cora.Device.DeviceBase.h"
#include "Csi.Messaging.Router.h"
#include <list>


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         //@forward declarations
         class InlocsAreaCreator;
         //@end forward declarations

         ////////////////////////////////////////////////////////////
         // class InlocId
         ////////////////////////////////////////////////////////////
         class InlocId
         {
         public:
            ////////////////////////////////////////////////////////////
            // default constructor
            ////////////////////////////////////////////////////////////
            InlocId():
               inloc_id(0)
            { }

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            InlocId(uint2 id,StrUni const &name)
            {
               inloc_id = id;
               field_name = name;
            }

            ////////////////////////////////////////////////////////////
            // copy constructor
            ////////////////////////////////////////////////////////////
            InlocId(InlocId const &other)
            { operator =(other); }


            ////////////////////////////////////////////////////////////
            // copy operator
            ////////////////////////////////////////////////////////////
            InlocId &operator =(InlocId const &other)
            {
               inloc_id = other.inloc_id;
               field_name = other.field_name;
               return *this;
            } // copy operator

            ////////////////////////////////////////////////////////////
            // inloc_id
            ////////////////////////////////////////////////////////////
            uint2 inloc_id;

            ////////////////////////////////////////////////////////////
            // field_name
            ////////////////////////////////////////////////////////////
            StrUni field_name;
         };

         
         ////////////////////////////////////////////////////////////
         // class InlocsAreaCreatorClient
         ////////////////////////////////////////////////////////////
         class InlocsAreaCreatorClient: 
            public Csi::InstanceValidator
         {
         public:
            ////////////////////////////////////////////////////////////
            // on_complete
            //
            // Called after the add log message attempt has completed. The
            // outcome parameter will indicate whether the attempt
            // succeeded.
            ////////////////////////////////////////////////////////////
            enum outcome_type
            {
               outcome_unknown = 0,
               outcome_success = 1,
               outcome_invalid_tran_no = 2,
               outcome_invalid_collect_area_name = 3,
               outcome_invalid_number_of_identifiers = 4,
               outcome_invalid_inloc_id = 5,
               outcome_invalid_field_name = 6,
               outcome_session_failed = 7,
               outcome_invalid_logon = 8,
               outcome_server_security_blocked = 9,
               outcome_communication_failed = 10,
               outcome_communication_disabled = 11,
               outcome_logger_security_blocked = 12,
               outcome_invalid_device_name = 13,
               outcome_unsupported = 14,
               outcome_cancelled = 15,
               outcome_device_busy = 16,
            };
            virtual void on_complete(InlocsAreaCreator *creator,outcome_type outcome) = 0;
         };

            
         ////////////////////////////////////////////////////////////
         // class InlocsAreaCreator
         //
         // Defines a component that allows an application to create a
         // custom input locations collect area for classic dataloggers.
         // In order to use this component, the application must provide a
         // client object derived from class InlocsAreaCreatorClient.  It
         // must then create an instance of this class, call the
         // appropriate methods to set component properties, and then
         // invoke start().  Once the server transaction has been carried
         // out, the component will invoke the client's on_complete()
         // method to indicate that the area has been created.  It will
         // then enter into a state where the device session is maintained
         // until finish() is called or the component destroyed.  The
         // device session is maintained so that temporary areas can be
         // tied to the life of the component.  
         ////////////////////////////////////////////////////////////
         class InlocsAreaCreator: 
            public DeviceBase, 
            public Csi::EventReceiver
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            InlocsAreaCreator();

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~InlocsAreaCreator();

            ////////////////////////////////////////////////////////////
            // set_temporary
            ////////////////////////////////////////////////////////////
            void set_temporary(bool temporary_);

            ////////////////////////////////////////////////////////////
            // set_collect_area_name
            ////////////////////////////////////////////////////////////
            void set_collect_area_name(StrUni const &area_name_);

            ////////////////////////////////////////////////////////////
            // get_collect_area_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_collect_area_name() const
            { return area_name; }

            ////////////////////////////////////////////////////////////
            // add_field
            ////////////////////////////////////////////////////////////
            void add_field(uint2 inloc_number, StrUni const &field_name);

            ////////////////////////////////////////////////////////////
            // clear_fields
            ////////////////////////////////////////////////////////////
            void clear_fields();

            ////////////////////////////////////////////////////////////
            // set_make_unique_name
            ////////////////////////////////////////////////////////////
            void set_make_unique_name(bool make_unique_name_);

            ////////////////////////////////////////////////////////////
            // get_make_unique_name
            ////////////////////////////////////////////////////////////
            bool get_make_unique_name() const
            { return make_unique_name; }
            
            ////////////////////////////////////////////////////////////
            // start
            ////////////////////////////////////////////////////////////
            typedef InlocsAreaCreatorClient client_type;
            void start(
               client_type *client_,
               router_handle &router);
            void start(
               client_type *client,
               ClientBase *other_component);
                
            ////////////////////////////////////////////////////////////
            // finish
            ////////////////////////////////////////////////////////////
            void finish();

         protected:
            //@group events to derived classes
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
               state_standby = 0,
               state_attaching,
               state_steady,
            } state;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            client_type *client;
                
            ////////////////////////////////////////////////////////////
            // area_name
            ////////////////////////////////////////////////////////////
            StrUni area_name;

            ////////////////////////////////////////////////////////////
            // temporary
            //
            // Set to true if the life of the area created should be tied
            // to the life of this component
            ////////////////////////////////////////////////////////////
            bool temporary;

            ////////////////////////////////////////////////////////////
            // locations
            ////////////////////////////////////////////////////////////
            typedef std::list<InlocId> locations_type;
            locations_type locations;

            ////////////////////////////////////////////////////////////
            // make_unique_name
            ////////////////////////////////////////////////////////////
            bool make_unique_name;
         };
      };
   };
};

#endif //Cora_Device_CollectArea_InlocsAreaCreator_h
