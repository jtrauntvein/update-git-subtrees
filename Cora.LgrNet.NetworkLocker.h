/* Cora.LgrNet.NetworkLocker.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 29 April 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_NetworkLocker_h
#define Cora_LgrNet_NetworkLocker_h

#include "Cora.ClientBase.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class NetworkLocker;
      //@end group

      
      ////////////////////////////////////////////////////////////
      // class NetworkLockerClient
      ////////////////////////////////////////////////////////////
      class NetworkLockerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called after the server transaction has been started,  
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            NetworkLocker *locker)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called when the server transaction has failed.  The other_logon_name and other_app_name
         // parameters will be filled in if the failure indicates that the network is already
         // lcoked.  These will be empty strings otherwise.
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_logon = 1,
            failure_session_broken = 2,
            failure_unsupported = 3,
            failure_server_security_blocked = 4,
            failure_already_locked = 5
         };
         virtual void on_failure(
            NetworkLocker *locker,
            failure_type failure,
            StrUni const &other_logon_name,
            StrUni const &other_app_name) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class NetworkLocker
      ////////////////////////////////////////////////////////////
      class NetworkLocker:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         NetworkLockerClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
         } state;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         NetworkLocker():
            state(state_standby),
            client(0)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~NetworkLocker()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef NetworkLockerClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client,
            ClientBase *other_component);

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
         // describe_failure
         ////////////////////////////////////////////////////////////
         static void describe_failure(
            std::ostream &out,
            client_type::failure_type failure,
            StrUni const &other_logon_name = L"",
            StrUni const &other_app_name = L"");

      protected:
         ////////////////////////////////////////////////////////////
         // method on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // method on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // method on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure();

         ////////////////////////////////////////////////////////////
         // method receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // method onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg); 
      };
   };
};


#endif
