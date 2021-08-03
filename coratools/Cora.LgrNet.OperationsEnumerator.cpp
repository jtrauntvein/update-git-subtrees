/* Cora.LgrNet.OperationsEnumerator.cpp

   Copyright (C) 2010, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 29 September 2010
   Last Change: Friday 05 August 2011
   Last Commit: $Date: 2011-08-05 14:48:48 -0600 (Fri, 05 Aug 2011) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.OperationsEnumerator.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace OperationsEnumeratorHelpers
      {
         ////////////////////////////////////////////////////////////
         // enum attr_id_type
         ////////////////////////////////////////////////////////////
         enum attr_id_type
         {
            attr_description = 1,
            attr_start_time = 2,
            attr_device_name = 3,
            attr_priority = 4,
            attr_last_transmit_time = 5,
            attr_last_receive_time = 6,
            attr_timeout_interval = 7,
            attr_state = 8,
            attr_app_name = 10,
            attr_account_name = 11
         };

         
         ////////////////////////////////////////////////////////////
         // class Report definitions
         ////////////////////////////////////////////////////////////
         void Report::read(Csi::Messaging::Message &message)
         {
            // initialise the changed members so the client can tell what got changed on a new
            // changed notification.
            priority_changed = false;
            last_transmit_time_changed = false;
            last_receive_time_changed = false;
            timeout_interval_changed = false;
            state_changed = false;
            app_name_changed = false;
            account_name_changed = false;
            
            // we will now process the message content
            uint4 attr_count;
            if(message.readUInt4(attr_count))
            {
               for(uint4 i = 0; i < attr_count && message.whatsLeft() >= 8; ++i)
               {
                  uint4 attr_id;
                  uint4 attr_len;
                  int8 nsec;
                  message.readUInt4(attr_id);
                  message.readUInt4(attr_len);
                  if(attr_len <= message.whatsLeft())
                  {
                     switch(attr_id)
                     {
                     case attr_description:
                        message.readStr(description);
                        break;
                        
                     case attr_start_time:
                        message.readInt8(nsec);
                        start_time = nsec;
                        break;
                        
                     case attr_device_name:
                        message.readWStr(device_name);
                        break;
                        
                     case attr_priority:
                        message.readUInt4(priority);
                        priority_changed = true;
                        break;
                        
                     case attr_last_transmit_time:
                        message.readInt8(nsec);
                        last_transmit_time = nsec;
                        last_transmit_time_changed = true;
                        break;
                        
                     case attr_last_receive_time:
                        message.readInt8(nsec);
                        last_receive_time = nsec;
                        last_receive_time_changed = true;
                        break;
                        
                     case attr_timeout_interval:
                        message.readUInt4(timeout_interval);
                        timeout_interval_changed = true;
                        break;
                        
                     case attr_state:
                        message.readStr(state);
                        state_changed = true;
                        break;

                     case attr_app_name:
                        message.readWStr(app_name);
                        app_name_changed = true;
                        break;
                        
                     case attr_account_name:
                        message.readWStr(account_name);
                        account_name_changed = true;
                        break;
                        
                     default:
                        message.movePast(attr_len);
                        break;
                     }
                  }
                  else
                     break;
               }
            }
         } // read


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
            // client
            ////////////////////////////////////////////////////////////
            typedef OperationsEnumerator lister_type;
            typedef lister_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef client_type::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(lister_type *lister, client_type *client, failure_type failure)
            {
               event_failure *event(new event_failure(lister, client, failure));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(lister_type *lister, client_type *client_, failure_type failure_):
               Event(event_id, lister),
               client(client_),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id(
            Csi::Event::registerType("Cora::LgrNet::OperationsEnumerator::event_failure"));


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
            // client
            ////////////////////////////////////////////////////////////
            typedef OperationsEnumerator lister_type;
            typedef lister_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(lister_type *lister, client_type *client)
            {
               event_started *event(new event_started(lister, client));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(lister_type *lister, client_type *client_):
               Event(event_id, lister),
               client(client_)
            { }
         };


         uint4 const event_started::event_id(
            Csi::Event::registerType("Cora::LgrNet::OperationsEnumerator::event_started"));


         ////////////////////////////////////////////////////////////
         // class event_added
         ////////////////////////////////////////////////////////////
         class event_added: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef OperationsEnumerator lister_type;
            typedef lister_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // report
            ////////////////////////////////////////////////////////////
            typedef client_type::report_handle report_handle;
            report_handle report;
            
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(lister_type *lister, client_type *client, report_handle &report)
            {
               event_added *event(new event_added(lister, client, report));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_added(lister_type *lister, client_type *client_, report_handle &report_):
               Event(event_id, lister),
               client(client_),
               report(report_)
            { }
         };


         uint4 const event_added::event_id(
            Csi::Event::registerType("Cora::LgrNet::OperationsEnumerator::event_added"));


         ////////////////////////////////////////////////////////////
         // class event_changed
         ////////////////////////////////////////////////////////////
         class event_changed: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef OperationsEnumerator lister_type;
            typedef lister_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // report
            ////////////////////////////////////////////////////////////
            typedef client_type::report_handle report_handle;
            report_handle report;
            
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(lister_type *lister, client_type *client, report_handle &report)
            {
               event_changed *event(new event_changed(lister, client, report));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_changed(lister_type *lister, client_type *client_, report_handle &report_):
               Event(event_id, lister),
               client(client_),
               report(report_)
            { }
         };


         uint4 const event_changed::event_id(
            Csi::Event::registerType("Cora::LgrNet::OperationsEnumerator::event_changed"));


         ////////////////////////////////////////////////////////////
         // class event_deleted
         ////////////////////////////////////////////////////////////
         class event_deleted: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef OperationsEnumerator lister_type;
            typedef lister_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // report
            ////////////////////////////////////////////////////////////
            typedef client_type::report_handle report_handle;
            report_handle report;
            
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(lister_type *lister, client_type *client, report_handle &report)
            {
               event_deleted *event(new event_deleted(lister, client, report));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_deleted(lister_type *lister, client_type *client_, report_handle &report_):
               Event(event_id, lister),
               client(client_),
               report(report_)
            { }
         };


         uint4 const event_deleted::event_id(
            Csi::Event::registerType("Cora::LgrNet::OperationsEnumerator::event_deleted"));
      };


      ////////////////////////////////////////////////////////////
      // class OperationsEnumerator definitions
      ////////////////////////////////////////////////////////////
      void OperationsEnumerator::describe_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_session_failed:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::failure_invalid_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::failure_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::failure_server_security_blocked:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;

         default:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // describe_failure


      void OperationsEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace OperationsEnumeratorHelpers;
         if(ev->getType() == event_failure::event_id)
         {
            event_failure *event = static_cast<event_failure *>(ev.get_rep());
            client = 0;
            state = state_ready;
            if(client_type::is_valid_instance(event->client))
               event->client->on_failure(this, event->failure);
         }
         else if(ev->getType() == event_started::event_id)
         {
            event_started *event = static_cast<event_started *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
               client->on_started(this);
            else
               finish();
         }
         else if(ev->getType() == event_added::event_id)
         {
            event_added *event = static_cast<event_added *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
               client->on_operation_added(this, event->report);
            else
               finish();
         }
         else if(ev->getType() == event_changed::event_id)
         {
            event_changed *event = static_cast<event_changed *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
               client->on_operation_changed(this, event->report);
            else
               finish();
         }
         else if(ev->getType() == event_deleted::event_id)
         {
            event_deleted *event = static_cast<event_deleted *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
               client->on_operation_deleted(this, event->report);
            else
               finish();
         }
      } // receive

      
      void OperationsEnumerator::on_corabase_failure(
         corabase_failure_type failure_)
      {
         using namespace OperationsEnumeratorHelpers;
         client_type::failure_type failure(client_type::failure_unknown);
         switch(failure_)
         {
         case corabase_failure_logon:
            failure = client_type::failure_invalid_logon;
            break;
            
         case corabase_failure_session:
            failure = client_type::failure_session_failed;
            break;
            
         case corabase_failure_unsupported:
            failure = client_type::failure_unsupported;
            break;
            
         case corabase_failure_security:
            failure = client_type::failure_server_security_blocked;
            break;
         }
         event_failure::cpost(this, client, failure);
      } // on_corabase_failure

      
      void OperationsEnumerator::on_corabase_ready()
      {
         message_type start_cmd(net_session, Messages::operation_enum_start_cmd);
         start_cmd.addUInt4(++last_tran_no);
         state = state_waiting_for_start;
         router->sendMessage(&start_cmd);
      } // on_corabase_ready

      
      void OperationsEnumerator::onNetMessage(
         router_type *router, message_type *message)
      {
         if(state >= state_waiting_for_start)
         {
            switch(message->getMsgType())
            {
            case Messages::operation_enum_start_ack:
               on_start_ack(message);
               break;
               
            case Messages::operation_enum_op_added_not:
               on_added_not(message);
               break;
               
            case Messages::operation_enum_op_changed_not:
               on_changed_not(message);
               break;
               
            case Messages::operation_enum_op_deleted_not:
               on_deleted_not(message);
               break;

            default:
               ClientBase::onNetMessage(router, message);
            }
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage


      void OperationsEnumerator::on_start_ack(message_type *message)
      {
         using namespace OperationsEnumeratorHelpers;
         if(state == state_waiting_for_start)
         {
            uint4 tran_no;
            uint4 outcome;
            message->readUInt4(tran_no);
            message->readUInt4(outcome);
            if(outcome == 1)
            {
               state = state_started;
               event_started::cpost(this, client);
            }
            else
               event_failure::cpost(this, client, client_type::failure_unknown);
         }
      } // on_start_ack


      void OperationsEnumerator::on_added_not(message_type *message)
      {
         using namespace OperationsEnumeratorHelpers;
         if(state >= state_waiting_for_start)
         {
            uint4 tran_no;
            int8 id;
            report_handle report;
            
            message->readUInt4(tran_no);
            message->readInt8(id);
            report.bind(new report_type(id));
            report->read(*message);
            reports[id] = report;
            if(state == state_started)
               event_added::cpost(this, client, report);
         }
      } // on_added_not


      void OperationsEnumerator::on_changed_not(message_type *message)
      {
         using namespace OperationsEnumeratorHelpers;
         if(state == state_started)
         {
            uint4 tran_no;
            int8 id;
            reports_type::iterator ri;

            message->readUInt4(tran_no);
            message->readInt8(id);
            ri = reports.find(id);
            if(ri != reports.end())
            {
               ri->second->read(*message);
               event_changed::cpost(this, client, ri->second);
            }
         }
      } // on_changed_not


      void OperationsEnumerator::on_deleted_not(message_type *message)
      {
         using namespace OperationsEnumeratorHelpers;
         if(state == state_started)
         {
            uint4 tran_no;
            int8 id;
            reports_type::iterator ri;

            message->readUInt4(tran_no);
            message->readInt8(id);
            ri = reports.find(id);
            if(ri != reports.end())
            {
               report_handle report(ri->second);
               reports.erase(ri);
               event_deleted::cpost(this, client, report);
            }
         }
      } // on_deleted_not
   };
};
   
