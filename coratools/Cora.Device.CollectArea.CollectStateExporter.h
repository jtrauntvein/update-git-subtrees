/* Cora.Device.CollectArea.CollectStateExporter.h

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 16 December 2004
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_CollectArea_CollectStateExporter_h
#define Cora_Device_CollectArea_CollectStateExporter_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.RangeList.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         //@group class forward declarations
         class CollectStateExporter;
         //@endgroup


         ////////////////////////////////////////////////////////////
         // class CollectStateExporterClient
         ////////////////////////////////////////////////////////////
         class CollectStateExporterClient: public Csi::InstanceValidator
         {
         public:
            ////////////////////////////////////////////////////////////
            // on_complete
            ////////////////////////////////////////////////////////////
            enum outcome_type
            {
               outcome_unknown = 0,
               outcome_success = 1,
               outcome_connection_failed = 2,
               outcome_invalid_logon = 3,
               outcome_server_security_blocked = 4,
               outcome_invalid_device_name = 5,
               outcome_invalid_collect_area_name = 6,
               outcome_invalid_collect_area_type = 7
            };
            virtual void on_complete(
               CollectStateExporter *exporter,
               outcome_type outcome,
               Csi::RangeList const &records_collected,
               Csi::RangeList const &records_expected) = 0;
         };

         
         ////////////////////////////////////////////////////////////
         // class CollectStateExporter
         ////////////////////////////////////////////////////////////
         class CollectStateExporter:
            public DeviceBase,
            public Csi::EventReceiver
         {
         private:
            //@group parameters
            ////////////////////////////////////////////////////////////
            // area_name
            ////////////////////////////////////////////////////////////
            StrUni area_name;
            //@endgroup
            
            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            CollectStateExporterClient *client;
            
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
            CollectStateExporter():
               client(0),
               state(state_standby)
            { }
            
            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~CollectStateExporter()
            { finish(); }
            
            ////////////////////////////////////////////////////////////
            // set_area_name
            ////////////////////////////////////////////////////////////
            void set_area_name(StrUni const &area_name_)
            {
               if(state == state_standby)
                  area_name = area_name_;
               else
                  throw exc_invalid_state();
            }

            ////////////////////////////////////////////////////////////
            // get_area_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_area_name() const
            { return area_name; }
            
            ////////////////////////////////////////////////////////////
            // start (new router)
            ////////////////////////////////////////////////////////////
            typedef CollectStateExporterClient client_type;
            void start(
               client_type *client_,
               router_handle &router)
            {
               if(state == state_standby)
               {
                  if(client_type::is_valid_instance(client_))
                  {
                     if(area_name.length() > 0)
                     {
                        state = state_delegate;
                        client = client_;
                        DeviceBase::start(router);
                     }
                     else
                        throw std::invalid_argument("Invalid collect area name");
                  }
                  else
                     throw std::invalid_argument("Invalid client pointer");
               }
               else
                  throw exc_invalid_state();
            }
            
            
            ////////////////////////////////////////////////////////////
            // start (other component)
            ////////////////////////////////////////////////////////////
            void start(
               client_type *client_,
               ClientBase *other_component)
            {
               if(state == state_standby)
               {
                  if(client_type::is_valid_instance(client_))
                  {
                     if(area_name.length() > 0)
                     {
                        state = state_delegate;
                        client = client_;
                        DeviceBase::start(other_component);
                     }
                     else
                        throw std::invalid_argument("Invalid collect area name");
                  }
                  else
                     throw std::invalid_argument("Invalid client pointer");
               }
               else
                  throw exc_invalid_state();            
            }
            
            
            ////////////////////////////////////////////////////////////
            // finish
            ////////////////////////////////////////////////////////////
            virtual void finish()
            {
               client = 0;
               state = state_standby;
               DeviceBase::finish();
            }
            
         protected:
            ////////////////////////////////////////////////////////////
            // receive
            ////////////////////////////////////////////////////////////
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
            
            ////////////////////////////////////////////////////////////
            // onNetMessage
            ////////////////////////////////////////////////////////////
            virtual void onNetMessage(
               Csi::Messaging::Router *rtr,
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
            virtual void on_devicebase_session_failure()
            { on_devicebase_failure(devicebase_failure_session); }
         };
      };
   };
};


#endif
