/* Cora.LgrNet.BrokerBrowser.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 01 June 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.BrokerBrowser.h"
#include "Cora.Defs.h"
#include <assert.h>


namespace Cora
{
   namespace LgrNet
   {
      namespace BrokerBrowserHelpers
      {
         ////////////////////////////////////////////////////////////
         // class ev_failure declaration and definitions
         ////////////////////////////////////////////////////////////

         class ev_failure: public Csi::Event
         {
         public:
            static const uint4 event_id;
            BrokerBrowserClient *client;
            BrokerBrowserClient::failure_type failure;
            
         private:
            ev_failure(BrokerBrowser *tran,
                       BrokerBrowserClient *client_,
                       BrokerBrowserClient::failure_type failure_): 
               Event(event_id,tran),
               client(client_),
               failure(failure_)
            { }

         public:
            static void create_and_post(BrokerBrowser *tran,
                                        BrokerBrowserClient *client,
                                        BrokerBrowserClient::failure_type failure)
            {
               try
               {
                  ev_failure *ev = new ev_failure(tran,client,failure);
                  ev->post();
               }
               catch(Csi::Event::BadPost &)
               { assert(false); }
            }
         };


         uint4 const ev_failure::event_id =
         Csi::Event::registerType("Cora::LgrNet::BrokerBrowserHelpers::ev_failure");

         
         ////////////////////////////////////////////////////////////
         // class ev_all_started declaration and definitions
         ////////////////////////////////////////////////////////////

         class ev_all_started: public Csi::Event
         {
         public:
            static const uint4 event_id;
            BrokerBrowserClient *client;
            
         private:
            ev_all_started(BrokerBrowser *tran, BrokerBrowserClient *client_):
               Event(event_id,tran),
               client(client_)
            { }

         public:
            static void create_and_post(BrokerBrowser *tran, BrokerBrowserClient *client)
            {
               try
               {
                  ev_all_started *ev = new ev_all_started(tran,client);
                  ev->post();
               }
               catch(Csi::Event::BadPost &)
               { assert(false); }
            }
         };


         uint4 const ev_all_started::event_id =
         Csi::Event::registerType("Cora::LgrNet::BrokerBrowserHelpers::ev_all_started");

         
         ////////////////////////////////////////////////////////////
         // class ev_broker_added declaration and definitions
         ////////////////////////////////////////////////////////////

         class ev_broker_added: public Csi::Event
         {
         public:
            static const uint4 event_id;
            BrokerBrowserClient *client;
            Csi::SharedPtr<TableBrowserEx> broker;
            
         private:
            ev_broker_added(BrokerBrowser *tran,
                            BrokerBrowserClient *client_,
                            Csi::SharedPtr<TableBrowserEx> &broker_):
               Event(event_id,tran),
               client(client_),
               broker(broker_)
            { }

         public:
            static void create_and_post(BrokerBrowser *tran,
                                        BrokerBrowserClient *client,
                                        Csi::SharedPtr<TableBrowserEx> &broker)
            {
               try
               {
                  ev_broker_added *ev = new ev_broker_added(tran,client,broker);
                  ev->post();
               }
               catch(Csi::Event::BadPost &)
               { assert(false); }
            }
         };


         uint4 const ev_broker_added::event_id =
         Csi::Event::registerType("Cora::LgrNet::BrokerBrowserHelpers::ev_broker_added");

         
         ////////////////////////////////////////////////////////////
         // class ev_broker_deleted declaration and definitions
         ////////////////////////////////////////////////////////////

         class ev_broker_deleted: public Csi::Event
         {
         public:
            static const uint4 event_id;
            BrokerBrowserClient *client;
            Csi::SharedPtr<TableBrowserEx> broker;

         private:
            ev_broker_deleted(BrokerBrowser *tran,
                              BrokerBrowserClient *client_,
                              Csi::SharedPtr<TableBrowserEx> &broker_):
               Event(event_id,tran),
               client(client_),
               broker(broker_)
            { }

         public:
            static void create_and_post(BrokerBrowser *tran,
                                        BrokerBrowserClient *client,
                                        Csi::SharedPtr<TableBrowserEx> &broker)
            {
               try
               {
                  ev_broker_deleted *ev = new ev_broker_deleted(tran,client,broker);
                  ev->post();
               }
               catch(Csi::Event::BadPost &)
               { assert(false); }
            }
         };


         uint4 const ev_broker_deleted::event_id =
         Csi::Event::registerType("Cora::LgrNet::BrokerBrowserHelpers::ev_broker_deleted");

         
         ////////////////////////////////////////////////////////////
         // class ev_table_added declaration and definitions
         ////////////////////////////////////////////////////////////

         class ev_table_added: public Csi::Event
         {
         public:
            static const uint4 event_id;
            BrokerBrowserClient *client;
            Csi::SharedPtr<TableBrowserEx> broker;
            Csi::SharedPtr<Cora::Broker::TableDesc> table;
            
         private:
            ev_table_added(BrokerBrowser *tran,
                           BrokerBrowserClient *client_,
                           Csi::SharedPtr<TableBrowserEx> &broker_,
                           Csi::SharedPtr<Cora::Broker::TableDesc> &table_):
               Event(event_id,tran),
               client(client_),
               broker(broker_),
               table(table_)
            { }

         public:
            static void create_and_post(BrokerBrowser *tran,
                                        BrokerBrowserClient *client,
                                        Csi::SharedPtr<TableBrowserEx> &broker,
                                        Csi::SharedPtr<Cora::Broker::TableDesc> &table)
            {
               try
               {
                  ev_table_added *ev = new ev_table_added(tran,client,broker,table);
                  ev->post();
               }
               catch(Csi::Event::BadPost &)
               { assert(false); }
            }
         };


         uint4 const ev_table_added::event_id =
         Csi::Event::registerType("Cora::LgrNet::BrokerBrowserHelpers::ev_table_added");

         
         ////////////////////////////////////////////////////////////
         // class ev_table_deleted declaration and definitions
         ////////////////////////////////////////////////////////////

         class ev_table_deleted: public Csi::Event
         {
         public:
            static const uint4 event_id;
            BrokerBrowserClient *client;
            Csi::SharedPtr<TableBrowserEx> broker;
            Csi::SharedPtr<Cora::Broker::TableDesc> table;
            
         private:
            ev_table_deleted(BrokerBrowser *tran,
                             BrokerBrowserClient *client_,
                             Csi::SharedPtr<TableBrowserEx> &broker_,
                             Csi::SharedPtr<Cora::Broker::TableDesc> &table_):
               Event(event_id,tran),
               client(client_),
               broker(broker_),
               table(table_)
            { }

         public:
            static void create_and_post(BrokerBrowser *tran,
                                        BrokerBrowserClient *client,
                                        Csi::SharedPtr<TableBrowserEx> &broker,
                                        Csi::SharedPtr<Cora::Broker::TableDesc> &table)
            {
               try
               {
                  ev_table_deleted *ev = new ev_table_deleted(tran,client,broker,table);
                  ev->post();
               }
               catch(Csi::Event::BadPost &)
               { assert(false); }
            }
         };


         uint4 const ev_table_deleted::event_id =
         Csi::Event::registerType("Cora::LgrNet::BrokerBrowserHelpers::ev_table_deleted");
      };


      ////////////////////////////////////////////////////////////
      // class TableBrowserEx definitions
      ////////////////////////////////////////////////////////////
      TableBrowserEx::TableBrowserEx(
         uint4 identifier_,
         StrUni const &name_,
         uint4 type_):
         identifier(identifier_),
         name(name_),
         type(type_)
      { set_open_broker_id(identifier,name); }


      ////////////////////////////////////////////////////////////
      // class BrokerBrowser definitions
      ////////////////////////////////////////////////////////////

      BrokerBrowser::BrokerBrowser():
         state(state_standby),
         first_set_wait_count(0),
         client(0),
         broker_mask(broker_mask_active|broker_mask_statistics) ,
         send_temporaries(false)
      { }


      BrokerBrowser::~BrokerBrowser()
      { finish(); }

      
      void BrokerBrowser::add_broker_type(broker_mask_type mask)
      { set_broker_mask(broker_mask | mask); }

      
      void BrokerBrowser::remove_broker_type(broker_mask_type mask)
      { set_broker_mask(broker_mask & (~mask)); }

      
      void BrokerBrowser::set_broker_mask(byte broker_mask_)
      {
         if(state == state_standby)
            broker_mask = broker_mask_;
         else
            throw exc_invalid_state();
      } // set_broker_mask

      
      void BrokerBrowser::set_send_temporaries(bool send_temporaries_)
      {
         if(state == state_standby)
            send_temporaries = send_temporaries_;
         else
            throw exc_invalid_state();
      } // set_send_temporaries

      
      void BrokerBrowser::start(
         BrokerBrowserClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(BrokerBrowserClient::is_valid_instance(client_))
            {
               state = state_attaching;
               client = client_;
               brokers.clear();
               ClientBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void BrokerBrowser::start(
         BrokerBrowserClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(BrokerBrowserClient::is_valid_instance(client_))
            {
               state = state_attaching;
               client = client_;
               brokers.clear();
               ClientBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void BrokerBrowser::finish()
      {
         state = state_standby;
         brokers.clear();
         ClientBase::finish();
      } // finish


      bool BrokerBrowser::find_broker_by_id(broker_handle &dest, uint4 broker_id)
      {
         bool rtn = false;

         dest.bind(0);
         if(state == state_first_set || state == state_steady)
         {
            for(brokers_type::iterator bi = brokers.begin();
                !rtn && bi != brokers.end();
                ++bi)
            {
               if((*bi)->get_identifier() == broker_id)
               {
                  dest = *bi;
                  rtn = true;
               }
            }
         }
         return rtn;
      } // find_broker_by_id


      bool BrokerBrowser::find_active_broker_by_name(
         broker_handle &broker,
         StrUni const &broker_name)
      {
         bool rtn = false;
         for(brokers_type::iterator bi = brokers.begin();
             !rtn && bi != brokers.end();
             ++bi)
         {
            if((*bi)->get_name() == broker_name)
            {
               broker = *bi;
               rtn = true;
            }
         }
         if(!rtn)
            broker.clear();
         return rtn;
      } // find_active_broker_by_name

      
      void BrokerBrowser::on_all_started(Cora::Broker::TableBrowser *broker_)
      {
         broker_handle broker;
         if(find_broker(broker_,broker))
         {
            // inform the client that a broker has been added
            BrokerBrowserHelpers::ev_broker_added::create_and_post(
               this,client,broker); 
            if(state == state_first_set)
            {
               if(--first_set_wait_count == 0)
               {
                  state = state_steady;
                  BrokerBrowserHelpers::ev_all_started::create_and_post(this,client);
               }
            }
         }
         else
            assert(false);
      } // on_all_started

      
      void BrokerBrowser::on_table_added(Cora::Broker::TableBrowser *broker_,
                                         Csi::SharedPtr<Cora::Broker::TableDesc> &table)
      {
         broker_handle broker;
         if(find_broker(broker_,broker))
            BrokerBrowserHelpers::ev_table_added::create_and_post(this,client,broker,table);
         else
            assert(false);
      } // on_table_added

      
      void BrokerBrowser::on_table_deleted(Cora::Broker::TableBrowser *broker_,
                                           Csi::SharedPtr<Cora::Broker::TableDesc> &table)
      {
         broker_handle broker;
         if(find_broker(broker_,broker))
            BrokerBrowserHelpers::ev_table_deleted::create_and_post(this,client,broker,table);
         else
            assert(false);
      } // on_table_deleted

      
      void BrokerBrowser::on_failure(
         Cora::Broker::TableBrowser *broker,
         Cora::Broker::TableBrowserClient::failure_type failure)
      {
         // we will ignore connection failed messages from the table browser because the broker has
         // probably been deleted. This event should get reported though the enumerate brokers
         // transaction.
         if(failure != Cora::Broker::TableBrowserClient::failure_connection_failed)
         {
            BrokerBrowserHelpers::ev_failure::create_and_post(
               this,client,BrokerBrowserClient::failure_table_browser);
         }
      } // on_failure

      
      void BrokerBrowser::onNetMessage(
         Csi::Messaging::Router*rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_first_set || state == state_steady)
         {
            switch(msg->getMsgType())
            {
            case LgrNet_DataBrokersEnumNot:
               on_data_brokers_enum_not(msg);
               break;
               
            default:
               ClientBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void BrokerBrowser::on_corabase_ready()
      {
         // start the network map enumeration
         Csi::Messaging::Message cmd(net_session,Cora::LgrNet_DataBrokersEnumCmd);
         cmd.addUInt4(++last_tran_no);
         router->sendMessage(&cmd);
         state = state_first_set;
         first_set_wait_count = 0;
      } // on_corabase_ready

      
      void BrokerBrowser::on_corabase_failure(corabase_failure_type failure)
      {
         // map the failure into one understood by the client
         BrokerBrowserClient::failure_type client_failure = BrokerBrowserClient::failure_unknown;
         switch(failure)
         {
         case corabase_failure_logon:
            client_failure = BrokerBrowserClient::failure_invalid_logon;
            break;
            
         case corabase_failure_session:
            client_failure = BrokerBrowserClient::failure_connection_failed;
            break;
         }
         BrokerBrowserHelpers::ev_failure::create_and_post(this,client,client_failure);
      } // on_corabase_start_failure

      
      void BrokerBrowser::on_corabase_session_failure()
      { 
         BrokerBrowserHelpers::ev_failure::create_and_post(
            this,client,BrokerBrowserClient::failure_connection_failed); 
      } // on_corabase_session_failure

      
      void BrokerBrowser::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace BrokerBrowserHelpers;
         if(ev->getType() == ev_failure::event_id)
         {
            ev_failure *event = static_cast<ev_failure *>(ev.get_rep());

            finish();
            if(BrokerBrowserClient::is_valid_instance(event->client))
               event->client->on_failure(this,event->failure);
         }
         else if(ev->getType() == ev_all_started::event_id)
         {
            ev_all_started *event = static_cast<ev_all_started *>(ev.get_rep());
            if(BrokerBrowserClient::is_valid_instance(event->client))
               event->client->on_all_started(this);
         }
         else if(ev->getType() == ev_broker_added::event_id)
         {
            ev_broker_added *event = static_cast<ev_broker_added *>(ev.get_rep());
            if(BrokerBrowserClient::is_valid_instance(event->client))
               event->client->on_broker_added(this,event->broker);
         }
         else if(ev->getType() == ev_broker_deleted::event_id)
         {
            ev_broker_deleted *event = static_cast<ev_broker_deleted *>(ev.get_rep());
            if(BrokerBrowserClient::is_valid_instance(event->client))
               event->client->on_broker_deleted(this,event->broker);
         }
         else if(ev->getType() == ev_table_added::event_id)
         {
            ev_table_added *event = static_cast<ev_table_added *>(ev.get_rep());
            if(BrokerBrowserClient::is_valid_instance(event->client))
               event->client->on_table_added(this,event->broker,event->table); 
         }
         else if(ev->getType() == ev_table_deleted::event_id)
         {
            ev_table_deleted *event = static_cast<ev_table_deleted *>(ev.get_rep());
            if(BrokerBrowserClient::is_valid_instance(event->client))
               event->client->on_table_deleted(this,event->broker,event->table);
         }
      } // receive


      bool BrokerBrowser::find_broker(Cora::Broker::TableBrowser *key,
                                      broker_handle &broker)
      {
         bool rtn = false;
         broker.bind(0);
         for(brokers_type::iterator bi = brokers.begin();
             rtn == false && bi != brokers.end();
             ++bi)
         {
            if(static_cast<Cora::Broker::TableBrowser *>(bi->get_rep()) == key)
            {
               broker = *bi;
               rtn = true;
            }
         }
         return rtn;
      } // find_broker


      void BrokerBrowser::on_data_brokers_enum_not(Csi::Messaging::Message *msg)
      {
         uint4 tran_no;
         uint4 resp_code;
         uint4 count;

         msg->readUInt4(tran_no);
         msg->readUInt4(resp_code);
         msg->readUInt4(count);
         if(resp_code == 1)
         {
            uint4 op_code;
            uint4 broker_id;
            uint4 broker_type;
            StrUni broker_name;
            
            for(uint4 i = 0; i < count; ++i)
            {
               // read the broker information
               msg->readUInt4(op_code);
               msg->readUInt4(broker_id);
               msg->readUInt4(broker_type);
               msg->readWStr(broker_name);

               // at this point, we need to look at the type and broker mask to see if this broker
               // should be reported/expanded
               bool should_expand = false;
               switch(broker_type)
               {
               case Cora::Broker::Type::active:
                  if(broker_mask&broker_mask_active)
                     should_expand = true;
                  break;
                  
               case Cora::Broker::Type::backup:
                  if(broker_mask&broker_mask_backup)
                     should_expand = true;
                  break;
                  
               case Cora::Broker::Type::client_defined:
                  if(broker_mask&broker_mask_client_defined)
                     should_expand = true;
                  break;
                  
               case Cora::Broker::Type::statistics:
                  if(broker_mask&broker_mask_statistics)
                     should_expand = true;
                  break;
               }

               // handle the event
               switch(op_code)
               {
               case 1:
                  on_broker_added(broker_id,broker_type,broker_name,should_expand);
                  break;
                  
               case 2:
                  on_broker_changed(broker_id,broker_type,broker_name,should_expand);
                  break;

               case 3:
                  on_broker_deleted(broker_id);
                  break;
                  
               default:
                  assert(false);
                  break;
               }
               
            }

            // There might not have been any brokers or they might have all been blocked out. In any
            // case, we need to see if the all started event needs to be sent
            if(state == state_first_set && first_set_wait_count == 0)
            {
               state = state_steady;
               BrokerBrowserHelpers::ev_all_started::create_and_post(this,client);
            }
         }
         else
            BrokerBrowserHelpers::ev_failure::create_and_post(
               this,client,BrokerBrowserClient::failure_connection_failed);
      } // on_data_brokers_enum_not


      void BrokerBrowser::on_broker_added(
         uint4 identifier, 
         uint4 type,
         StrUni const &name,
         bool should_expand)
      {
         if(should_expand)
         {
            // create the new broker object and add it to our list
            broker_handle new_broker(new TableBrowserEx(identifier,name,type));
            brokers.push_back(new_broker);
            
            // increment the wait count
            if(state == state_first_set)
               ++first_set_wait_count;
            
            // start the new broker using our network session as the default (the new broker
            // will clone that session)
            new_broker->set_send_temporaries(send_temporaries);
            new_broker->start(this,this);
         } 
      } // on_broker_added

      
      void BrokerBrowser::on_broker_changed(uint4 identifier,
                                            uint4 type,
                                            StrUni const &name,
                                            bool should_expand)
      {
         on_broker_deleted(identifier);
         on_broker_added(identifier,type,name,should_expand);
      } // on_broker_changed

      
      void BrokerBrowser::on_broker_deleted(uint4 identifier)
      {
         // search for the specified data broker
         using namespace BrokerBrowserHelpers;
         for(brokers_type::iterator bi = brokers.begin(); bi != brokers.end(); ++bi)
         {
            broker_handle broker(*bi);
            if(broker->get_identifier() == identifier)
            {
               brokers.erase(bi);
               for(Cora::Broker::TableBrowser::const_iterator ti = broker->begin();
                   ti != broker->end();
                   ++ti)
               {
                  Csi::SharedPtr<Cora::Broker::TableDesc> table(*ti);
                  ev_table_deleted::create_and_post(this,client,broker,table);
               }
               ev_broker_deleted::create_and_post(this,client,broker);
               break;
            }
         }
      } // on_broker_deleted 
   };
};
