/* Cora.LgrNet.TapiLinesEnumerator.cpp

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 12 June 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.TapiLinesEnumerator.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace TapiLinesEnumeratorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef TapiLinesEnumeratorClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // receiver
            ////////////////////////////////////////////////////////////
            typedef TapiLinesEnumerator receiver_type;
            receiver_type *receiver;

            ////////////////////////////////////////////////////////////
            // notify
            //
            // Should be overloaded by the derived class to invoke the appropriate client method.
            //////////////////////////////////////////////////////////// 
            virtual void notify() = 0;

         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            //////////////////////////////////////////////////////////// 
            event_base(
               uint4 event_id,
               client_type *client_,
               receiver_type *receiver_):
               Event(event_id,receiver_),
               client(client_),
               receiver(receiver_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            //////////////////////////////////////////////////////////// 
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // create_and_post
            //////////////////////////////////////////////////////////// 
            static void create_and_post(
               client_type *client,
               receiver_type *receiver)
            {
               try { (new event_started(client,receiver))->post(); }
               catch(Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_started(receiver); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               client_type *client,
               receiver_type *receiver):
               event_base(event_id,client,receiver)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::LgrNet::TapiLinesEnumerator::event_started");


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            //////////////////////////////////////////////////////////// 
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // create_and_post
            //////////////////////////////////////////////////////////// ty
            static void create_and_post(
               client_type *client,
               receiver_type *receiver,
               client_type::failure_type failure)
            {
               try { (new event_failure(client,receiver,failure))->post(); }
               catch(Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(receiver,failure); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               client_type *client,
               receiver_type *receiver,
               client_type::failure_type failure_):
               event_base(event_id,client,receiver),
               failure(failure_)
            { }

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            client_type::failure_type failure;
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::LgrNet::TapiLinesEnumerator::event_failure");


         ////////////////////////////////////////////////////////////
         // class event_line_added
         ////////////////////////////////////////////////////////////
         class event_line_added: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            //////////////////////////////////////////////////////////// 
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // create_and_post
            //////////////////////////////////////////////////////////// 
            static void create_and_post(
               client_type *client,
               receiver_type *receiver,
               StrAsc const &line_name)
            {
               try { (new event_line_added(client,receiver,line_name))->post(); }
               catch(Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_line_added(receiver,line_name); }

         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_line_added(
               client_type *client,
               receiver_type *receiver,
               StrAsc const &line_name_):
               event_base(event_id,client,receiver),
               line_name(line_name_)
            { }

            ////////////////////////////////////////////////////////////
            // line_name
            ////////////////////////////////////////////////////////////
            StrAsc line_name;
         };


         uint4 const event_line_added::event_id =
         Csi::Event::registerType("Cora::LgrNet::TapiLinesEnumerator::event_line_added");


         ////////////////////////////////////////////////////////////
         // event_line_removed
         ////////////////////////////////////////////////////////////
         class event_line_removed: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            //////////////////////////////////////////////////////////// 
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // create_and_post
            //////////////////////////////////////////////////////////// 
            static void create_and_post(
               client_type *client,
               receiver_type *receiver,
               StrAsc const &line_name)
            {
               try { (new event_line_removed(client,receiver,line_name))->post(); }
               catch(Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_line_added(receiver,line_name); }

         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_line_removed(
               client_type *client,
               receiver_type *receiver,
               StrAsc const &line_name_):
               event_base(event_id,client,receiver),
               line_name(line_name_)
            { }

            ////////////////////////////////////////////////////////////
            // line_name
            ////////////////////////////////////////////////////////////
            StrAsc line_name;
         };


         uint4 const event_line_removed::event_id =
         Csi::Event::registerType("Cora::LgrNet::TapiLinesEnumerator::event_line_removed"); 
      };

      
      ////////////////////////////////////////////////////////////
      // class TapiLinesEnumerator definitions
      //////////////////////////////////////////////////////////// 
      TapiLinesEnumerator::TapiLinesEnumerator():
         state(state_standby),
         client(0)
      { }

      
      TapiLinesEnumerator::~TapiLinesEnumerator()
      { finish(); }


      void TapiLinesEnumerator::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client  = client_;
               state = state_delegate;
               ClientBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void TapiLinesEnumerator::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client  = client_;
               state = state_delegate;
               ClientBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void TapiLinesEnumerator::finish()
      {
         state = state_standby;
         client = 0;
         ClientBase::finish();
      } // finish

      
      void TapiLinesEnumerator::on_corabase_ready()
      {
         Csi::Messaging::Message start_command(
            net_session,
            Messages::enum_tapi_lines_start_cmd);
         
         start_command.addUInt4(++last_tran_no);
         router->sendMessage(&start_command);
         state = state_active; 
      } // on_corabase_ready

      
      void TapiLinesEnumerator::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace TapiLinesEnumeratorHelpers;
         client_type::failure_type client_failure;
         switch(failure)
         {
         case corabase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case corabase_failure_session:
            client_failure = client_type::failure_session_broken;
            break;
            
         case corabase_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case corabase_failure_security:
            client_failure = client_type::failure_server_security;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(client,this,client_failure);
      } // on_corabase_failure

      
      void TapiLinesEnumerator::on_corabase_session_failure()
      {
         using namespace TapiLinesEnumeratorHelpers;
         event_failure::create_and_post(client,this,client_type::failure_session_broken);
      } // on_corabase_session_failure

      
      void TapiLinesEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace TapiLinesEnumeratorHelpers;
         event_base *event = static_cast<event_base *>(ev.get_rep());
         if(event->client == client)
         {
            if(event->getType() == event_failure::event_id)
               finish();
            if(client_type::is_valid_instance(event->client))
               event->notify();
         }
      } // receive

      
      void TapiLinesEnumerator::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::enum_tapi_lines_started_not:
               on_started_notification(msg);
               break;
               
            case Messages::enum_tapi_lines_line_added_not:
               on_line_added_notification(msg);
               break;
               
            case Messages::enum_tapi_lines_line_removed_not:
               on_line_removed_notification(msg);
               break;
               
            case Messages::enum_tapi_lines_stopped_not:
               on_stopped_notification(msg);
               break;

            default:
               ClientBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage


      void TapiLinesEnumerator::on_started_notification(
         Csi::Messaging::Message *message)
      {
         using namespace TapiLinesEnumeratorHelpers;
         event_started::create_and_post(client,this);
      } // on_started_notification

      
      void TapiLinesEnumerator::on_line_added_notification(
         Csi::Messaging::Message *message)
      {
         using namespace TapiLinesEnumeratorHelpers;
         uint4 tran_no;
         StrAsc line_name;
         if(message->readUInt4(tran_no) && message->readStr(line_name))
            event_line_added::create_and_post(client,this,line_name);
      } // on_line_added_notification

      
      void TapiLinesEnumerator::on_line_removed_notification(
         Csi::Messaging::Message *message)
      {
         using namespace TapiLinesEnumeratorHelpers;
         uint4 tran_no;
         StrAsc line_name;
         if(message->readUInt4(tran_no) && message->readStr(line_name))
            event_line_removed::create_and_post(client,this,line_name);
      } // on_line_removed_notification

      
      void TapiLinesEnumerator::on_stopped_notification(
         Csi::Messaging::Message *message)
      {
         using namespace TapiLinesEnumeratorHelpers;
         event_failure::create_and_post(client,this,client_type::failure_server_aborting);
      } // on_stopped_notification 
   };
};
