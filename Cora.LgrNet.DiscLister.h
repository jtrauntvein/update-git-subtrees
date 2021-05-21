/* Cora.LgrNet.DiscLister.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 21 October 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_DiscLister_h
#define Cora_LgrNet_DiscLister_h

#include <list>
#include "Cora.ClientBase.h"
#include "CsiTypeDefs.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class DiscLister;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class DiscListerClient
      ////////////////////////////////////////////////////////////
      class DiscListerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // class drive_type
         ////////////////////////////////////////////////////////////
         
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         struct drive_type
         {
         public:
            ////////////////////////////////////////////////////////////
            // root_path
            ////////////////////////////////////////////////////////////
            StrAsc root_path;

            ////////////////////////////////////////////////////////////
            // type_code
            ////////////////////////////////////////////////////////////
            enum type_code_type
            {
               type_fixed = 1,
               type_removable = 2,
               type_remote = 3,
               type_cdrom = 4,
               type_ramdisc = 5,
            } type_code;

            ////////////////////////////////////////////////////////////
            // size
            ////////////////////////////////////////////////////////////
            int8 size;

            ////////////////////////////////////////////////////////////
            // free_space
            ////////////////////////////////////////////////////////////
            int8 free_space;
         };
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_session_broken = 3,
            outcome_server_security_blocked = 4,
            outcome_unsupported = 5,
         };
         typedef std::list<drive_type> drives_type;
         virtual void on_complete(
            DiscLister *lister,
            outcome_type outcome,
            drives_type const &drives) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class DiscLister
      //
      // Defines a component that helps the application get the list of disc
      // drives currently  known on the server host machine.  In order to use
      // this component, an application must provide a class derived from class
      // DiscListerClient (also typedefed as client_type within this
      // declaration).  It should then create an instance of this class and
      // invoke its start() method.  When the server transaction is complete,
      // the component will invoke the client's on_complete method.
      ////////////////////////////////////////////////////////////
      class DiscLister:
         public Csi::EventReceiver,
         public ClientBase
      {
      private:
         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
         } state;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         DiscListerClient *client;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         DiscLister():
            client(0),
            state(state_standby)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~DiscLister()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // start (with new router)
         ////////////////////////////////////////////////////////////
         typedef DiscListerClient client_type;
         void start(
            client_type *client_,
            router_handle &router);

         ////////////////////////////////////////////////////////////
         // start (with other component)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &event);

         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure();
      };
   };
};


#endif
