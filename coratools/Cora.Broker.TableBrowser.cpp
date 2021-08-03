/* Cora.Broker.TableBrowser.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 30 May 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.TableBrowser.h"
#include <assert.h>


namespace Cora
{
   namespace Broker
   {
      namespace TableBrowserHelpers
      {
         ////////////////////////////////////////////////////////////
         // class ev_all_started declaration and definitions
         ////////////////////////////////////////////////////////////
         class ev_all_started: public Csi::Event
         {
         public:
            static uint4 const event_id;
            TableBrowserClient *client;

         private:
            ev_all_started(TableBrowser *tran, TableBrowserClient *client_):
               Event(event_id,tran),
               client(client_)
            { }

         public:
            static void create_and_post(TableBrowser *tran, TableBrowserClient *client)
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
         Csi::Event::registerType("Cora::Broker::TableBrowserHelpers::ev_all_started");
         

         ////////////////////////////////////////////////////////////
         // class ev_failure_type declaration and definitions
         ////////////////////////////////////////////////////////////

         class ev_failure: public Csi::Event
         {
         public:
            static uint4 const event_id;
            TableBrowserClient *client;
            TableBrowserClient::failure_type failure;

         private:
            ev_failure(TableBrowser *tran,
                       TableBrowserClient *client_,
                       TableBrowserClient::failure_type failure_):
               Event(event_id,tran),
               failure(failure_),
               client(client_)
            { }

         public:
            static void create_and_post(TableBrowser *tran,
                                        TableBrowserClient *client,
                                        TableBrowserClient::failure_type failure)
                                        
            {
               try
               {
                  ev_failure *ev = new ev_failure(tran,client,failure);
                  ev->post();
               }
               catch(Csi::Event::BadPost &)
               { }
            }
         };

         uint4 const ev_failure::event_id =
         Csi::Event::registerType("Cora::Broker::TableBrowserHelpers::ev_failure_type");


         ////////////////////////////////////////////////////////////
         // class ev_table_added declaration and definitions
         ////////////////////////////////////////////////////////////

         class ev_table_added: public Csi::Event
         {
         public:
            TableBrowserClient *client;
            Csi::SharedPtr<TableDesc> table;
            static uint4 const event_id;

         private:
            ev_table_added(TableBrowser *tran,
                           TableBrowserClient *client_,
                           Csi::SharedPtr<TableDesc> &table_):
               Event(event_id,tran),
               table(table_),
               client(client_)
            { }

         public:
            static void create_and_post(TableBrowser *tran,
                                        TableBrowserClient *client,
                                        Csi::SharedPtr<TableDesc> &table)
            {
               try
               {
                  ev_table_added *ev = new ev_table_added(tran,client,table);
                  ev->post();
               }
               catch(Csi::Event::BadPost &)
               { assert(false); }
            }
         };

         uint4 const ev_table_added::event_id =
         Csi::Event::registerType("Cora::Broker::TableBrowserHelpers::ev_table_added");


         ////////////////////////////////////////////////////////////
         // class ev_table_deleted declaration and definitions
         ////////////////////////////////////////////////////////////

         class ev_table_deleted: public Csi::Event
         {
         public:
            static uint4 const event_id;
            Csi::SharedPtr<TableDesc> table;
            TableBrowserClient *client;

         private:
            ev_table_deleted(TableBrowser *tran,
                             TableBrowserClient *client_,
                             Csi::SharedPtr<TableDesc> &table_):
               Event(event_id,tran),
               table(table_),
               client(client_)
            { }

         public:
            static void create_and_post(TableBrowser *tran,
                                        TableBrowserClient *client,
                                        Csi::SharedPtr<TableDesc> &table)
            {
               try
               {
                  ev_table_deleted *ev = new ev_table_deleted(tran,client,table);
                  ev->post();
               }
               catch(Csi::Event::BadPost &)
               { assert(false); }
            }
         };


         uint4 const ev_table_deleted::event_id =
         Csi::Event::registerType("Cora::Broker::TableBrowserHelpers::ev_table_deleted");

      };


      ////////////////////////////////////////////////////////////
      // class TableBrowser definitions
      ////////////////////////////////////////////////////////////
      
      TableBrowser::TableBrowser():
         state(state_standby),
         client(0),
         send_temporaries(false)
      { }

      
      TableBrowser::~TableBrowser()
      { finish(); }


      void TableBrowser::set_send_temporaries(bool send_temporaries_)
      {
         if(state == state_standby)
            send_temporaries = send_temporaries_;
         else
            throw exc_invalid_state();
      } // set_send_temporaries

      
      void TableBrowser::start(
         TableBrowserClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(TableBrowserClient::is_valid_instance(client_))
            {
               client = client_;
               state = state_attach;
               BrokerBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client instance");
         }
         else
            throw exc_invalid_state();
      } // start


      void TableBrowser::start(
         TableBrowserClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(TableBrowserClient::is_valid_instance(client_))
            {
               client = client_;
               state = state_attach;
               BrokerBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client instance");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void TableBrowser::finish()
      {
         client = 0;
         state = state_standby;
         BrokerBase::finish();
      } // finish

      
      void TableBrowser::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_attach)
            BrokerBase::onNetMessage(rtr,msg);
         else
         {
            switch(msg->getMsgType())
            {
            case Messages::table_defs_enum_not:
               on_table_defs_enum_not(msg);
               break;
               
            case Messages::table_def_get_ack:
            case Messages::extended_table_def_get_ack:
               on_table_def_get_ack(msg);
               break;

            default:
               BrokerBase::onNetMessage(rtr,msg);
               break;
            }
         }
      } // onNetMessage

      
      void TableBrowser::on_brokerbase_ready()
      {
         // start the table definitions enumerate transaction
         Csi::Messaging::Message cmd(broker_session,Messages::table_defs_enum_cmd);
         cmd.addUInt4(++last_tran_no);
         if(interface_version >= Csi::VersionNumber("1.3.1.27"))
            cmd.addBool(send_temporaries);
         router->sendMessage(&cmd);
         state = state_first_set; 
      } // on_brokerbase_ready

      
      void TableBrowser::on_brokerbase_failure(brokerbase_failure_type failure)
      {
         // map the failure code into one understood by the client
         TableBrowserClient::failure_type client_failure = TableBrowserClient::failure_unknown;
         switch(failure)
         {
         case brokerbase_failure_logon:
            client_failure = TableBrowserClient::failure_invalid_logon;
            break;
            
         case brokerbase_failure_session:
            client_failure = TableBrowserClient::failure_connection_failed;
            break;
            
         case brokerbase_failure_invalid_id:
            client_failure = TableBrowserClient::failure_invalid_broker_id;
            break;
         }
         TableBrowserHelpers::ev_failure::create_and_post(this,client,client_failure); 
      } // on_brokerbase_failure

      
      void TableBrowser::on_brokerbase_session_failure()
      {
         TableBrowserHelpers::ev_failure::create_and_post(
            this,client,TableBrowserClient::failure_connection_failed);
      } // on_brokerbase_session_failure


      void TableBrowser::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace TableBrowserHelpers;
         if(ev->getType() == ev_all_started::event_id)
         {
            ev_all_started *event = static_cast<ev_all_started *>(ev.get_rep());
            if(TableBrowserClient::is_valid_instance(event->client))
               event->client->on_all_started(this);
         }
         else if(ev->getType() == ev_failure::event_id)
         {
            ev_failure *event = static_cast<ev_failure *>(ev.get_rep());
            finish();
            if(TableBrowserClient::is_valid_instance(event->client))
               event->client->on_failure(this,event->failure);
         }
         else if(ev->getType() == ev_table_added::event_id)
         {
            ev_table_added *event = static_cast<ev_table_added *>(ev.get_rep());
            if(TableBrowserClient::is_valid_instance(event->client))
               event->client->on_table_added(this,event->table);
         }
         else if(ev->getType() == ev_table_deleted::event_id)
         {
            ev_table_deleted *event = static_cast<ev_table_deleted *>(ev.get_rep());
            if(TableBrowserClient::is_valid_instance(event->client))
               event->client->on_table_deleted(this,event->table);
         }
      } // receive

      
      void TableBrowser::on_table_defs_enum_not(Csi::Messaging::Message *msg)
      {
         // read the notification header
         uint4 tran_no;
         uint4 resp_code;
         uint4 count;

         msg->readUInt4(tran_no);
         msg->readUInt4(resp_code);
         msg->readUInt4(count);

         if(resp_code == 1)
         {
            StrUni table_name;
            uint4 change_code;
            
            for(uint4 i = 0; i < count; ++i)
            {
               // read the table information
               msg->readWStr(table_name);
               msg->readUInt4(change_code);
               process_table_change(table_name,change_code);
            }

            // this broker might not define any tables. If this is the case and we haven't reached a
            // steady state, we will need to report completion
            if(state == state_first_set && waiting_trans.empty())
            {
               state = state_steady;
               TableBrowserHelpers::ev_all_started::create_and_post(this,client);
            }
         }
         else
            TableBrowserHelpers::ev_failure::create_and_post(
               this,client,TableBrowserClient::failure_unknown);
      } // on_table_defs_enum_not

      
      void TableBrowser::on_table_def_get_ack(Csi::Messaging::Message *msg)
      {
         uint4 tran_no;
         uint4 resp_code;
         msg->readUInt4(tran_no);
         msg->readUInt4(resp_code);

         if(resp_code == 1)
         {
            // search for the description associated with the transaction number
            waiting_trans_type::iterator wti = waiting_trans.find(tran_no);
            if(wti != waiting_trans.end())
            {
               Csi::SharedPtr<TableDesc> new_desc(wti->second);
               waiting_trans.erase(wti);
               
               if(new_desc->read(msg))
               {
                  // add this table to the list managed and send a message to inform the client
                  tables.push_back(new_desc);
                  TableBrowserHelpers::ev_table_added::create_and_post(this,client,new_desc);
                  
                  // If the wait count drops to zero and we are in a first_set state, we need to make a
                  // transition to steady state.
                  if(state == state_first_set && waiting_trans.empty())
                  {
                     state = state_steady;
                     TableBrowserHelpers::ev_all_started::create_and_post(this,client);
                  }
               }
               else
                  assert(false);
            }
            else
               assert(false);
         }
         else
            TableBrowserHelpers::ev_failure::create_and_post(
               this,client,TableBrowserClient::failure_unknown);
      } // on_table_def_get_ack


      void TableBrowser::process_table_change(StrUni const &table_name, uint4 change_code)
      {
         if(change_code == 1)
         {
            // create a new table description object that will hold the description that will be
            // read.
            Csi::SharedPtr<TableDesc> new_desc(new TableDesc(table_name));
            uint4 tran_no = ++last_tran_no;

            waiting_trans[tran_no] = new_desc;

            // servers newer than 1.2 (1.3.1 and greater) need to use the extended table definitions
            // transaction
            uint4 command_code;
            if(interface_version < Csi::VersionNumber("1.3.1"))
               command_code = Messages::table_def_get_cmd;
            else
               command_code = Messages::extended_table_def_get_cmd;
            
            // the table was added so we need to get its definition
            Csi::Messaging::Message cmd(broker_session,command_code);
            cmd.addUInt4(tran_no);
            cmd.addWStr(table_name);
            router->sendMessage(&cmd);
         }
         else if(change_code == 2)
         {
            // a table was deleted. We need to search for this table and, if found, notify the
            // client and delete the description record
            for(tables_type::iterator ti = tables.begin(); ti != tables.end(); ++ti)
            {
               if((*ti)->get_name() == table_name)
               {
                  TableBrowserHelpers::ev_table_deleted::create_and_post(this,client,*ti);
                  tables.erase(ti);
                  break;
               }
            }
         }
         else
            assert(false);
      } // process_table_change
   };
};
