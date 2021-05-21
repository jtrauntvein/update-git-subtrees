/* Cora.LgrNet.ViewsLister.h

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 21 August 2012
   Last Change: Tuesday 21 August 2012
   Last Commit: $Date: 2012-08-22 09:16:13 -0600 (Wed, 22 Aug 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_ViewsLister_h
#define Cora_LgrNet_ViewsLister_h

#include "Cora.ClientBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace LgrNet
   {
      ////////////////////////////////////////////////////////////
      // class ViewsListerClient
      ////////////////////////////////////////////////////////////
      class ViewsLister;
      class ViewsListerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_logon = 1,
            failure_session_broken = 2,
            failure_unsupported = 3,
            failure_server_security = 4
         };
         virtual void on_failure(ViewsLister *lister, failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when all of the initial views have been received.
         ////////////////////////////////////////////////////////////
         virtual void on_started(ViewsLister *lister)
         { }

         ////////////////////////////////////////////////////////////
         // on_view_added
         //
         // Called when a view has been added.
         ////////////////////////////////////////////////////////////
         virtual void on_view_added(
            ViewsLister *lister,
            uint4 view_id,
            StrUni const &view_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_view_changed
         //
         // Called when a view's configuration has been changed.
         ////////////////////////////////////////////////////////////
         virtual void on_view_changed(
            ViewsLister *lister,
            uint4 view_id,
            StrUni const &view_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_view_removed
         //
         // Called when a view has been removed.
         ////////////////////////////////////////////////////////////
         virtual void on_view_removed(
            ViewsLister *lister,
            uint4 view_id,
            StrUni const &view_name)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class ViewsLister
      //
      // This class defines a component that wraps the LgrNet Enumerate Views
      // transaction.  In order to use this component, the application must
      // provide a client object that extends class ViewsListerClient.  It
      // should then create an instance of this class and call one of the two
      // versions of start().  If the LgrNet transaction is successfully
      // started, the component will call the client's on_view_added() method
      // for each of the current set of views and will then call the client's
      // on_started() method.  Thereafter, the component will call the client's
      // on_view_added(), on_view_changed(), or on_view_removed() methods
      // whenever it receives a notification from the server.  
      ////////////////////////////////////////////////////////////
      class ViewsLister: public ClientBase, public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         ViewsListerClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_before_start,
            state_started
         } state;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ViewsLister():
            client(0),
            state(state_standby)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ViewsLister()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef ViewsListerClient client_type;
         void start(client_type *client_, ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(other_component);
         }
         void start(client_type *client_, router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(router);
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
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // describe_failure
         ////////////////////////////////////////////////////////////
         static void describe_failure(std::ostream &out, client_type::failure_type failure);

      protected:
         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router,
            Csi::Messaging::Message *message);
      };
   };
};


#endif

