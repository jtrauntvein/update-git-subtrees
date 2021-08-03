/* Cora.PbRouter.CommResourceManager.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 19 February 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $
   CVS $Header: /home/group/cvs2/cora/coratools/Cora.PbRouter.CommResourceManager.h,v 1.2 2005/03/25 15:31:34 jon Exp $

*/

#ifndef Cora_PbRouter_CommResourceManager_h
#define Cora_PbRouter_CommResourceManager_h

#include "Cora.PbRouter.PbRouterBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace PbRouter
   {
      //@group class forward declarations
      class CommResourceManager;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class CommResourceManagerClient
      ////////////////////////////////////////////////////////////
      class CommResourceManagerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            CommResourceManager *manager)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_server_session_lost = 1,
            failure_invalid_logon = 2,
            failure_server_security_blocked = 3,
            failure_invalid_router_id = 4,
            failure_unsupported = 5,
            failure_unreachable = 6,
            failure_router_shut_down = 7,
         };
         virtual void on_failure(
            CommResourceManager *manager,
            failure_type failure) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class CommResourceManager
      //
      // Declares a component that can be used to manage the communication resource(s) associated
      // with the specified address.  The presence of this transaction in the PakBus router will
      // serve to make the router keep a session open with the specified device as long as the
      // device can be reached.  The server will also send periodic ping messages to the specified
      // address while this transaction is present even if there is no other activity going on with
      // the node in question.  This component pereforms a similar task to that performed by the
      // Cora::Device::CommResourceManager component.  The main difference between these two
      // components is that this component will work with any deivce that is known by the PakBus
      // router whereas the device version must work with a device that is instantiated in the
      // server's network map.
      //
      // In order to use this component, an application must provide an object that is derived from
      // class CommResourceManagerClient (this type is typedefed as client_type in the definition of
      // this class).  It should then create an instance of this class, set the appropriate
      // attributes including router address and pakbus address, and then call start().  If the
      // component is successful at starting the transaction with the server, the component will
      // invoke the client object's on_started() method.  If, at any time, the transaction cannot
      // continue, the component will invoke the client's on_failure() method.
      //
      // The application can stop the comm resource management transaction at any time by calling
      // finish() or by deleting the component.  
      ////////////////////////////////////////////////////////////
      class CommResourceManager:
         public PbRouterBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // enum priority_type
         ////////////////////////////////////////////////////////////
         enum priority_type
         {
            priority_high = 0,
            priority_normal = 1,
            priority_low = 2
         };

      private:
         ////////////////////////////////////////////////////////////
         // priority
         ////////////////////////////////////////////////////////////
         priority_type priority;

         ////////////////////////////////////////////////////////////
         // pakbus_address
         ////////////////////////////////////////////////////////////
         uint2 pakbus_address;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         CommResourceManager();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~CommResourceManager();

         ////////////////////////////////////////////////////////////
         // get_priority
         ////////////////////////////////////////////////////////////
         priority_type get_priority() const
         { return priority; }

         ////////////////////////////////////////////////////////////
         // get_pakbus_address
         ////////////////////////////////////////////////////////////
         uint2 get_pakbus_address() const
         { return pakbus_address; }

         ////////////////////////////////////////////////////////////
         // set_priority
         ////////////////////////////////////////////////////////////
         void set_priority(priority_type priority_);

         ////////////////////////////////////////////////////////////
         // set_pakbus_address
         ////////////////////////////////////////////////////////////
         void set_pakbus_address(uint2 pakbus_address_);

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef CommResourceManagerClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
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
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_ready();

         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_failure(pbrouterbase_failure_type failure);

      private:
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
      };
   }; 
};


#endif
