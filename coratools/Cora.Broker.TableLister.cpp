/* Cora.Broker.TableLister.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 08 August 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2019-10-29 15:49:38 -0600 (Tue, 29 Oct 2019) $ 
   Committed by: $Author: amortenson $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.TableLister.h"
#include <assert.h>


namespace Cora
{
   namespace Broker
   {
      namespace TableListerHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         protected:
            TableLister *lister;
            TableListerClient *client;
            friend class Cora::Broker::TableLister;

         public:
            ////////// constructor
            event_base(uint4 event_id,
                       TableLister *lister_,
                       TableListerClient *client_):
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

            static void create_and_post(TableLister *lister,
                                        TableListerClient *client);

            virtual void notify()
            { client->on_started(lister); }
            
         private:
            event_started(TableLister *lister,
                          TableListerClient *client):
               event_base(event_id,lister,client)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Broker::TableLister::event_started");


         void event_started::create_and_post(TableLister *lister,
                                             TableListerClient *client)
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
            static uint4 const event_id;
            typedef TableListerClient::failure_type failure_type;
            failure_type failure;

            static void create_and_post(TableLister *lister,
                                        TableListerClient *client,
                                        failure_type failure);

            virtual void notify()
            { client->on_failure(lister,failure); }
            
         private:
            event_failure(TableLister *lister,
                          TableListerClient *client,
                          failure_type failure_):
               event_base(event_id,lister,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Broker::TableLister::event_failure");


         void event_failure::create_and_post(TableLister *lister,
                                             TableListerClient *client,
                                             failure_type failure)
         {
            try { (new event_failure(lister,client,failure))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_table_added
         ////////////////////////////////////////////////////////////
         class event_table_added: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni table_name;

            static void create_and_post(TableLister *lister,
                                        TableListerClient *client,
                                        StrUni const &table_name);

            virtual void notify()
            { client->on_table_added(lister,table_name); }
            
         private:
            event_table_added(TableLister *lister,
                          TableListerClient *client,
                          StrUni const &table_name_):
               event_base(event_id,lister,client),
               table_name(table_name_)
            { }
         };


         uint4 const event_table_added::event_id =
         Csi::Event::registerType("Cora::Broker::TableLister::event_table_added");


         void event_table_added::create_and_post(TableLister *lister,
                                                 TableListerClient *client,
                                                 StrUni const &table_name)
         {
            try { (new event_table_added(lister,client,table_name))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post
         

         ////////////////////////////////////////////////////////////
         // class event_table_deleted
         ////////////////////////////////////////////////////////////
         class event_table_deleted: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni table_name;

            static void create_and_post(TableLister *lister,
                                        TableListerClient *client,
                                        StrUni const &table_name);

            virtual void notify()
            { client->on_table_deleted(lister,table_name); }
            
         private:
            event_table_deleted(TableLister *lister,
                          TableListerClient *client,
                          StrUni const &table_name_):
               event_base(event_id,lister,client),
               table_name(table_name_)
            { }
         };


         uint4 const event_table_deleted::event_id =
         Csi::Event::registerType("Cora::Broker::TableLister::event_table_deleted");


         void event_table_deleted::create_and_post(TableLister *lister,
                                                   TableListerClient *client,
                                                   StrUni const &table_name)
         {
            try { (new event_table_deleted(lister,client,table_name))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post 
      };
      

      ////////////////////////////////////////////////////////////
      // class TableLister definitions
      ////////////////////////////////////////////////////////////

      TableLister::TableLister():
         state(state_standby),
         client(0),
         send_temporaries(true)
      { }

      
      TableLister::~TableLister()
      { finish(); }


      void TableLister::set_send_temporaries(bool send_temporaries_)
      {
         if(state == state_standby)
            send_temporaries = send_temporaries_;
         else
            throw exc_invalid_state();
      }


      void TableLister::start(
         TableListerClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(TableListerClient::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               BrokerBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void TableLister::start(
         TableListerClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(TableListerClient::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               BrokerBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void TableLister::finish()
      {
         client = 0;
         state = state_standby;
         BrokerBase::finish();
      } // finish

      void TableLister::format_failure(std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
            case client_type::failure_invalid_broker_id:
               BrokerBase::format_failure(out, brokerbase_failure_invalid_id);
               break;
            case client_type::failure_invalid_logon:
               BrokerBase::format_failure(out, brokerbase_failure_logon);
               break;
            case client_type::failure_session_failed:
               BrokerBase::format_failure(out, brokerbase_failure_session);
               break;
            case client_type::failure_unsupported:
               BrokerBase::format_failure(out, brokerbase_failure_unsupported);
               break;
            case client_type::failure_server_security_blocked:
               BrokerBase::format_failure(out, brokerbase_failure_security);
               break;
            default:
               BrokerBase::format_failure(out, brokerbase_failure_unknown);
               break;
         }
      }

      void TableLister::onNetMessage(Csi::Messaging::Router *rtr,
                                     Csi::Messaging::Message *msg)
      {
         if(state == state_active || state == state_before_active)
         {
            if(msg->getMsgType() == Messages::table_defs_enum_not)
            {
               // read the message header
               uint4 tran_no;
               uint4 resp_code;
               uint4 count;
               using namespace TableListerHelpers;

               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               msg->readUInt4(count);

               if(resp_code == 1)
               {
                  uint4 change_code;
                  StrUni table_name;

                  for(uint4 i = 0; i < count; ++i)
                  {
                     msg->readWStr(table_name);
                     msg->readUInt4(change_code);
                     switch(change_code)
                     {
                     case 1:
                        event_table_added::create_and_post(this,client,table_name);
                        break;
                        
                     case 2:
                        event_table_deleted::create_and_post(this,client,table_name);
                        break;
                     }
                  }
                  if(state == state_before_active)
                  {
                     state = state_active;
                     event_started::create_and_post(this,client);
                  }
               }
               else
                  event_failure::create_and_post(this,
                                                 client,
                                                 TableListerClient::failure_session_failed);
            }
            else
               BrokerBase::onNetMessage(rtr,msg);
         }
         else
            BrokerBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void TableLister::on_brokerbase_ready()
      {
         Csi::Messaging::Message command(broker_session,Messages::table_defs_enum_cmd);

         command.addUInt4(++last_tran_no);
         if(interface_version >= Csi::VersionNumber("1.3.1.27"))
            command.addBool(send_temporaries);
         router->sendMessage(&command);
         state = state_before_active;
      } // on_brokerbase_ready

      
      void TableLister::on_brokerbase_failure(brokerbase_failure_type failure)
      {
         using namespace TableListerHelpers;
         TableListerClient::failure_type client_failure;
         
         switch(failure)
         {
         default:
            client_failure = TableListerClient::failure_unknown;
            break;

         case brokerbase_failure_logon:
            client_failure = TableListerClient::failure_invalid_logon;
            break;
            
         case brokerbase_failure_session:
            client_failure = TableListerClient::failure_session_failed;
            break;
            
         case brokerbase_failure_invalid_id:
            client_failure = TableListerClient::failure_invalid_broker_id;
            break;
            
         case brokerbase_failure_unsupported:
            client_failure = TableListerClient::failure_unsupported;
            break;
            
         case brokerbase_failure_security:
            client_failure = TableListerClient::failure_server_security_blocked;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_brokerbase_failure

      
      void TableLister::on_brokerbase_session_failure()
      {
         using namespace TableListerHelpers;
         event_failure::create_and_post(this,client,TableListerClient::failure_session_failed);
      } // on_brokerbase_session_failure


      void TableLister::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace TableListerHelpers;
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         assert(event != 0);
         if(event->getType() == event_failure::event_id)
            finish();
         if(TableListerClient::is_valid_instance(event->client))
            event->notify();
         else
            finish();
      } // receive
   };
};
