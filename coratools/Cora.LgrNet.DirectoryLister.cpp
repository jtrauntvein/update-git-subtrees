/* Cora.LgrNet.DirectoryLister.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 22 October 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.DirectoryLister.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
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
            // client
            ////////////////////////////////////////////////////////////
            typedef DirectoryLister::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // full_path
            ////////////////////////////////////////////////////////////
            StrAsc full_path;

            ////////////////////////////////////////////////////////////
            // elements
            ////////////////////////////////////////////////////////////
            typedef client_type::elements_type elements_type;
            elements_type elements;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               DirectoryLister *lister,
               client_type *client,
               outcome_type outcome)
            {
               try{(new event_complete(lister,client,outcome))->post();}
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static event_complete *create(
               DirectoryLister *lister,
               client_type *client)
            { return new event_complete(lister,client,client_type::outcome_success); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               DirectoryLister *lister,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,lister),
               client(client_),
               outcome(outcome_)
            { } 
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::LgrNet::DirectoryLister::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class DirectoryLister definitions
      ////////////////////////////////////////////////////////////
      void DirectoryLister::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(client == event->client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(
                  this,
                  event->outcome,
                  event->full_path,
                  event->elements);
            }
         }
      } // receive

      
      void DirectoryLister::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(
            net_session,
            Messages::list_directory_files_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addStr(path);
         cmd.addBool(list_path_parent);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_corabase_ready

      
      void DirectoryLister::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active &&
            msg->getMsgType() == Messages::list_directory_files_ack)
         {
            uint4 tran_no;
            uint4 response;
            msg->readUInt4(tran_no);
            msg->readUInt4(response);
            if(response == 1)
            {
               event_complete *event = event_complete::create(this,client);
               uint4 elements_count;
               
               msg->readStr(event->full_path);
               msg->readUInt4(elements_count);
               for(uint4 i = 0; i < elements_count; ++i)
               {
                  DirectoryElement element;
                  uint4 type_code;
                  uint4 attrib_count;
                  uint4 attrib_type;
                  uint4 attrib_size;
                  int8 nano_sec;
                  
                  msg->readStr(element.name);
                  msg->readUInt4(type_code);
                  element.type_code = static_cast<DirectoryElement::type_code_type>(type_code);
                  msg->readUInt4(attrib_count);
                  for(uint4 j = 0; j < attrib_count; ++j)
                  {
                     msg->readUInt4(attrib_type);
                     msg->readUInt4(attrib_size);
                     switch(attrib_type)
                     {
                     case 1:
                        msg->readBool(element.is_read_only);
                        break;

                     case 2:
                        msg->readBool(element.is_hidden);
                        break;

                     case 3:
                        msg->readBool(element.is_system);
                        break;

                     case 4:
                        msg->readBool(element.is_temporary);
                        break;

                     case 5:
                        msg->readInt8(nano_sec);
                        element.creation_time = nano_sec;
                        break;

                     case 6:
                        msg->readInt8(nano_sec);
                        element.last_access_time = nano_sec;
                        break;

                     case 7:
                        msg->readInt8(nano_sec);
                        element.last_write_time = nano_sec;
                        break;

                     case 8:
                        msg->readInt8(element.file_size);
                        break;

                     default:
                        msg->movePast(attrib_size);
                        break;
                     }
                  }
                  event->elements.push_back(element); 
               }

               // post the completion event
               try {event->post();}
               catch(Csi::Event::BadPost &) { }
            }
            else
            {
               client_type::outcome_type outcome;
               switch(response)
               {
               case 2:
                  outcome = client_type::outcome_invalid_path;
                  break;
                  
               case 3:
                  outcome = client_type::outcome_path_specifies_file;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::cpost(this,client,outcome);
            }
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void DirectoryLister::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;

         case corabase_failure_session:
            outcome = client_type::outcome_session_broken;
            break;

         case corabase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;

         case corabase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;

         default:
            outcome = client_type::outcome_unknown;
            break;   
         }
         event_complete::cpost(this,client,outcome);
      } // on_corabase_failure

      
      void DirectoryLister::on_corabase_session_failure()
      {
         event_complete::cpost(this,client,client_type::outcome_session_broken); 
      } // on_corabase_session_failure
   };
};

