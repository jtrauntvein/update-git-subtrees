/* Cora.LgrNet.ViewRemover.h

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 21 August 2012
   Last Change: Tuesday 21 August 2012
   Last Commit: $Date: 2012-08-22 09:16:13 -0600 (Wed, 22 Aug 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_ViewRemover_h
#define Cora_LgrNet_ViewRemover_h
#include "Cora.ClientBase.h"
#include "Cora.LgrNet.Defs.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace LgrNet
   {
      ////////////////////////////////////////////////////////////
      // class ViewRemoverClient
      ////////////////////////////////////////////////////////////
      class ViewRemover;
      class ViewRemoverClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_failure_logon = 2,
            outcome_failure_session = 3,
            outcome_failure_unsupported = 4,
            outcome_failure_security = 5,
            outcome_failure_invalid_view_id = 6
         };
         virtual void on_complete(
            ViewRemover *remover, outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class ViewRemover
      //
      // This class defines a component that wraps the LgrNet Remove View
      // transaction.  in order to use this component, the application must
      // provide a client object that extends class ViewRemoverClient.  It must
      // then create an instance of class ViewRemover, call set_view_desc() to
      // set the view description, and then call one of the two versions of
      // start().  When  the server transaction is complete, the client
      // object's on_complete() method will be invoked. 
      ////////////////////////////////////////////////////////////
      class ViewRemover: public ClientBase, public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         ViewRemoverClient *client;

         ////////////////////////////////////////////////////////////
         // view_id
         ////////////////////////////////////////////////////////////
         uint4 view_id;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ViewRemover():
            client(0),
            state(state_standby),
            view_id(0)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ViewRemover()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // get_view_id
         ////////////////////////////////////////////////////////////
         uint4 get_view_id() const
         { return view_id; }

         ////////////////////////////////////////////////////////////
         // set_view_id
         ////////////////////////////////////////////////////////////
         void set_view_id(uint4 id)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            view_id = id;
         }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef ViewRemoverClient client_type;
         void start(client_type *client_, router_handle router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            state = state_delegate;
            client = client_;
            ClientBase::start(router);
         }
         void start(client_type *client_, ClientBase *other)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            state = state_delegate;
            client = client_;
            ClientBase::start(other);
         }

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         {
            client = 0;
            state = state_standby;
            ClientBase::finish();
         }

         ////////////////////////////////////////////////////////////
         // describe_outcome
         ////////////////////////////////////////////////////////////
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      protected:
         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure_);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);
      };
   };
};


#endif

