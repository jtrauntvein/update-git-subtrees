/* Cora.Device.HoleLister.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 26 July 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.HoleLister.h"
#include <assert.h>


namespace Cora
{
   namespace Device
   {
      namespace HoleListerHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         //
         // Defines a base class for all event objects that will be posted by class HoleLister to
         // itself.
         class event_base: public Csi::Event
         {
         protected:
            HoleLister *lister;
            HoleListerClient *client;
            friend class Cora::Device::HoleLister;

         public:
            ////////// constructor
            event_base(uint4 event_id,
                       HoleLister *lister_,
                       HoleListerClient *client_):
               Event(event_id,lister_),
               lister(lister_),
               client(client_)
            { }

            ////////// notify
            virtual void notify() = 0;
         };

         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public event_base
         {
         public:
            static uint4 const event_id;
            static void create_and_post(HoleLister *lister, HoleListerClient *client);
            virtual void notify() { client->on_started(lister); }

         private:
            event_started(HoleLister *lister, HoleListerClient *client):
               event_base(event_id,lister,client)
            { }
         };

         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Device::HoleLister::event_started");

         void event_started::create_and_post(HoleLister *lister,
                                             HoleListerClient *client)
         {
            try { (new event_started(lister,client))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            ////////// event_id
            static uint4 const event_id;

            ////////// create_and_post
            typedef HoleListerClient::failure_type failure_type;
            static void create_and_post(HoleLister *lister,
                                        HoleListerClient *client,
                                        failure_type failure);

            ////////// notify
            virtual void notify() { client->on_failure(lister,failure); }

         private:
            ////////// failure
            failure_type failure;

            ////////// constructor
            event_failure(HoleLister *lister,
                          HoleListerClient *client,
                          failure_type failure_):
               event_base(event_id,lister,client),
               failure(failure_)
            { }
         }; 

         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Device::HoleLister::event_failure");

         void event_failure::create_and_post(HoleLister *lister,
                                             HoleListerClient *client,
                                             failure_type failure)
         {
            try { (new event_failure(lister,client,failure))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_hole_change
         ////////////////////////////////////////////////////////////
         class event_hole_change: public event_base
         {
         public:
            ////////// event_id
            static uint4 const event_id;

            ////////// create_and_post
            enum change_type
            {
               hole_added = 1,
               hole_collected = 2,
               hole_uncollectable = 3,
               hole_collect_started = 4,
               hole_collect_ended = 5
            };
            static void create_and_post(HoleLister *lister,
                                        HoleListerClient *client,
                                        StrUni const &table_name,
                                        uint4 begin_record_no,
                                        uint4 end_record_no,
                                        change_type change);

            ////////// notify
            virtual void notify();

         private:
            ////////// table_name
            StrUni table_name;

            ////////// begin_record_no
            uint4 begin_record_no;

            ////////// end_record_no
            uint4 end_record_no;

            ////////// change
            change_type change;

            ////////// constructor
            event_hole_change(HoleLister *lister,
                              HoleListerClient *client,
                              StrUni const &table_name_,
                              uint4 begin_record_no_,
                              uint4 end_record_no_,
                              change_type change_):
               event_base(event_id,lister,client),
               table_name(table_name_),
               begin_record_no(begin_record_no_),
               end_record_no(end_record_no_),
               change(change_)
            { }
         };

         uint4 const event_hole_change::event_id =
         Csi::Event::registerType("Cora::Device::HoleLister::event_hole_change");

         void event_hole_change::create_and_post(HoleLister *lister,
                                                 HoleListerClient *client,
                                                 StrUni const &table_name,
                                                 uint4 begin_record_no,
                                                 uint4 end_record_no,
                                                 change_type change)
         {
            try
            {
               (new event_hole_change(lister,
                                      client,
                                      table_name,
                                      begin_record_no,
                                      end_record_no,
                                      change))->post();
            }
            catch(Csi::Event::BadPost &)
            { }
         } // create_and_post


         void event_hole_change::notify()
         {
            switch(change)
            {
            case hole_added:
               client->on_hole_added(lister,table_name,begin_record_no,end_record_no);
               break;
               
            case hole_collected:
               client->on_hole_collected(lister,table_name,begin_record_no,end_record_no);
               break;
               
            case hole_uncollectable:
               client->on_hole_uncollectable(lister,table_name,begin_record_no,end_record_no);
               break;
               
            case hole_collect_started:
               client->on_collection_started(lister,table_name,begin_record_no,end_record_no);
               break;

            case hole_collect_ended:
               client->on_collection_ended(lister,table_name,begin_record_no,end_record_no);
               break; 
            }
         } // notify
      };

      
      ////////////////////////////////////////////////////////////
      // class HoleLister definitions
      ////////////////////////////////////////////////////////////
      HoleLister::HoleLister():
         state(state_standby),
         client(0)
      { }

      
      HoleLister::~HoleLister()
      { finish(); }

      
      void HoleLister::start(
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
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void HoleLister::start(
         HoleListerClient *client_,
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
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void HoleLister::finish()
      {
         client = 0;
         state = state_standby;
         DeviceBase::finish();
      } // finish

      
      void HoleLister::onNetMessage(
         Csi::Messaging::Router *rtr, 
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::hole_advise_start_ack:
               on_start_ack(msg);
               break;
               
            case Messages::hole_advise_not:
               on_advise_not(msg);
               break;
               
            case Messages::hole_advise_stopped_not:
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

      
      void HoleLister::on_devicebase_ready()
      {
         Csi::Messaging::Message start_command(
            device_session,
            Messages::hole_advise_start_cmd);
         
         state = state_active;
         start_command.addUInt4(++last_tran_no);
         start_command.addUInt4(UInt4_Max); // send as many records as possible each time
         router->sendMessage(&start_command);
      } // on_devicebase_ready

      
      void HoleLister::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace HoleListerHelpers;
         HoleListerClient::failure_type client_failure;
         switch(failure)
         {
         case devicebase_failure_logon:
            client_failure = HoleListerClient::failure_invalid_logon;
            break;
            
         case devicebase_failure_session:
            client_failure = HoleListerClient::failure_session_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            client_failure = HoleListerClient::failure_invalid_device_name;
            break;

         case devicebase_failure_security:
            client_failure = HoleListerClient::failure_security_blocked;
            break;
            
         case devicebase_failure_unsupported:
            client_failure = HoleListerClient::failure_not_supported;
            break; 
            
         default:
            client_failure = HoleListerClient::failure_unknown;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_devicebase_failure

      
      void HoleLister::on_devicebase_session_failure()
      {
         using namespace HoleListerHelpers;
         event_failure::create_and_post(this,client,HoleListerClient::failure_session_failed);
      } // on_devicebase_session_failure

      
      void HoleLister::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace HoleListerHelpers;
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            if(event->getType() == event_failure::event_id)
               finish();
            if(HoleListerClient::is_valid_instance(event->client))
               event->notify();
            else
               finish();
         }
         else
            assert(false);
      } // receive


      void HoleLister::on_start_ack(Csi::Messaging::Message *message)
      {
         using namespace HoleListerHelpers;
         event_started::create_and_post(this,client);
      } // on_start_ack


      void HoleLister::on_advise_not(Csi::Messaging::Message *message)
      {
         // process the message
         uint4 tran_no;
         uint4 count;
         StrUni table_name;
         uint4 begin_record_no;
         uint4 end_record_no;
         uint4 event_type;
         using namespace HoleListerHelpers;

         message->readUInt4(tran_no);
         message->readUInt4(count);
         for(uint4 i = 0; i < count; ++i)
         {
            message->readUInt4(event_type);
            message->readWStr(table_name);
            message->readUInt4(begin_record_no);
            message->readUInt4(end_record_no);
            event_hole_change::create_and_post(
               this,
               client,
               table_name,
               begin_record_no,
               end_record_no,
               static_cast<event_hole_change::change_type>(event_type));
         }

         // send a comand to continue the transaction
         Csi::Messaging::Message cont_command(
            device_session,
            Messages::hole_advise_cont_cmd);
         cont_command.addUInt4(tran_no);
         router->sendMessage(&cont_command);
      } // on_advise_not

      
      void HoleLister::on_stopped_not(Csi::Messaging::Message *message)
      {
         using namespace HoleListerHelpers;
         event_failure::create_and_post(this,client,HoleListerClient::failure_unknown);
      } // on_stopped_not 
   };
};
