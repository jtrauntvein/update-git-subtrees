/* Cora.LgrNet.LogsZipper.cpp

   Copyright (C) 2008, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 07 April 2008
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.LogsZipper.h"
#include "Cora.LgrNet.Defs.h"
#include "coratools.strings.h"


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
            typedef LogsZipper::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type const outcome;

            ////////////////////////////////////////////////////////////
            // file_name
            ////////////////////////////////////////////////////////////
            StrAsc const file_name;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               LogsZipper *zipper,
               client_type *client,
               outcome_type outcome,
               StrAsc const &file_name = "")
            {
               event_complete *event = new event_complete(zipper, client, outcome, file_name);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               LogsZipper *zipper,
               client_type *client_,
               outcome_type outcome_,
               StrAsc const &file_name_):
               Event(event_id, zipper),
               client(client_),
               outcome(outcome_),
               file_name(file_name_)
            { }
         };
         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::LgrNet::LogsZipper::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class LogsZipper definitions
      ////////////////////////////////////////////////////////////
      void LogsZipper::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *my_client = client;
            
            finish();
            if(client_type::is_valid_instance(event->client) && my_client == event->client)
               my_client->on_complete(this, event->outcome, event->file_name); 
         }
      } // receive


      void LogsZipper::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace LogsZipperStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case client_type::outcome_failure_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_failure_session:
            describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_failure_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_failure_create:
            out << my_strings[strid_outcome_create_failed];
            break;
            
         default:
            describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // describe_outcome


      void LogsZipper::on_corabase_ready()
      {
         Csi::Messaging::Message command(net_session, Messages::zip_logs_cmd);
         command.addUInt4(++last_tran_no);
         if(create_file_name.length())
            command.addStr(create_file_name);
         state = state_active;
         router->sendMessage(&command);
      } // on_corabase_ready


      void LogsZipper::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_failure_logon;
            break;

         case corabase_failure_session:
            outcome = client_type::outcome_failure_session;
            break;

         case corabase_failure_unsupported:
            outcome = client_type::outcome_failure_unsupported;
            break;

         case corabase_failure_security:
            outcome = client_type::outcome_failure_security;
            break;
         }
         event_complete::cpost(this, client, outcome);
      } // on_corabase_failure


      void LogsZipper::on_corabase_session_failure()
      { on_corabase_failure(corabase_failure_session); }


      void LogsZipper::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::zip_logs_ack)
            {
               uint4 tran_no;
               uint4 response;
               StrAsc file_name;
               client_type::outcome_type outcome = client_type::outcome_unknown;

               message->readUInt4(tran_no);
               message->readUInt4(response);
               switch(response)
               {
               case 1:
                  message->readStr(file_name);
                  outcome = client_type::outcome_success;
                  break;
                  
               case 3:
                  outcome = client_type::outcome_failure_create;
                  break;
               }
               event_complete::cpost(this, client, outcome, file_name);
            }
            else
               ClientBase::onNetMessage(rtr, message);
         }
         else
            ClientBase::onNetMessage(rtr, message);
      } // onNetMessage
   };
};

