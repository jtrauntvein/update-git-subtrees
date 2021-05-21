/* Cora.LgrNet.ViewMapper.cpp

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 23 August 2012
   Last Change: Tuesday 04 September 2012
   Last Commit: $Date: 2012-09-04 09:01:53 -0600 (Tue, 04 Sep 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.ViewMapper.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef ViewMapperClient::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(ViewMapper *mapper, failure_type failure)
            {
               event_failure *event(new event_failure(mapper, failure));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(ViewMapper *mapper, failure_type failure_):
               Event(event_id, mapper),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id(
            Csi::Event::registerType("Cora::LgrNet::ViewMapper::event_failure"));


         ////////////////////////////////////////////////////////////
         // class event_not
         ////////////////////////////////////////////////////////////
         class event_not: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // network
            ////////////////////////////////////////////////////////////
            typedef ViewMapperClient::map_network_type network_type;
            network_type network;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_not(ViewMapper *mapper):
               Event(event_id, mapper)
            { }
         };


         uint4 const event_not::event_id(
            Csi::Event::registerType("Cora::LgrNet::ViewMapper::event_not"));


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(ViewMapper *mapper)
            {
               event_started *event(new event_started(mapper));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(ViewMapper *mapper):
               Event(event_id, mapper)
            { }
         };


         uint4 const event_started::event_id(
            Csi::Event::registerType("Cora::LgrNet::ViewMapper::event_started"));


         ////////////////////////////////////////////////////////////
         // connect_entries
         //
         // Connects the devices in the specified network in parent/child relationships
         ////////////////////////////////////////////////////////////
         template <class container_type>
         void connect_entries(container_type &network)
         {
            // for each device, we will locate its parent
            for(typename container_type::iterator ei = network.begin();
                ei != network.end();
                ++ei)
            {
               // we will search backward in the network to find the device that has an indent
               // level one greater than this entry
               typename container_type::value_type &entry(*ei);
               if(ei != network.begin())
               {
                  uint4 this_level(entry->level);
                  typename container_type::iterator pi(ei); --pi;
                  typename container_type::value_type prospect(*pi);
                  while(prospect->level >= this_level)
                  {
                     if(pi != network.begin())
                     {
                        --pi;
                        prospect = *pi;
                     }
                     else
                        break;
                  }
                  if(prospect->level < this_level)
                  {
                     entry->parent = prospect.get_rep();
                     prospect->children.push_back(entry.get_rep());
                  }
               }
            }
         } // connect_entries
      };


      ////////////////////////////////////////////////////////////
      // class ViewMapper definitions
      ////////////////////////////////////////////////////////////
      void ViewMapper::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_failure::event_id)
         {
            event_failure *event(static_cast<event_failure *>(ev.get_rep()));
            client_type *client(this->client);
            finish();
            if(client_type::is_valid_instance(client))
               client->on_failure(this, event->failure);
         }
         else if(ev->getType() == event_not::event_id)
         {
            event_not *event(static_cast<event_not *>(ev.get_rep()));
            if(client_type::is_valid_instance(client))
               client->on_view_changed(this, event->network);
            else
               finish();
         }
         else if(ev->getType() == event_started::event_id)
         {
            if(client_type::is_valid_instance(client))
               client->on_started(this);
            else
               finish();
         }
      } // receive


      void ViewMapper::describe_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;

         case client_type::failure_session:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;

         case client_type::failure_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;

         case client_type::failure_security:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;

         case client_type::failure_invalid_view_id:
            out << "an invalid view ID was specified";
            break;

         case client_type::failure_view_deleted:
            out << "the view was deleted";
            break;
            
         default:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // describe_failure


      void ViewMapper::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(net_session, Messages::enum_view_map_start_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt4(view_id);
         cmd.addUInt4(view_option);
         state = state_starting;
         router->sendMessage(&cmd);
      } // on_corabase_ready


      void ViewMapper::on_corabase_failure(corabase_failure_type failure_)
      {
         client_type::failure_type failure(client_type::failure_unknown);
         switch(failure_)
         {
         case corabase_failure_logon:
            failure = client_type::failure_logon;
            break;
            
         case corabase_failure_session:
            failure = client_type::failure_session;
            break;
            
         case corabase_failure_unsupported:
            failure = client_type::failure_unsupported;
            break;
            
         case corabase_failure_security:
            failure = client_type::failure_security;
            break;
         }
         event_failure::cpost(this, failure);
      } // on_corabase_failure


      void ViewMapper::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_starting || state == state_started)
         {
            if(message->getMsgType() == Messages::enum_view_map_not)
            {
               // parse the message
               uint4 tran_no;
               uint4 count;
               event_not *event(new event_not(this));
               
               message->readUInt4(tran_no);
               message->readUInt4(count);
               for(uint4 i = 0; i < count; ++i)
               {
                  client_type::value_type entry(
                     new client_type::entry_type(*message));
                  event->network.push_back(entry);
               }

               // we need to thread the entries in the network together
               connect_entries(event->network);
               event->post();
               if(state == state_starting)
               {
                  state = state_started;
                  event_started::cpost(this);
               }
            }
            else if(message->getMsgType() == Messages::enum_view_map_stopped_not)
            {
               client_type::failure_type failure(client_type::failure_unknown);
               uint4 tran_no;
               uint4 reason;
               message->readUInt4(tran_no);
               message->readUInt4(reason);
               switch(reason)
               {
               case 2:
                  failure = client_type::failure_invalid_view_id;
                  break;

               case 3:
                  failure = client_type::failure_view_deleted;
                  break;
               }
               event_failure::cpost(this, failure);
            }
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage
   };
};

