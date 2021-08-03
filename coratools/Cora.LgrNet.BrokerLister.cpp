/* Cora.LgrNet.BrokerLister.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 28 July 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.BrokerLister.h"
#include <assert.h>


namespace Cora
{
   namespace LgrNet
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         //
         // Defines a base class for all events posted by class BrokerLister to itself.
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         protected:
            ////////////////////////////////////////////////////////////
            // lister
            ////////////////////////////////////////////////////////////
            BrokerLister *lister;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef BrokerLister::client_type client_type;
            client_type *client;
            friend class Cora::LgrNet::BrokerLister;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               BrokerLister *lister_,
               client_type *client_):
               Event(event_id,lister_),
               lister(lister_),
               client(client_)
            { }

         public:
            virtual void notify() = 0;
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
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(BrokerLister *lister,
                                        BrokerListerClient *client);

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_started(lister); }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(BrokerLister *lister,
                          BrokerListerClient *client):
               event_base(event_id,lister,client)
            { }
         };

         
         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::LgrNet::BrokerLister::event_started");


         void event_started::create_and_post(BrokerLister *lister,
                                             BrokerListerClient *client)
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
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef BrokerListerClient::failure_type failure_type;
            failure_type failure;
            
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(BrokerLister *lister,
                                        BrokerListerClient *client,
                                        failure_type failure);

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(lister,failure); }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(BrokerLister *lister,
                          BrokerListerClient *client,
                          failure_type failure_):
               event_base(event_id,lister,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::LgrNet::BrokerLister::event_failure");


         void event_failure::create_and_post(BrokerLister *lister,
                                             BrokerListerClient *client,
                                             failure_type failure)
         {
            try { (new event_failure(lister,client,failure))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_change
         ////////////////////////////////////////////////////////////
         class event_change: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // change
            ////////////////////////////////////////////////////////////
            enum change_type
            {
               change_added,
               change_deleted,
               change_renamed,
            } change;
            
            ////////////////////////////////////////////////////////////
            // broker_name
            ////////////////////////////////////////////////////////////
            StrUni broker_name;

            ////////////////////////////////////////////////////////////
            // old_broker_name
            ////////////////////////////////////////////////////////////
            StrUni old_broker_name;

            ////////////////////////////////////////////////////////////
            // broker_type
            ////////////////////////////////////////////////////////////
            typedef BrokerListerClient::broker_type_code broker_type_code;
            broker_type_code broker_type;

            ////////////////////////////////////////////////////////////
            // broker_id
            ////////////////////////////////////////////////////////////
            uint4 broker_id;
            
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(BrokerLister *lister,
                                        BrokerListerClient *client,
                                        change_type change,
                                        uint4 broker_id,
                                        broker_type_code broker_type,
                                        StrUni const &broker_name,
                                        StrUni const &old_broker_name = "");
            
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               switch(change)
               {
               case change_added:
                  client->on_broker_added(
                     lister,
                     broker_name,
                     broker_id,
                     broker_type);
                  break;
                  
               case change_deleted:
                  client->on_broker_deleted(
                     lister,
                     broker_name,
                     broker_id,
                     broker_type);
                  break;
                  
               case change_renamed:
                  client->on_broker_renamed(
                     lister,
                     old_broker_name,
                     broker_name,
                     broker_id,
                     broker_type);
                  break;
                  
               default:
                  assert(false);
                  break;
               }
            }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_change(BrokerLister *lister,
                         BrokerListerClient *client,
                         change_type change_,
                         uint4 broker_id_,
                         broker_type_code broker_type_,
                         StrUni const &broker_name_,
                         StrUni const &old_broker_name_):
               event_base(event_id,lister,client),
               change(change_),
               broker_id(broker_id_),
               broker_type(broker_type_),
               broker_name(broker_name_),
               old_broker_name(old_broker_name_)
            { }
         };

         uint4 const event_change::event_id =
         Csi::Event::registerType("Cora::LgrNet::BrokerLister::event_change");


         void event_change::create_and_post(BrokerLister *lister,
                                            BrokerListerClient *client,
                                            change_type change,
                                            uint4 broker_id,
                                            broker_type_code broker_type,
                                            StrUni const &broker_name,
                                            StrUni const &old_broker_name)
         {
            try
            {
               (new event_change(
                  lister,
                  client,
                  change,
                  broker_id,
                  broker_type,
                  broker_name,
                  old_broker_name))->post();
            }
            catch(Csi::Event::BadPost &)
            { }
         } // create_and_post
      };


      ////////////////////////////////////////////////////////////
      // class BrokerLister definitions
      ////////////////////////////////////////////////////////////
      BrokerLister::BrokerLister():
         client(0),
         state(state_standby),
         broker_mask(broker_mask_active | broker_mask_statistics)
      { }


      BrokerLister::~BrokerLister()
      { finish(); }

      
      void BrokerLister::add_broker_type(broker_mask_type mask)
      { set_broker_mask(broker_mask | mask); }

      
      void BrokerLister::remove_broker_type(broker_mask_type mask)
      { set_broker_mask(broker_mask & (~mask)); }

      
      void BrokerLister::set_broker_mask(byte broker_mask_)
      {
         if(state == state_standby)
            broker_mask = broker_mask_;
         else
            throw exc_invalid_state();
      } // set_broker_mask

      
      void BrokerLister::start(
         BrokerListerClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            state = state_delegate;
            client = client_;
            ClientBase::start(router);
         }
         else
            throw exc_invalid_state();
      } // start


      void BrokerLister::start(
         BrokerListerClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            state = state_delegate;
            client = client_;
            ClientBase::start(other_component);
         }
         else
            throw exc_invalid_state();
      } // start

      
      void BrokerLister::finish()
      {
         brokers.clear();
         state = state_standby;
         client = 0;
         ClientBase::finish();
      } // finish

      
      void BrokerLister::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_before_active || state == state_active)
         {
            if(msg->getMsgType() == LgrNet_DataBrokersEnumNot)
               on_enum_not(msg);
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void BrokerLister::on_corabase_ready()
      {
         Csi::Messaging::Message message(net_session,LgrNet_DataBrokersEnumCmd);
         
         state = state_before_active;
         message.addUInt4(++last_tran_no);
         router->sendMessage(&message);
      } // on_corabase_ready

      
      void BrokerLister::on_corabase_failure(corabase_failure_type failure)
      {
         // map the failure into one understood by the client
         client_type::failure_type client_failure = client_type::failure_unknown;
         switch(failure)
         {
         case corabase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case corabase_failure_session:
            client_failure = client_type::failure_connection_failed;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_corabase_failure

      
      void BrokerLister::on_corabase_session_failure()
      {
         event_failure::create_and_post(this,client,client_type::failure_connection_failed);
      } // on_corabase_session_failure

      
      void BrokerLister::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());

         if(event)
         {
            if(event->getType() == event_failure::event_id)
               finish();
            if(client_type::is_valid_instance(event->client))
               event->notify();
         }
      } // receive


      void BrokerLister::on_enum_not(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 resp_code;
         uint4 count;

         message->readUInt4(tran_no);
         message->readUInt4(resp_code);
         message->readUInt4(count);
         if(resp_code == 1)
         {
            // read the broker parameters
            uint4 op_code;
            uint4 broker_id;
            uint4 type;
            StrUni name;
            StrUni old_name;

            for(uint4 i = 0; i < count; ++i)
            {
               message->readUInt4(op_code);
               message->readUInt4(broker_id);
               message->readUInt4(type);
               message->readWStr(name);
               
               // determine what to do with the parameters
               bool announce_change;
               switch(type)
               {
               case Broker::Type::active:
                  announce_change = (broker_mask & broker_mask_active);
                  break;
                  
               case Broker::Type::backup:
                  announce_change = (broker_mask & broker_mask_backup) != 0;
                  break;
                  
               case Broker::Type::client_defined:
                  announce_change = (broker_mask & broker_mask_client_defined) != 0;
                  break;
                  
               case Broker::Type::statistics:
                  announce_change = (broker_mask & broker_mask_statistics) != 0;
                  break;
                  
               default:
                  announce_change = false;
                  break;
               }

               // we need to maintain our own internal list of broker names to handle the rename
               // event.
               enum op_code_type {
                  broker_added = 1,
                  broker_renamed = 2,
                  broker_deleted = 3,
               };
               if(op_code == broker_added)
                  brokers[broker_id] = name;
               else if(op_code == broker_deleted)
                  brokers.erase(broker_id);
               
               // announce the change
               if(announce_change)
               {
                  event_change::change_type change;
                  
                  switch(op_code)
                  {
                  case broker_added:
                     change = event_change::change_added;
                     break;
                     
                  case broker_deleted:
                     change = event_change::change_deleted;
                     break;

                  case broker_renamed:
                     old_name = brokers[broker_id];
                     brokers[broker_id] = name;
                     change = event_change::change_renamed;
                     break;
                     
                  default:
                     announce_change = false;
                     break;
                  }
                  if(announce_change)
                  {
                     event_change::create_and_post(
                        this,
                        client,
                        change,
                        broker_id,
                        static_cast<client_type::broker_type_code>(type),
                        name,
                        old_name);
                  }
               }
            }

            // if this is the first notification, we can now announce that the transaction has
            // entered a steady state
            if(state == state_before_active)
            {
               state = state_active;
               event_started::create_and_post(this,client);
            }
         }
         else
            event_failure::create_and_post(this,client,client_type::failure_unknown);
      } // on_enum_not
   };
};
