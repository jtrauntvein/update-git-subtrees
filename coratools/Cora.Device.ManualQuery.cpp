/* Cora.Device.ManualQuery.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 02 August 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.ManualQuery.h"
#include <assert.h>

namespace Cora
{
   namespace Device
   {
      namespace ManualQueryHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef ManualQueryClient::resp_code_type resp_code_type;

            static void create_and_post(ManualQuery *query,
                                        ManualQueryClient *client,
                                        resp_code_type resp_code,
                                        uint4 file_mark_no = 0);

            void notify();

         private:
            ManualQuery *query;
            ManualQueryClient *client;
            resp_code_type resp_code;
            uint4 file_mark_no;

            event_complete(ManualQuery *query_,
                           ManualQueryClient *client_,
                           resp_code_type resp_code_,
                           uint4 file_mark_no_):
               Event(event_id,query_),
               query(query_),
               client(client_),
               resp_code(resp_code_),
               file_mark_no(file_mark_no_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::ManualQuery::event_complete");


         void event_complete::create_and_post(ManualQuery *query,
                                             ManualQueryClient *client,
                                             resp_code_type resp_code,
                                             uint4 file_mark_no)
         {
            try { (new event_complete(query,client,resp_code,file_mark_no))->post(); }
            catch(Csi::Event::BadPost &) { }
         }


         void event_complete::notify()
         {
            if(ManualQueryClient::is_valid_instance(client))
               client->on_complete(query,resp_code,file_mark_no);
         } // notify 
      };


      ////////////////////////////////////////////////////////////
      // class ManualQuery definitions
      ////////////////////////////////////////////////////////////

      ManualQuery::ManualQuery():
         client(0),
         state(state_standby),
         query_mode(mode_unspecified),
         begin_date(0),
         end_date(0),
         offset(0),
         begin_record_no(0),
         end_record_no(0)
      { }

      
      ManualQuery::~ManualQuery()
      { finish(); }


      void ManualQuery::set_table_name(StrUni const &table_name_)
      {
         if(state == state_standby)
            table_name = table_name_;
         else
            throw exc_invalid_state();
      } // set_table_name

      
      void ManualQuery::set_date_range(int8 const &begin_stamp_, int8 const &end_stamp_)
      {
         if(state == state_standby)
         {
            query_mode = mode_by_time_stamp;
            begin_date = begin_stamp_;
            end_date = end_stamp_;
         }
         else
            throw exc_invalid_state();
      } // set_date_range

      
      void ManualQuery::set_offset(uint4 offset_)
      {
         if(state == state_standby)
         {
            query_mode = mode_most_recent_x;
            offset = offset_;
         }
         else
            throw exc_invalid_state();
      } // set_offset

      
      void ManualQuery::set_record_no_range(uint4 begin_record_no_, uint4 end_record_no_)
      {
         if(state == state_standby)
         {
            query_mode = mode_by_record_no;
            begin_record_no = begin_record_no_;
            end_record_no = end_record_no_;
         }
         else
            throw exc_invalid_state();         
      } // set_record_no_range

      
      void ManualQuery::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(ManualQueryClient::is_valid_instance(client_))
            {
               if(query_mode != mode_unspecified)
               {
                  state = state_delegate;
                  client = client_;
                  DeviceBase::start(router); 
               }
               else
                  throw std::invalid_argument("Invalid query mode");
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void ManualQuery::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(ManualQueryClient::is_valid_instance(client_))
            {
               if(query_mode != mode_unspecified)
               {
                  state = state_delegate;
                  client = client_;
                  DeviceBase::start(other_component); 
               }
               else
                  throw std::invalid_argument("Invalid query mode");
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void ManualQuery::finish()
      {
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish

      
      void ManualQuery::on_devicebase_ready()
      {
         Csi::Messaging::Message command(device_session,Messages::manual_query_cmd);
         command.addUInt4(++last_tran_no);
         command.addUInt4(query_mode);
         command.addWStr(table_name);
         switch(query_mode)
         {
         case mode_by_time_stamp:
            command.addInt8(begin_date);
            command.addInt8(end_date);
            break;
            
         case mode_most_recent_x:
            command.addUInt4(offset);
            break;
            
         case mode_by_record_no:
            command.addUInt4(begin_record_no);
            command.addUInt4(end_record_no);
            break;
            
         default:
            assert(false);
            break;
         }
         router->sendMessage(&command);
         state = state_active;
      } // on_devicebase_ready

      
      void ManualQuery::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace ManualQueryHelpers;
         
         ManualQueryClient::resp_code_type resp_code;
         switch(failure)
         {
         case devicebase_failure_logon:
            resp_code = ManualQueryClient::resp_invalid_logon;
            break;
            
         case devicebase_failure_session:
            resp_code = ManualQueryClient::resp_session_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            resp_code = ManualQueryClient::resp_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            resp_code = ManualQueryClient::resp_unsupported;
            break;

         case devicebase_failure_security:
            resp_code = ManualQueryClient::resp_server_security_blocked;
            break;
            
         default:
            resp_code = ManualQueryClient::resp_unknown;
            break; 
         }
         event_complete::create_and_post(this,client,resp_code);
      } // on_devicebase_failure

      
      void ManualQuery::on_devicebase_session_failure()
      {
         using namespace ManualQueryHelpers;
         event_complete::create_and_post(this,client,ManualQueryClient::resp_session_failed);
      } // on_devicebase_session_failure

      
      void ManualQuery::onNetMessage(Csi::Messaging::Router *rtr,
                                     Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::manual_query_ack)
            {
               // read the acknowledgement
               uint4 tran_no;
               uint4 status;
               uint4 file_mark_no;

               msg->readUInt4(tran_no);
               msg->readUInt4(status);
               if(status == 1)
                  msg->readUInt4(file_mark_no);

               // process the acknowledgement
               ManualQueryClient::resp_code_type resp_code;
               using namespace ManualQueryHelpers;
               
               switch(status)
               {
               default: resp_code = ManualQueryClient::resp_unknown; break;
               case 1: resp_code = ManualQueryClient::resp_success; break;
               case 2: resp_code = ManualQueryClient::resp_invalid_table_name; break;
               case 3: resp_code = ManualQueryClient::resp_another_in_progress; break; 
               case 4: resp_code = ManualQueryClient::resp_logger_communication_failure; break; 
               case 5: resp_code = ManualQueryClient::resp_logger_security_blocked; break;
               case 7: resp_code = ManualQueryClient::resp_logger_communications_disabled; break; 
               case 8: resp_code = ManualQueryClient::resp_table_enabled; break;
               }
               event_complete::create_and_post(this,client,resp_code,file_mark_no);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void ManualQuery::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace ManualQueryHelpers;
         event_complete *event = dynamic_cast<event_complete *>(ev.get_rep());
         assert(event != 0);
         finish();
         event->notify();
      } // receive
   };
};
