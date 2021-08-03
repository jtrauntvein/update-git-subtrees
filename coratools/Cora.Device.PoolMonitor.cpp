/* Cora.Device.PoolMonitor.cpp

   Copyright (C) 2011, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 01 February 2011
   Last Change: Wednesday 02 February 2011
   Last Commit: $Date: 2011-02-03 13:45:54 -0600 (Thu, 03 Feb 2011) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.PoolMonitor.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace PoolMonitorHelpers
      {
         enum param_id_type
         {
            param_error_rate = 1,
            param_skipped_count = 2,
            param_available = 3,
            param_child_name = 4
         };

         
         ////////////////////////////////////////////////////////////
         // class Resource definitions
         ////////////////////////////////////////////////////////////
         void Resource::read(Csi::Messaging::Message *in)
         {
            uint4 param_count(0);
            uint4 param_size(0);
            uint4 param_id;
            
            in->readUInt4(param_count);
            for(uint4 i = 0; i < param_count; ++i)
            {
               in->readUInt4(param_id);
               in->readUInt4(param_size);
               switch(param_id)
               {
               case param_error_rate:
                  in->readFloat(error_rate);
                  break;
                  
               case param_skipped_count:
                  in->readUInt4(skipped_count);
                  break;
                  
               case param_available:
                  in->readBool(available);
                  break;
                  
               default:
                  in->movePast(param_size);
                  break;
               } 
            }
         } // read


         ////////////////////////////////////////////////////////////
         // class Decision definitions
         ////////////////////////////////////////////////////////////
         void Decision::read(Csi::Messaging::Message *in)
         {
            uint4 msg_event;
            int8 msg_time;
            uint4 param_count;
            uint4 param_size;
            uint4 param_id;
            float msg_error_rate;
            
            if(in->readUInt4(msg_event) &&
               in->readInt8(msg_time) &&
               in->readStr(resource_name) &&
               in->readUInt4(param_count))
            {
               event = static_cast<event_type>(msg_event);
               time = msg_time;
               resource_name = resource_name;
               for(uint4 i = 0; in->whatsLeft() > 8 && i < param_count; ++i)
               {
                  in->readUInt4(param_id);
                  in->readUInt4(param_size);
                  switch(param_id)
                  {
                  case param_error_rate:
                     in->readFloat(msg_error_rate);
                     error_rate = msg_error_rate;
                     break;
                     
                  case param_skipped_count:
                     in->readUInt4(skipped_count);
                     break;
                     
                  case param_available:
                     in->readBool(available);
                     break;
                     
                  case param_child_name:
                     in->readWStr(child_name);
                     break;
                     
                  default:
                     in->movePast(param_size);
                     break;
                  }
               }
            } 
         } // read
      };
      

      namespace
      {
         ////////////////////////////////////////////////////////////
         // class EventBase
         ////////////////////////////////////////////////////////////
         class EventBase: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef PoolMonitor::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // monitor
            ////////////////////////////////////////////////////////////
            PoolMonitor *monitor;

            ////////////////////////////////////////////////////////////
            // deliver
            ////////////////////////////////////////////////////////////
            virtual void deliver() = 0;

         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            EventBase(PoolMonitor *monitor_, client_type *client_):
               Event(event_id, monitor_),
               client(client_),
               monitor(monitor_)
            { }
         };


         uint4 const EventBase::event_id(
            Csi::Event::registerType("Cora::Device::PoolMonitor::EventBase"));


         ////////////////////////////////////////////////////////////
         // class EventStarted
         ////////////////////////////////////////////////////////////
         class EventStarted: public EventBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               PoolMonitor *monitor, client_type *client)
            {
               EventStarted *event(new EventStarted(monitor, client));
               event->post();
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
            EventStarted(PoolMonitor *monitor, client_type *client):
               EventBase(monitor, client)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class EventFailure
         ////////////////////////////////////////////////////////////
         class EventFailure: public EventBase
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
               PoolMonitor *monitor, client_type *client, failure_type failure)
            {
               EventFailure *event(new EventFailure(monitor, client, failure));
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
            EventFailure(
               PoolMonitor *monitor, client_type *client, failure_type failure_):
               EventBase(monitor, client),
               failure(failure_)
            { }
         };

         
         ////////////////////////////////////////////////////////////
         // class EventResourceBase
         ////////////////////////////////////////////////////////////
         class EventResourceBase: public EventBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // resource
            ////////////////////////////////////////////////////////////
            typedef PoolMonitor::resource_handle resource_handle;
            resource_handle resource;

         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            EventResourceBase(
               PoolMonitor *monitor, client_type *client, resource_handle &resource_):
               EventBase(monitor, client),
               resource(resource_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class EventResourceAdded
         ////////////////////////////////////////////////////////////
         class EventResourceAdded: public EventResourceBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // deliver
            ////////////////////////////////////////////////////////////
            virtual void deliver()
            { client->on_resource_added(monitor, resource); }

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               PoolMonitor *monitor, client_type *client, resource_handle &resource)
            {
               EventResourceAdded *event(new EventResourceAdded(monitor, client, resource));
               event->post();
            }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            EventResourceAdded(
               PoolMonitor *monitor, client_type *client, resource_handle &resource):
               EventResourceBase(monitor, client, resource)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class EventResourceRemoved
         ////////////////////////////////////////////////////////////
         class EventResourceRemoved: public EventResourceBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // deliver
            ////////////////////////////////////////////////////////////
            virtual void deliver()
            { client->on_resource_removed(monitor, resource); }

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               PoolMonitor *monitor, client_type *client, resource_handle &resource)
            {
               EventResourceRemoved *event(new EventResourceRemoved(monitor, client, resource));
               event->post();
            }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            EventResourceRemoved(
               PoolMonitor *monitor, client_type *client, resource_handle &resource):
               EventResourceBase(monitor, client, resource)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class EventResourceChanged
         ////////////////////////////////////////////////////////////
         class EventResourceChanged: public EventResourceBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // deliver
            ////////////////////////////////////////////////////////////
            virtual void deliver()
            { client->on_resource_changed(monitor, resource); }

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               PoolMonitor *monitor, client_type *client, resource_handle &resource)
            {
               EventResourceChanged *event(new EventResourceChanged(monitor, client, resource));
               event->post();
            }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            EventResourceChanged(
               PoolMonitor *monitor, client_type *client, resource_handle &resource):
               EventResourceBase(monitor, client, resource)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class EventDecision
         ////////////////////////////////////////////////////////////
         class EventDecision: public EventBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // decision
            ////////////////////////////////////////////////////////////
            typedef PoolMonitorHelpers::Decision decision_type;
            decision_type decision;

            ////////////////////////////////////////////////////////////
            // deliver
            ////////////////////////////////////////////////////////////
            virtual void deliver()
            { client->on_decision(monitor, decision); }

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               PoolMonitor *monitor, client_type *client, decision_type const &decision)
            {
               EventDecision *event(new EventDecision(monitor, client, decision));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            EventDecision(
               PoolMonitor *monitor, client_type *client, decision_type const &decision_):
               EventBase(monitor, client),
               decision(decision_)
            { }
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class PoolMonitor definitions
      ////////////////////////////////////////////////////////////
      void PoolMonitor::describe_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::failure_session:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::failure_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::failure_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         case client_type::failure_security:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::failure_device_shut_down:
            out << common_strings[common_device_shut_down];
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // describe_failure


      void PoolMonitor::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == EventBase::event_id)
         {
            EventBase *event(static_cast<EventBase *>(ev.get_rep()));
            if(client_type::is_valid_instance(event->client) && client == event->client)
               event->deliver();
            else
               finish();
         }
      } // receive


      void PoolMonitor::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state != state_delegate && state != state_inactive)
         {
            uint4 message_type(message->getMsgType());
            uint4 tran_no;
            message->readUInt4(tran_no);
            if(tran_no == transaction_no)
            {
               if(message_type == Messages::monitor_pool_start_ack && state == state_starting)
               {
                  state = state_started;
                  EventStarted::cpost(this, client);
               }
               else if(message_type == Messages::monitor_pool_status_not &&
                       (state == state_starting || state == state_started))
               {
                  StrAsc resource_name;
                  uint4 event;
                  
                  message->readStr(resource_name);
                  message->readUInt4(event);
                  if(event == 1)
                  {
                     resource_handle resource(new resource_type(resource_name));
                     resource->read(message);
                     resources[resource_name] = resource;
                     EventResourceAdded::cpost(this, client, resource);
                  }
                  else if(event == 2)
                  {
                     resources_type::iterator ri(resources.find(resource_name));
                     if(ri != resources.end())
                     {
                        resource_handle resource(ri->second);
                        resources.erase(ri);
                        EventResourceRemoved::cpost(this, client, resource);
                     }
                  }
                  else if(event == 3)
                  {

                     resources_type::iterator ri(resources.find(resource_name));
                     resource_handle resource;

                     if(ri == resources.end())
                     {
                        resource.bind(new resource_type(resource_name));
                        resources[resource_name] = resource;
                     }
                     else
                        resource = ri->second;
                     resource->read(message);
                     EventResourceChanged::cpost(this, client, resource);
                  }
               }
               else if(message_type == Messages::monitor_pool_decision_not &&
                       (state == state_starting || state == state_started))
               {
                  PoolMonitorHelpers::Decision decision;
                  decision.read(message);
                  EventDecision::cpost(this, client, decision);
               }
            }
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage


      void PoolMonitor::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(device_session, Messages::monitor_pool_start_cmd);
         transaction_no = ++last_tran_no;
         cmd.addUInt4(transaction_no);
         cmd.addUInt4(decision_past_interval);
         state = state_starting;
         router->sendMessage(&cmd);
      } // on_devicebase_ready


      void PoolMonitor::on_devicebase_failure(devicebase_failure_type failure_)
      {
         client_type::failure_type failure(client_type::failure_unknown);
         switch(failure_)
         {
         case devicebase_failure_logon:
            failure = client_type::failure_logon;
            break;
            
         case devicebase_failure_session:
            failure = client_type::failure_session;
            break;
            
         case devicebase_failure_invalid_device_name:
            failure = client_type::failure_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            failure = client_type::failure_unsupported;
            break;
            
         case devicebase_failure_security:
            failure = client_type::failure_security;
            break; 
         }
         EventFailure::cpost(this, client, failure);
      } // on_devicebase_failure
   };
};


