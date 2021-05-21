/* Cora.LgrNet.DirectoryCreator.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 22 October 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_DirectoryCreator_h
#define Cora_LgrNet_DirectoryCreator_h

#include "Cora.ClientBase.h"
#include "Csi.InstanceValidator.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class DirectoryCreator;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class DirectoryCreatorClient
      ////////////////////////////////////////////////////////////
      class DirectoryCreatorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_session_broken = 3,
            outcome_server_security_blocked = 4,
            outcome_unsupported = 5,
            outcome_create_failed = 6,
         };
         virtual void on_complete(
            DirectoryCreator *creator,
            outcome_type outcome,
            StrAsc const &failure_reason) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class DirectoryCreator
      //
      // Defines a component that can create a directory on the LoggerNet
      // server's local file system.  In order to use this component, an
      // application must provide a client object derived from class
      // DirectoryCreatorClient (also known as DirectoryCreator::client_type).
      // The application should create an instance of this class, invoke
      // methods to set properties including set_path_name(), and then invoke
      // the start() method.  When the LoggerNet server transaction is
      // complete, the component will invoke the client object's on_complete()
      // method. 
      ////////////////////////////////////////////////////////////
      class DirectoryCreator:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_connect,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         DirectoryCreatorClient *client;

         ////////////////////////////////////////////////////////////
         // path_name
         ////////////////////////////////////////////////////////////
         StrAsc path_name;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         DirectoryCreator():
            state(state_standby),
            client(0)
         { }


         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~DirectoryCreator()
         { finish(); }

         
         ////////////////////////////////////////////////////////////
         // set_path_name
         ////////////////////////////////////////////////////////////
         void set_path_name(StrAsc const &path_name_)
         {
            if(state == state_standby)
               path_name = path_name_;
            else
               throw exc_invalid_state();
         }

         
         ////////////////////////////////////////////////////////////
         // get_path_name
         ////////////////////////////////////////////////////////////
         StrAsc const &get_path_name() const
         { return path_name; }

         
         ////////////////////////////////////////////////////////////
         // start (using new router)
         ////////////////////////////////////////////////////////////
         typedef DirectoryCreatorClient client_type;
         void start(
            client_type *client_,
            router_handle &router)
         {
            if(state == state_standby)
            {
               if(client_type::is_valid_instance(client_))
               {
                  client = client_;
                  state = state_connect;
                  ClientBase::start(router);
               }
               else 
                  throw std::invalid_argument("Invalid client pointer");
            }
            else
               throw exc_invalid_state();
         }


         ////////////////////////////////////////////////////////////
         // start (using other component)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            ClientBase *other_component)
         {
            if(state == state_standby)
            {
               if(client_type::is_valid_instance(client_))
               {
                  client = client_;
                  state = state_connect;
                  ClientBase::start(other_component);
               }
               else 
                  throw std::invalid_argument("Invalid client pointer");
            }
            else
               throw exc_invalid_state();
         }


         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         {
            state = state_standby;
            client = 0;
            ClientBase::finish();
         }


      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

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
