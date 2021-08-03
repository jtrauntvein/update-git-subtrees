/* Cora.LgrNet.FileRetriever.cpp

   Copyright (C) 2009, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 07 January 2009
   Last Change: Friday 16 November 2012
   Last Commit: $Date: 2013-04-04 15:29:50 -0600 (Thu, 04 Apr 2013) $
   Last Changed by: $Author: jon $

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.FileRetriever.h"
#include "Csi.Utils.h"
#include "Cora.LgrNet.Defs.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace LgrNet
   {
      using namespace FileRetrieverStrings;


      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client_type
            ////////////////////////////////////////////////////////////
            typedef FileRetriever::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(FileRetriever *retriever, client_type *client, outcome_type outcome)
            {
               event_complete *event = new event_complete(retriever, client, outcome);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               FileRetriever *retriever, client_type *client_, outcome_type outcome_):
               Event(event_id, retriever),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::LgrNet::FileRetriever::event_complete");
      };
      
      
      ////////////////////////////////////////////////////////////
      // class FileRetriever definitions
      ////////////////////////////////////////////////////////////
      void FileRetriever::start(client_type *client_, router_handle router)
      {
         if(remote_file_name.length() == 0)
            throw std::invalid_argument(my_strings[strid_invalid_server_file_name].c_str());
         if(local_file_name.length() == 0)
            throw std::invalid_argument(my_strings[strid_invalid_local_file_name].c_str());
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("invalid client pointer");
         client = client_;
         state = state_delegate;
         ClientBase::start(router);
      } // start

      
      void FileRetriever::start(client_type *client_, ClientBase *other_component)
      {
         if(remote_file_name.length() == 0)
            throw std::invalid_argument(my_strings[strid_invalid_server_file_name].c_str());
         if(local_file_name.length() == 0)
            throw std::invalid_argument(my_strings[strid_invalid_local_file_name].c_str());
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("invalid client pointer");
         client = client_;
         state = state_delegate;
         ClientBase::start(other_component);
      } // start

      
      void FileRetriever::finish()
      {
         state = state_standby;
         client = 0;
         if(output != 0)
         {
            fclose(output);
            output = 0;
         }
         ClientBase::finish();
      } // finish


      void FileRetriever::format_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case client_type::outcome_invalid_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_server_security_blocked:
            describe_failure(out, corabase_failure_security);
            break;
            
         case client_type::outcome_session_failed:
            describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_invalid_file_name:
            out << my_strings[strid_invalid_file_name];
            break;
            
         case client_type::outcome_server_file_open_failed:
            out << my_strings[strid_server_file_open_failed];
            break;
            
         case client_type::outcome_local_file_open_failed:
            out << my_strings[strid_local_file_open_failed];
            break;

         default:
            describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // format_outcome

      
      void FileRetriever::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = this->client;
            finish();
            if(client == event->client && client_type::is_valid_instance(client))
               client->on_complete(this, event->outcome);
         }
      } // receive

      
      void FileRetriever::on_corabase_ready()
      {
         Csi::Messaging::Message command(net_session, Messages::retrieve_file_cmd);
         command.addUInt4(++last_tran_no);
         command.addStr(remote_file_name);
         state = state_active;
         router->sendMessage(&command);
      } // on_corabase_ready


      void FileRetriever::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type outcome = client_type::outcome_unknown;
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case corabase_failure_session:
            outcome = client_type::outcome_session_failed;
            break;
            
         case corabase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case corabase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;
         }
         event_complete::cpost(this, client, outcome);
      } // on_corabase_failure

      
      void FileRetriever::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            uint4 const message_type = message->getMsgType();
            if(message_type == Messages::retrieve_file_ack)
            {
               uint4 tran_no;
               uint4 response_code;
               message->readUInt4(tran_no);
               message->readUInt4(response_code);
               if(response_code == 1)
               {
                  Csi::Messaging::Message command(net_session, Messages::retrieve_file_cont_cmd);
                  command.addUInt4(last_tran_no);
                  message->readInt8(file_size);
                  message->readInt8(file_modified_date);
                  router->sendMessage(&command);
               }
               else
               {
                  client_type::outcome_type outcome = client_type::outcome_unknown;
                  switch(response_code)
                  {
                  case 3:
                     outcome = client_type::outcome_invalid_file_name;
                     break;
                     
                  case 4:
                     outcome = client_type::outcome_server_file_open_failed;
                     break;
                  }
                  event_complete::cpost(this, client, outcome);
               }
            }
            else if(message_type == Messages::retrieve_file_frag_ack)
            {
               uint4 tran_no;
               uint4 response_code;
               message->readUInt4(tran_no);
               message->readUInt4(response_code);
               if(response_code == 1)
               {
                  bool last_fragment;
                  message->readBool(last_fragment);
                  message->readBStr(rx_buff);
                  if(output == 0)
                     output = Csi::open_file(local_file_name.c_str(), "wb");
                  if(output != 0)
                  {
                     if(!last_fragment)
                     {
                        Csi::Messaging::Message command(
                           net_session, Messages::retrieve_file_cont_cmd);
                        command.addUInt4(last_tran_no);
                        router->sendMessage(&command);
                     }
                     fwrite(rx_buff.getContents(), rx_buff.length(), 1, output);
                     if(last_fragment)
                     {
                        fclose(output);
                        output = 0;
                        Csi::set_file_time(local_file_name.c_str(), file_modified_date);
                        event_complete::cpost(this, client, client_type::outcome_success);
                     }
                  }
                  else
                     event_complete::cpost(this, client, client_type::outcome_local_file_open_failed);
               }
               else
                  event_complete::cpost(this, client, client_type::outcome_unknown);
            }
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage 
   };
};

