/* Cora.Broker.TableDataIndexGetter.cpp

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 18 June 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.TableDataIndexGetter.h"
#include "Cora.Broker.Defs.h"


namespace Cora
{
   namespace Broker
   {
      namespace TableDataIndexGetterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class evnet_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef TableDataIndexGetter::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // index_records
            ////////////////////////////////////////////////////////////
            typedef client_type::index_record_type index_record_type;
            typedef client_type::index_records_type index_records_type;
            index_records_type index_records;

            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            typedef TableDataIndexGetter receiver_type;
            static event_complete *create(
               receiver_type *receiver,
               client_type *client,
               outcome_type outcome)
            { return new event_complete(receiver,client,outcome); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               receiver_type *receiver,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,receiver),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Broker::TableDataIndexGetter::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class TableDataIndexGetter definitions
      ////////////////////////////////////////////////////////////
      TableDataIndexGetter::TableDataIndexGetter():
         state(state_standby),
         client(0)
      { }

      
      TableDataIndexGetter::~TableDataIndexGetter()
      { finish(); }
      

      void TableDataIndexGetter::set_table_name(StrUni const &table_name_)
      {
         if(state == state_standby)
            table_name = table_name_;
         else
            throw exc_invalid_state();
      } // set_table_name

      
      void TableDataIndexGetter::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
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


      void TableDataIndexGetter::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
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
      

      void TableDataIndexGetter::finish()
      {
         state = state_standby;
         client = 0;
         BrokerBase::finish();
      } // finish

      
      void TableDataIndexGetter::on_brokerbase_ready()
      {
         Csi::Messaging::Message command(
            broker_session,
            Messages::get_table_data_index_cmd);
         command.addUInt4(++last_tran_no);
         command.addWStr(table_name);
         router->sendMessage(&command);
         state = state_active;
      } // on_brokerbase_ready

      
      void TableDataIndexGetter::on_brokerbase_failure(brokerbase_failure_type failure)
      {
         using namespace TableDataIndexGetterHelpers;
         client_type::outcome_type outcome;
         switch(failure)
         {
         default:
            outcome = client_type::outcome_unknown;
            break;

         case brokerbase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case brokerbase_failure_session:
            outcome = client_type::outcome_connection_failed;
            break;
            
         case brokerbase_failure_invalid_id:
            outcome = client_type::outcome_invalid_station_name;
            break;
            
         case brokerbase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case brokerbase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;
            
         }
         event_complete::create(this,client,outcome)->post();
      } // on_brokerbase_failure

      
      void TableDataIndexGetter::on_brokerbase_session_failure()
      {
         using namespace TableDataIndexGetterHelpers;
         event_complete::create(this,client,client_type::outcome_connection_failed)->post();
      } // on_brokerbase_session_failure

      
      void TableDataIndexGetter::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::get_table_data_index_ack)
            {
               // read and map the the server outcome
               uint4 tran_no;
               uint4 resp_code;
               client_type::outcome_type outcome;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               switch(resp_code)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 2:
                  outcome = client_type::outcome_invalid_table_name;
                  break;

               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }

               // prepare the evnet to be passed to the client
               using namespace TableDataIndexGetterHelpers;
               event_complete *event = event_complete::create(this,client,outcome);

               if(outcome == client_type::outcome_success)
               {
                  uint4 count;
                  msg->readUInt4(count);
                  for(uint4 i = 0; i < count; ++i)
                  {
                     client_type::index_record_type index_record;
                     msg->readUInt4(index_record.file_mark_no);
                     msg->readUInt4(index_record.begin_record_no);
                     msg->readUInt4(index_record.end_record_no);
                     msg->readInt8(index_record.begin_stamp);
                     msg->readInt8(index_record.end_stamp);
                     event->index_records.push_back(index_record);
                  }
               }
               event->post();
            }
            else
               BrokerBase::onNetMessage(rtr,msg);
         }
         else
            BrokerBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void TableDataIndexGetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace TableDataIndexGetterHelpers; 
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(this,event->outcome,event->index_records);
            }
            else
               finish();
         }
      } // receive
   };
};
