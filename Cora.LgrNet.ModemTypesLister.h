/* Cora.LgrNet.ModemTypesLister.h

   Copyright (C) 2009, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 14 September 2009
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_LgrNet_ModemTypesLister_h
#define Cora_LgrNet_ModemTypesLister_h

#include "Cora.ClientBase.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace LgrNet
   {
      ////////////////////////////////////////////////////////////
      // struct ModemType
      ////////////////////////////////////////////////////////////
      struct ModemType
      {
         StrUni type_name;
         StrAsc reset;
         StrAsc init;
         bool custom;
      };

      
      ////////////////////////////////////////////////////////////
      // class ModemTypesListerClient
      ////////////////////////////////////////////////////////////
      class ModemTypesLister;
      class ModemTypesListerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called after the modem type list has been retrieved from the server
         // and the information for all individual modems has also been
         // retrieved.  
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_session_broken = 3,
            outcome_unsupported = 4,
            outcome_server_security_blocked = 5
         }; 
         virtual void on_complete(
            ModemTypesLister *lister,
            outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class ModemTypesLister
      ////////////////////////////////////////////////////////////
      class ModemTypesLister:
         public ClientBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ModemTypesLister();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ModemTypesLister();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef ModemTypesListerClient client_type;
         void start(
            client_type *client_, ClientBase::router_handle &router);
         void start(
            client_type *client, ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // describe_outcome
         ////////////////////////////////////////////////////////////
         static void describe_outcome(
            std::ostream &out, client_type::outcome_type outcome);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         // @group: ModemType records container declarations

         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef std::list<ModemType> modems_type;
         typedef modems_type::iterator iterator;
         typedef modems_type::const_iterator const_iterator;
         iterator begin()
         { return modems.begin(); }
         const_iterator begin() const
         { return modems.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end()
         { return modems.end(); }
         const_iterator end() const
         { return modems.end(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return modems.empty(); }

         // @endgroup

      protected:
         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(
            corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure()
         { on_corabase_failure(corabase_failure_session); }

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);

      private:
         ////////////////////////////////////////////////////////////
         // on_modems_enum_ack
         ////////////////////////////////////////////////////////////
         void on_modems_enum_ack(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_modem_get_ack
         ////////////////////////////////////////////////////////////
         void on_modem_get_ack(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // get_next_modem
         ////////////////////////////////////////////////////////////
         void get_next_modem(bool pop_first);
         
      private:
         ////////////////////////////////////////////////////////////
         // modems
         ////////////////////////////////////////////////////////////
         modems_type modems;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // waiting
         ////////////////////////////////////////////////////////////
         std::list<StrUni> waiting;
      };
   };
};

#endif
