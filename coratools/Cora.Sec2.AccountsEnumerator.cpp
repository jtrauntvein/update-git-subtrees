/* Cora.Sec2.AccountsEnumerator.cpp

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 30 December 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2019-10-29 14:52:52 -0600 (Tue, 29 Oct 2019) $ 
   Last Changed by: $Author: amortenson $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Sec2.AccountsEnumerator.h"


namespace Cora
{
   namespace Sec2
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
            // enumerator
            ////////////////////////////////////////////////////////////
            typedef AccountsEnumerator enumerator_type;
            enumerator_type *enumerator;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef enumerator_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify() = 0;

         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               enumerator_type *enumerator_,
               client_type *client_):
               Event(event_id,enumerator_),
               enumerator(enumerator_),
               client(client_)
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
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_started(enumerator); }

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               enumerator_type *enumerator,
               client_type *client)
            {
               try{(new event_started(enumerator,client))->post();}
               catch(Csi::Event::BadPost &){ }
            }
            
         private:
            event_started(
               enumerator_type *enumerator,
               client_type *client):
               event_base(event_id,enumerator,client)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Sec2::AccountsEnumerator::event_started");


         ////////////////////////////////////////////////////////////
         // class event_account_added
         ////////////////////////////////////////////////////////////
         class event_account_added: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // account_name
            ////////////////////////////////////////////////////////////
            StrUni account_name;

            ////////////////////////////////////////////////////////////
            // account_password
            ////////////////////////////////////////////////////////////
            StrUni account_password;

            ////////////////////////////////////////////////////////////
            // access_level
            ////////////////////////////////////////////////////////////
            typedef AccessLevels::AccessLevelType access_level_type;
            access_level_type access_level;

            ////////////////////////////////////////////////////////////
            // device_additions
            ////////////////////////////////////////////////////////////
            typedef client_type::device_additions_type device_additions_type;
            device_additions_type device_additions;
            
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               client->on_account_added(
                  enumerator,
                  account_name,
                  account_password,
                  access_level,
                  device_additions);
            }

            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static event_account_added *create(
               enumerator_type *enumerator,
               client_type *client)
            { return new event_account_added(enumerator,client); }
            
         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_account_added(
               enumerator_type *enumerator,
               client_type *client,
               uint4 id = event_id):
               event_base(id,enumerator,client)
            { }
         };


         uint4 const event_account_added::event_id =
         Csi::Event::registerType("Cora::Sec2::AccountsEnumerator::event_account_added");


         ////////////////////////////////////////////////////////////
         // class event_account_changed
         ////////////////////////////////////////////////////////////
         class event_account_changed: public event_account_added
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;
            
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               client->on_account_changed(
                  enumerator,
                  account_name,
                  account_password,
                  access_level,
                  device_additions);
            }

            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static event_account_changed *create(
               enumerator_type *enumerator,
               client_type *client)
            { return new event_account_changed(enumerator,client); }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_account_changed(
               enumerator_type *enumerator,
               client_type *client):
               event_account_added(enumerator,client)
            { }
         };


         uint4 const event_account_changed::event_id =
         Csi::Event::registerType("Cora::Sec2::AccountsEnumerator::event_account_changed");


         ////////////////////////////////////////////////////////////
         // class event_account_deleted
         ////////////////////////////////////////////////////////////
         class event_account_deleted: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // account_name
            ////////////////////////////////////////////////////////////
            StrUni account_name;
            
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_account_deleted(enumerator,account_name); }

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               enumerator_type *enumerator,
               client_type *client,
               StrUni const &account_name)
            {
               try{(new event_account_deleted(enumerator,client,account_name))->post();}
               catch(Csi::Event::BadPost &){ }
            }
            
         private:
            event_account_deleted(
               enumerator_type *enumerator,
               client_type *client,
               StrUni const &account_name_):
               event_base(event_id,enumerator,client),
               account_name(account_name_)
            { }
         };


         uint4 const event_account_deleted::event_id =
         Csi::Event::registerType("Cora::Sec2::AccountsEnumerator::event_account_deleted");


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
            // failure
            ////////////////////////////////////////////////////////////
            typedef client_type::failure_type failure_type;
            failure_type failure;
            
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(enumerator,failure); }

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               enumerator_type *enumerator,
               client_type *client,
               failure_type failure)
            {
               try{(new event_failure(enumerator,client,failure))->post();}
               catch(Csi::Event::BadPost &){ }
            }
            
         private:
            event_failure(
               enumerator_type *enumerator,
               client_type *client,
               failure_type failure_):
               event_base(event_id,enumerator,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Sec2::AccountsEnumerator::event_failure");
      };


      ////////////////////////////////////////////////////////////
      // class AccountsEnumerator definitions
      ////////////////////////////////////////////////////////////
      AccountsEnumerator::AccountsEnumerator():
         client(0),
         state(state_standby)
      { }

      
      AccountsEnumerator::~AccountsEnumerator()
      { finish(); }

      
      void AccountsEnumerator::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               Sec2Base::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         } 
         else
            throw exc_invalid_state();
      } // start

      
      void AccountsEnumerator::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               Sec2Base::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         } 
         else
            throw exc_invalid_state();
      } // start

      
      void AccountsEnumerator::finish()
      {
         client = 0;
         state = state_standby;
         Sec2Base::finish();
      } // finish

      void AccountsEnumerator::format_failure(std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
            case client_type::failure_connection_failed:
               describe_failure(out, corabase_failure_session);
               break;
            case client_type::failure_logon:
               describe_failure(out, corabase_failure_logon);
               break;
            case client_type::failure_insufficient_access:
               describe_failure(out, corabase_failure_security);
               break;
            case client_type::failure_unsupported:
               describe_failure(out, corabase_failure_unsupported);
               break;
            default:
               describe_failure(out, corabase_failure_unknown);
               break;
         }
      } // format_failure


      void AccountsEnumerator::on_sec2base_ready()
      {
         Csi::Messaging::Message command(sec2_session,Messages::enum_accounts_start_cmd);
         command.addUInt4(++last_tran_no);
         state = state_before_active;
         router->sendMessage(&command);
      } // on_sec2base_ready

      
      void AccountsEnumerator::on_sec2base_failure(sec2base_failure_type failure)
      {
         client_type::failure_type client_failure;
         switch(failure)
         {
         case sec2base_failure_logon:
            client_failure = client_type::failure_logon;
            break;
            
         case sec2base_failure_session:
            client_failure = client_type::failure_connection_failed;
            break;
            
         case sec2base_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case sec2base_failure_security:
            client_failure = client_type::failure_insufficient_access;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_sec2base_failure

      
      void AccountsEnumerator::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active || state == state_before_active)
         {
            if(msg->getMsgType() == Messages::enum_accounts_start_ack)
            {
               uint4 tran_no;
               uint4 outcome;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(outcome);
               if(outcome == 1)
               {
                  state = state_active;
                  event_started::create_and_post(this,client);
               }
               else
               {
                  client_type::failure_type failure;
                  switch(outcome)
                  {
                  case 3:
                     failure = client_type::failure_insufficient_access;
                     break;
                     
                  default:
                     failure = client_type::failure_unknown;
                     break;
                  }
                  event_failure::create_and_post(this,client,failure);
               }
            }
            if(msg->getMsgType() == Messages::enum_accounts_not)
            {
               uint4 tran_no;
               uint4 action;

               msg->readUInt4(tran_no);
               msg->readUInt4(action);
               if(action == 1 || action == 2)
               {
                  event_account_added *event = event_account_added::create(this,client);
                  uint4 access_level;
                  uint4 num_additions;
                  StrUni addition;
                  
                  msg->readWStr(event->account_name);
                  msg->readWStr(event->account_password);
                  msg->readUInt4(access_level);
                  event->access_level = static_cast<AccessLevels::AccessLevelType>(access_level);
                  msg->readUInt4(num_additions);
                  for(uint4 i = 0; i < num_additions; ++i)
                  {
                     msg->readWStr(addition);
                     event->device_additions.push_back(addition);
                  }
                  event->post();
               }
               else if(action == 3)
               {
                  event_account_added *event = event_account_changed::create(this,client);
                  uint4 access_level;
                  uint4 num_additions;
                  StrUni addition;
                  
                  msg->readWStr(event->account_name);
                  msg->readWStr(event->account_password);
                  msg->readUInt4(access_level);
                  event->access_level = static_cast<AccessLevels::AccessLevelType>(access_level);
                  msg->readUInt4(num_additions);
                  for(uint4 i = 0; i < num_additions; ++i)
                  {
                     msg->readWStr(addition);
                     event->device_additions.push_back(addition);
                  }
                  event->post();
               }
               else if(action == 4)
               {
                  StrUni account_name;
                  msg->readWStr(account_name);
                  event_account_deleted::create_and_post(this,client,account_name);
               }
               else
                  event_failure::create_and_post(this,client,client_type::failure_unknown);
            }
            else if(msg->getMsgType() == Messages::enum_accounts_stopped_not)
            {
               uint4 tran_no;
               uint4 server_failure;
               client_type::failure_type client_failure;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(server_failure);
               switch(server_failure)
               {
               case 2:
                  client_failure = client_type::failure_connection_failed;
                  break;

               case 3:
                  client_failure = client_type::failure_insufficient_access;
                  break;

               default:
                  client_failure = client_type::failure_unknown;
                  break;
               }
               event_failure::create_and_post(this,client,client_failure);
            }
            else
               Sec2Base::onNetMessage(rtr,msg);
         }
         else
            Sec2Base::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void AccountsEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            client_type *client = this->client;
            if(event->getType() == event_failure::event_id)
               finish();
            if(client == event->client && client_type::is_valid_instance(client))
               event->notify();
            else if(client == event->client)
               finish();
         }
      } // receive 
   };
};
