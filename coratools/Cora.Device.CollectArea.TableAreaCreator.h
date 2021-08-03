/* Cora.Device.CollectArea.TableAreaCreator.h

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 17 December 2004
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_CollectArea_TableAreaCreator_h
#define Cora_Device_CollectArea_TableAreaCreator_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         //@group class forward declarations
         class TableAreaCreator;
         //@endgroup


         ////////////////////////////////////////////////////////////
         // class TableAreaCreatorClient
         ////////////////////////////////////////////////////////////
         class TableAreaCreatorClient: public Csi::InstanceValidator
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
               outcome_invalid_area_name = 6,
               outcome_unsupported = 7 
            };
            virtual void on_complete(
               TableAreaCreator *creator,
               outcome_type outcome) = 0;
         };


         ////////////////////////////////////////////////////////////
         // class TableAreaCreator
         ////////////////////////////////////////////////////////////
         class TableAreaCreator:
            public DeviceBase,
            public Csi::EventReceiver
         {
         private:
            ////////////////////////////////////////////////////////////
            // area_name
            ////////////////////////////////////////////////////////////
            StrUni area_name;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            TableAreaCreatorClient *client;

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
            TableAreaCreator():
               client(0),
               state(state_standby)
            { }

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~TableAreaCreator()
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
            typedef TableAreaCreatorClient client_type;
            void start(
               client_type *client_,
               router_handle &router)
            {
               if(state != state_standby)
                  throw exc_invalid_state();
               if(!client_type::is_valid_instance(client_))
                  throw std::invalid_argument("Invalid client pointer");
               if(area_name.length() == 0)
                  throw std::invalid_argument("Invalid area name");
               state = state_delegate;
               client = client_;
               DeviceBase::start(router);
            }

            ////////////////////////////////////////////////////////////
            // start (other component)
            ////////////////////////////////////////////////////////////
            void start(
               client_type *client_,
               ClientBase *other_component)
            {
               if(state != state_standby)
                  throw exc_invalid_state();
               if(!client_type::is_valid_instance(client_))
                  throw std::invalid_argument("Invalid client pointer");
               if(area_name.length() == 0)
                  throw std::invalid_argument("Invalid area name");
               state = state_delegate;
               client = client_;
               DeviceBase::start(other_component);
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
            // on_devicebase_ready
            ////////////////////////////////////////////////////////////
            virtual void on_devicebase_ready();

            ////////////////////////////////////////////////////////////
            // onNetMessage
            ////////////////////////////////////////////////////////////
            virtual void onNetMessage(
               Csi::Messaging::Router *rtr,
               Csi::Messaging::Message *msg);

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
