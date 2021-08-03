/* Cora.LgrNet.PooledResourcesMonitor.cpp

   Copyright (C) 2011, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 04 February 2011
   Last Change: Friday 04 February 2011
   Last Commit: $Date: 2011-02-25 14:20:55 -0600 (Fri, 25 Feb 2011) $
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.PooledResourcesMonitor.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef PooledResourcesMonitor::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // monitor
            ////////////////////////////////////////////////////////////
            typedef PooledResourcesMonitor monitor_type;
            monitor_type *monitor;

            ////////////////////////////////////////////////////////////
            // deliver
            ////////////////////////////////////////////////////////////
            virtual void deliver() = 0;

         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(monitor_type *monitor_, client_type *client_):
               Event(event_id, monitor_),
               client(client_),
               monitor(monitor_)
            { }
         };


         uint4 const event_base::event_id(
            Csi::Event::registerType("Cora::LgrNet::PooledResourcesMonitor::event_base"));


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef client_type::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               monitor_type *monitor, client_type *client, failure_type failure)
            {
               event_failure *event(new event_failure(monitor, client, failure));
               event->post();
            }

            ////////////////////////////////////////////////////////////
            // deliver
            ////////////////////////////////////////////////////////////
            virtual void deliver()
            {
               monitor->finish();
               client->on_failure(monitor, failure); 
            }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               monitor_type *monitor, client_type *client, failure_type failure_):
               event_base(monitor, client),
               failure(failure_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               monitor_type *monitor, client_type *client)
            {
               event_started *ev(new event_started(monitor, client));
               ev->post();
            }

            ////////////////////////////////////////////////////////////
            // deliver
            ////////////////////////////////////////////////////////////
            virtual void deliver()
            { client->on_started(monitor); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               monitor_type *monitor, client_type *client):
               event_base(monitor, client)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class event_resource
         ////////////////////////////////////////////////////////////
         enum resource_event_type
         {
            resource_added = 1,
            resource_changed = 2,
            resource_removed = 3
         };
         class event_resource: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event
            ////////////////////////////////////////////////////////////
            uint4 event;

            ////////////////////////////////////////////////////////////
            // resource
            ////////////////////////////////////////////////////////////
            typedef client_type::resource_handle resource_handle;
            resource_handle resource;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               monitor_type *monitor,
               client_type *client,
               uint4 event,
               resource_handle &resource)
            {
               event_resource *ev(
                  new event_resource(
                     monitor, client, event, resource));
               ev->post();
            }

            ////////////////////////////////////////////////////////////
            // deliver
            ////////////////////////////////////////////////////////////
            virtual void deliver()
            {
               switch(event)
               {
               case resource_added:
                  client->on_resource_added(monitor, resource);
                  break;
                  
               case resource_changed:
                  client->on_resource_changed(monitor, resource);
                  break;
                  
               case resource_removed:
                  client->on_resource_removed(monitor, resource);
                  break;
               } 
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_resource(
               monitor_type *monitor,
               client_type *client,
               uint4 event_,
               resource_handle &resource_):
               event_base(monitor, client),
               event(event_),
               resource(resource_)
            { }
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class PooledResourcesMonitor definitions
      ////////////////////////////////////////////////////////////
      void PooledResourcesMonitor::describe_failure(
         std::ostream &out,  client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_unknown:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
            
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
         }
      } // describe_failure

      
      void PooledResourcesMonitor::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_base::event_id)
         {
            event_base *event(static_cast<event_base *>(ev.get_rep()));
            if(event->client == client && client_type::is_valid_instance(client))
               event->deliver();
            else
               finish();
         }
      } // receive

      
      void PooledResourcesMonitor::on_corabase_ready()
      {
         Csi::Messaging::Message command(
            net_session, Messages::monitor_pooled_resources_start_cmd);
         transaction_no = ++last_tran_no;
         command.addUInt4(transaction_no);
         state = state_active;
         router->sendMessage(&command);
      } // on_corabase_ready

      
      void PooledResourcesMonitor::on_corabase_failure(corabase_failure_type failure_)
      {
         client_type::failure_type failure;
         switch(failure_)
         {
         case corabase_failure_unknown:
            failure = client_type::failure_unknown;
            break;
            
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
         event_failure::cpost(this, client, failure);
      } // on_corabase_failure

      
      void PooledResourcesMonitor::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            uint4 message_type(message->getMsgType());
            if(message_type == Messages::monitor_pooled_resources_start_ack)
            {
               uint4 tran_no;
               message->readUInt4(tran_no);
               if(tran_no == transaction_no)
                  event_started::cpost(this, client); 
            }
            else if(message_type == Messages::monitor_pooled_resources_not)
            {
               uint4 tran_no;
               StrAsc resource_name;
               uint4 not_event;

               if(message->readUInt4(tran_no) &&
                  message->readStr(resource_name) &&
                  message->readUInt4(not_event))
               {
                  if(tran_no == transaction_no)
                  {
                     resource_handle resource;
                     if(not_event == resource_added)
                     {
                        resource.bind(new resource_type(resource_name));
                        resource->read(*message);
                        resources[resource_name] = resource; 
                     }
                     else if(not_event == resource_changed)
                     {
                        resources_type::iterator ri(resources.find(resource_name));
                        if(ri != resources.end())
                        {
                           resource = ri->second;
                           resource->read(*message);
                        }
                     }
                     else if(not_event == resource_removed)
                     {
                        resources_type::iterator ri(resources.find(resource_name));
                        if(ri != resources.end())
                        {
                           resource = ri->second;
                           resources.erase(ri);
                        }
                     }
                     if(resource != 0)
                        event_resource::cpost(this, client, not_event, resource);
                  }
               }
            }
            else if(message_type == Messages::monitor_pooled_resources_stopped_not)
            {
            }
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage


      namespace PooledResourcesMonitorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class Resource definitions
         ////////////////////////////////////////////////////////////
         void Resource::read(Csi::Messaging::Message &in)
         {
            uint4 param_count;
            uint4 param_id;
            uint4 param_size;

            if(in.readUInt4(param_count))
            {
               while(in.whatsLeft() >= 8)
               {
                  in.readUInt4(param_id);
                  in.readUInt4(param_size);
                  switch(param_id)
                  {
                  case 1:
                     in.readFloat(error_rate);
                     break;
                     
                  case 2:
                     in.readBool(available);
                     break;
                     
                  case 3:
                     in.readFloat(use_percent);
                     break;

                  case 4:
                     in.readWStr(target_device);
                     break;

                  default:
                     in.movePast(param_size);
                     break;
                  }
               }
            }
         } // read
      };
   };
};

