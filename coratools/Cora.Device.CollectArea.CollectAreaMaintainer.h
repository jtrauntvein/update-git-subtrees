/* File Name: $RCSfile: Cora.Device.CollectArea.CollectAreaMaintainer.h,v $

   Copyright (C) 2001, 2016 Campbell Scientific, Inc.

   Written By: Tyler Mecham
   Date Begun: 9/28/2001 14:32:35

   Last Changed By: $Author: jon $
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   File Revision Number: $Revision: 27879 $
*/

#ifndef Cora_Device_CollectArea_CollectAreaMaintainer_h
#define Cora_Device_CollectArea_CollectAreaMaintainer_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.Messaging.Router.h"
#include "Cora.Device.CollectArea.InlocsAreaCreator.h"
#include "Cora.Device.CollectArea.SettingSetter.h"
#include "OneShot.h"
#include <list>
#include <map>


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         //@begin forward declarations
         class CollectAreaMaintainer;
         //@endgroup

         ////////////////////////////////////////////////////////////
         // class CollectAreaMaintainerClient
         // receives events from the collect area maintainer when a collect
         // area setting is completed
         ////////////////////////////////////////////////////////////
         class CollectAreaMaintainerClient: 
            public Csi::InstanceValidator
         {
         public:
            ////////////////////////////////////////////////////////////
            // on_started
            //////////////////////////////////////////////////////////// 
            virtual void on_started(CollectAreaMaintainer *creator){};

            ////////////////////////////////////////////////////////////
            // on_failure
            ////////////////////////////////////////////////////////////
            enum failure_type
            {
               failure_unknown = 0,
               failure_invalid_logon = 1,
               failure_session_failed = 2,
               failure_invalid_device_name = 3,
               failure_unsupported = 4,
               failure_server_security_blocked = 5,
            };
            virtual void on_failure(
               CollectAreaMaintainer *creator,
               failure_type failure) = 0;
         };

            
         ////////////////////////////////////////////////////////////
         // class CollectAreaMaintainer
         // creates an inlocs collect area, enables it for scheduled
         // collection, and maintains the list of columns defined in
         // the collect area.
         ////////////////////////////////////////////////////////////
         class CollectAreaMaintainer:
            public InlocsAreaCreatorClient,
            public SettingSetterClient,
            public DeviceBase,
            public Csi::EvReceiver,
            public OneShotClient
         {
         public:
            ////////////////////////////////////////////////////////////
            // Constructor
            //////////////////////////////////////////////////////////// 
            CollectAreaMaintainer(Csi::SharedPtr<OneShot> oneshot = 0);

            ////////////////////////////////////////////////////////////
            // Destructor
            ////////////////////////////////////////////////////////////
            virtual ~CollectAreaMaintainer();

            ////////////////////////////////////////////////////////////
            // start
            ////////////////////////////////////////////////////////////
            void start(
               CollectAreaMaintainerClient *client_,
               router_handle &router);
            void start(
               CollectAreaMaintainerClient *client,
               ClientBase *other_component);
   
            ////////////////////////////////////////////////////////////
            // get_collect_area_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_collect_area_name()
            { return inlocs_area_creator->get_collect_area_name(); }

            ////////////////////////////////////////////////////////////
            // set_collect_area_name
            ////////////////////////////////////////////////////////////
            void set_collect_area_name(
               StrUni const &name,
               bool make_unique = false);

            ////////////////////////////////////////////////////////////
            // exists
            ////////////////////////////////////////////////////////////
            bool exists(
               uint2 inloc_number,
               StrUni const &field_name);

            ////////////////////////////////////////////////////////////
            // add_field
            //
            // returns the total number of locations after the add
            ////////////////////////////////////////////////////////////
            void add_field(
               uint2 inloc_number,
               StrUni const &field_name);

            ////////////////////////////////////////////////////////////
            // remove_field
            ////////////////////////////////////////////////////////////
            void remove_field(
               uint2 inloc_number, 
               StrUni const &field_name);

            ////////////////////////////////////////////////////////////
            // onNetMessage
            ////////////////////////////////////////////////////////////
            virtual void onNetMessage(
               Csi::Messaging::Router *rtr, 
               Csi::Messaging::Message *msg);

            ////////////////////////////////////////////////////////////
            // finish
            ////////////////////////////////////////////////////////////
            void finish();

            ////////////////////////////////////////////////////////////
            // get_field_count
            ////////////////////////////////////////////////////////////
            uint4 get_field_count()
            { return (uint4)locations.size(); }

         protected:
            //@group DeviceBase
            ////////////////////////////////////////////////////////////
            // on_devicebase_ready
            ////////////////////////////////////////////////////////////
            virtual void on_devicebase_ready();
            
            ////////////////////////////////////////////////////////////
            // on_devicebase_failure
            ////////////////////////////////////////////////////////////
            virtual void on_devicebase_failure(
               devicebase_failure_type failure);
            
            ////////////////////////////////////////////////////////////
            // on_devicebase_session_failure
            ////////////////////////////////////////////////////////////
            virtual void on_devicebase_session_failure();
            //@endgroup

            //@group InlocsAreaCreatorClient
            ////////////////////////////////////////////////////////////
            // on_complete
            ////////////////////////////////////////////////////////////
            virtual void on_complete(
               Cora::Device::CollectArea::InlocsAreaCreator *creator,
               Cora::Device::CollectArea::InlocsAreaCreatorClient::outcome_type outcome);
            //@endgroup

            //@group SettingSetterClient
            ////////////////////////////////////////////////////////////
            // on_complete
            ////////////////////////////////////////////////////////////
            virtual void on_complete(
               Cora::Device::CollectArea::SettingSetter *setter,
               Cora::Device::CollectArea::SettingSetterClient::outcome_type resp_code);
            //@endgroup

            ////////////////////////////////////////////////////////////
            // send_field_names
            //
            // This is called to change the collect area setting defining
            // the input locations in the collect area
            ////////////////////////////////////////////////////////////
            void send_field_names();

            //@group OneShotClient
            ////////////////////////////////////////////////////////////
            // onOneShotFired
            ////////////////////////////////////////////////////////////
            virtual void onOneShotFired(uint4 id);
            //@endgroup
 
         private:
            ////////////////////////////////////////////////////////////
            // receive
            ////////////////////////////////////////////////////////////
            virtual void receive(
               Csi::SharedPtr<Csi::Event> &ev);

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
            // pending_inlocs_exist
            ////////////////////////////////////////////////////////////
            bool pending_inlocs_exist;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            CollectAreaMaintainerClient *client;

            ////////////////////////////////////////////////////////////
            // inlocs_area_creator
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<InlocsAreaCreator> inlocs_area_creator;
                
            ////////////////////////////////////////////////////////////
            // setting_setters
            ////////////////////////////////////////////////////////////
            std::list<Csi::SharedPtr<Cora::Device::CollectArea::SettingSetter> > setting_setters;

            ////////////////////////////////////////////////////////////
            // locations
            ////////////////////////////////////////////////////////////
            typedef std::list<InlocId> locations_type;
            locations_type locations;

            ////////////////////////////////////////////////////////////
            // ref_count
            //
            // Keep a reference count on the inloc_id so we know when it 
            // is safe to remove a column from a collect area.
            ////////////////////////////////////////////////////////////
            std::map<uint2,uint2> ref_counts;

            ////////////////////////////////////////////////////////////
            // send_settings_id
            ////////////////////////////////////////////////////////////
            uint4 send_settings_id;

            ////////////////////////////////////////////////////////////
            // oneshot
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<OneShot> oneshot;
         };
      };
   };
};

#endif //Cora_Device_CollectArea_CollectAreaMaintainer_h
