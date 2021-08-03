/* Cora.LgrNet.DiscLister.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 21 October 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.DiscLister.h"
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
            typedef DiscListerClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // drives
            ////////////////////////////////////////////////////////////
            typedef client_type::drives_type drives_type;
            drives_type drives;
            
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               DiscLister *lister,
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
               DiscLister *lister,
               client_type *client,
               outcome_type outcome)
            { return new event_complete(lister,client,outcome); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               DiscLister *lister_,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,lister_),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::LgrNet::DiscLister::event_complete");
      };


      void DiscLister::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               ClientBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void DiscLister::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               ClientBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void DiscLister::finish()
      {
         client = 0;
         state = state_standby;
         ClientBase::finish();
      } // finish


      void DiscLister::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(client == event->client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(this,event->outcome,event->drives);
            }
         }
      } // receive


      void DiscLister::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(
            net_session,
            Messages::list_discs_cmd);
         cmd.addUInt4(++last_tran_no);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_corabase_ready


      void DiscLister::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::list_discs_ack)
            {
               uint4 tran_no;
               uint4 count;
               event_complete *ev = event_complete::create(
                  this,
                  client,
                  client_type::outcome_success);
               client_type::drive_type drive;
               uint4 drive_type;
                  
               msg->readUInt4(tran_no);
               msg->readUInt4(count);
               for(uint4 i = 0; i < count; ++i)
               {
                  msg->readStr(drive.root_path);
                  msg->readUInt4(drive_type);
                  msg->readInt8(drive.size);
                  msg->readInt8(drive.free_space);
                  drive.type_code = static_cast<client_type::drive_type::type_code_type>(
                     drive_type);
                  ev->drives.push_back(drive);
               }
               try {ev->post();}
               catch(Csi::Event::BadPost &) { }
            }
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage
      

      void DiscLister::on_corabase_failure(corabase_failure_type failure)
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


      void DiscLister::on_corabase_session_failure()
      {
         event_complete::cpost(
            this,
            client,
            client_type::outcome_session_broken);
      } // on_corabase_session_failure 
   };
};

