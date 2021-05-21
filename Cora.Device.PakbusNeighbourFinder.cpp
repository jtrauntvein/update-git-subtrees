/* Cora.Device.PakbusNeighbourFinder.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 02 December 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.PakbusNeighbourFinder.h"


namespace Cora
{
   namespace Device
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         protected:
            ////////////////////////////////////////////////////////////
            // finder
            ////////////////////////////////////////////////////////////
            typedef PakbusNeighbourFinder finder_type;
            finder_type *finder;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef PakbusNeighbourFinderClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               finder_type *finder_,
               client_type *client_):
               Event(event_id,finder_),
               finder(finder_),
               client(client_)
            { }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify() = 0;

         public: 
            ////////////////////////////////////////////////////////////
            // do_notify
            ////////////////////////////////////////////////////////////
            void do_notify()
            {
               if(client_type::is_valid_instance(client))
                  notify();
            } 
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               finder_type *finder,
               client_type *client)
            {
               try{(new event_started(finder,client))->post();}
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_started(finder); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               finder_type *finder,
               client_type *client):
               event_base(event_id,finder,client)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Device::PakbusNeighbourFinder::event_started");


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef client_type::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               finder_type *finder,
               client_type *client,
               failure_type failure)
            {
               try{(new event_failure(finder,client,failure))->post();}
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(finder,failure); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               finder_type *finder,
               client_type *client,
               failure_type failure_):
               event_base(event_id,finder,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Device::PakbusNeighbourFinder::event_failure");
      };


      ////////////////////////////////////////////////////////////
      // class PakbusNeighbourFinder definitions
      ////////////////////////////////////////////////////////////
      void PakbusNeighbourFinder::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void PakbusNeighbourFinder::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start
      
      
      void PakbusNeighbourFinder::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            if(event->getType() == event_failure::event_id)
               finish();
            event->do_notify();
         }
      } // receive

      
      void PakbusNeighbourFinder::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::find_pakbus_neighbours_start_ack:
               event_started::cpost(this,client);
               break;
               
            case Messages::find_pakbus_neighbours_stopped_not:
               on_stopped_not(msg);
               break;
               
            default:
               DeviceBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage
      
      
      void PakbusNeighbourFinder::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Messages::find_pakbus_neighbours_start_cmd);
         cmd.addUInt4(++last_tran_no);
         router->sendMessage(&cmd);
         state = state_active; 
      } // on_devicebase_ready
      
      
      void PakbusNeighbourFinder::on_devicebase_failure(devicebase_failure_type failure_)
      {
         client_type::failure_type failure;
         switch(failure_)
         {
         case devicebase_failure_logon:
            failure = client_type::failure_invalid_logon;
            break;
                     
         case devicebase_failure_session:
            failure = client_type::failure_connection_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            failure = client_type::failure_invalid_device_name;
            break;
            
         default:
            failure = client_type::failure_unknown;
            break;
         }
         event_failure::cpost(this,client,failure);
      } // on_devicebase_failure
      
      
      void PakbusNeighbourFinder::on_devicebase_session_failure()
      {
         event_failure::cpost(this,client,client_type::failure_connection_failed);
      } // on_devicebase_session_failure


      void PakbusNeighbourFinder::on_stopped_not(
         Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 reason;
         client_type::failure_type failure;
         
         message->readUInt4(tran_no);
         message->readUInt4(reason);
         switch(reason)
         {
         case 3:
            failure = client_type::failure_communication_disabled;
            break;
            
         case 4:
            failure = client_type::failure_link_failed;
            break;
            
         case 5:
            failure = client_type::failure_other_transaction;
            break;
            
         default:
            failure = client_type::failure_unknown;
            break;
         }
         event_failure::cpost(this,client,failure);
      } // on_stopped_not
   };
};
