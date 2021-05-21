/* Cora.LgrNet.CountriesEnumerator.cpp

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 13 June 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.CountriesEnumerator.h"
#include <assert.h>


namespace Cora
{
   namespace LgrNet
   {
      namespace CountriesEnumeratorHelpers
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
            typedef CountriesEnumeratorClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // countries_type
            ////////////////////////////////////////////////////////////
            typedef client_type::countries_type countries_type;
            countries_type countries;

            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static event_complete *create(
               CountriesEnumerator *enumerator,
               client_type *client,
               outcome_type outcome)
            { return new event_complete(enumerator,client,outcome); }
            
            ////////////////////////////////////////////////////////////
            // add_country
            ////////////////////////////////////////////////////////////
            void add_country(uint4 country_code, StrAsc const &country_name)
            {
               countries_type::value_type value;
               value.country_name = country_name;
               value.country_code = country_code;
               countries.push_back(value);
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               CountriesEnumerator *enumerator,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,enumerator),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::LgrNet::CountriesEnumerator::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class CountriesEnumerator definitions
      ////////////////////////////////////////////////////////////
      CountriesEnumerator::CountriesEnumerator():
         client(0),
         state(state_standby)
      { }

      
      CountriesEnumerator::~CountriesEnumerator()
      { finish(); }

      
      void CountriesEnumerator::start(
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


      void CountriesEnumerator::start(
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

      
      void CountriesEnumerator::finish()
      {
         client = 0;
         state = state_standby;
         ClientBase::finish();
      } // finish

      
      void CountriesEnumerator::on_corabase_ready()
      {
         Csi::Messaging::Message command(
            net_session,
            Messages::enum_countries_cmd);

         command.addUInt4(++last_tran_no);
         router->sendMessage(&command);
         state = state_active; 
      } // on_corabase_ready

      
      void CountriesEnumerator::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace CountriesEnumeratorHelpers;
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
            outcome = client_type::outcome_server_security;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete *event = event_complete::create(this,client,outcome);
         event->post();
      } // on_corabase_failure

      
      void CountriesEnumerator::on_corabase_session_failure()
      {
         using namespace CountriesEnumeratorHelpers;
         event_complete *event = event_complete::create(
            this,
            client,
            client_type::outcome_session_broken);
         event->post();
      } // on_corabase_session_failure

      
      void CountriesEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace CountriesEnumeratorHelpers;
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(this,event->outcome,event->countries);
            }
            else
               finish();
         }
      } // receive

      
      void CountriesEnumerator::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::enum_countries_ack)
            {
               uint4 tran_no;
               uint4 server_outcome;
               uint4 num_countries;
               if(msg->readUInt4(tran_no) &&
                  msg->readUInt4(server_outcome) &&
                  msg->readUInt4(num_countries))
               {
                  // interpret the server's outcome code
                  client_type::outcome_type client_outcome;
                  switch(server_outcome)
                  {
                  case 1:
                     client_outcome = client_type::outcome_success;
                     break;

                  default:
                     client_outcome = client_type::outcome_unknown;
                     break;
                  }
                  
                  // create the event to be posted
                  using namespace CountriesEnumeratorHelpers;
                  event_complete *event = event_complete::create(this,client,client_outcome);

                  // read and add the set of countries to the event
                  uint4 country_code;
                  StrAsc country_name;

                  for(uint4 i = 0; i < num_countries; ++i)
                  {
                     if(msg->readUInt4(country_code) && msg->readStr(country_name))
                        event->add_country(country_code,country_name);
                     else
                        assert(false);
                  }
                  event->post();
               }
               else
                  assert(false);
            }
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage 
   };
};
