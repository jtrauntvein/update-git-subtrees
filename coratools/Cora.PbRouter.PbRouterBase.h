/* Cora.PbRouter.PbRouterBase.h

   Copyright (C) 2002, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 10 June 2002
   Last Change: Tuesday 13 December 2005
   Last Commit: $Date: 2019-11-22 15:34:57 -0600 (Fri, 22 Nov 2019) $ (UTC)
   Last Changed by: $Author: amortenson $

*/

#ifndef Cora_PbRouter_PbRouterBase_h
#define Cora_PbRouter_PbRouterBase_h

#include "Cora.ClientBase.h"
#include "Cora.PbRouter.Defs.h"


namespace Cora
{
   namespace PbRouter
   {
      ////////////////////////////////////////////////////////////
      // class PbRouterBase
      //
      // Defines a base class for components that access the functionality in the CsiLgrNet PakBus
      // Router Interface.  
      ////////////////////////////////////////////////////////////
      class PbRouterBase: public ClientBase
      {
      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // pakbus_router_name
         //
         // Specifies the name of the router with which this object is to maintain a session. 
         ////////////////////////////////////////////////////////////
         StrUni pakbus_router_name;

         ////////////////////////////////////////////////////////////
         // pbr_access_level
         ////////////////////////////////////////////////////////////
         uint4 pbr_access_level;
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         PbRouterBase();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~PbRouterBase();

         //@group properties access methods
         ////////////////////////////////////////////////////////////
         // set_pakbus_router_name
         ////////////////////////////////////////////////////////////
         void set_pakbus_router_name(StrUni const &pakbus_router_name_);

         ////////////////////////////////////////////////////////////
         // get_pakbus_router_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_pakbus_router_name() const
         { return pakbus_router_name; } 
         //@endgroup

         ////////////////////////////////////////////////////////////
         // start (new connection)
         ////////////////////////////////////////////////////////////
         virtual void start(router_handle &router);

         ////////////////////////////////////////////////////////////
         // start (use existing connection)
         ////////////////////////////////////////////////////////////
         virtual void start(ClientBase *other_client);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // get_pbr_access_level
         ////////////////////////////////////////////////////////////
         uint4 get_pbr_access_level() const
         { return pbr_access_level; }

         ////////////////////////////////////////////////////////////
         // get_pbrouter_address
         ////////////////////////////////////////////////////////////
         uint2 get_pbrouter_address() const
         { return pbrouter_address; }

      protected:
         //@group Notifications to derived classes
         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_ready
         //
         // Called when the session with the PakBus router object has been successfully opened. 
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_ready() = 0;

         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_failure
         ////////////////////////////////////////////////////////////
         enum pbrouterbase_failure_type
         {
            pbrouterbase_failure_unknown,
            pbrouterbase_failure_logon,
            pbrouterbase_failure_session,
            pbrouterbase_failure_invalid_router_id,
            pbrouterbase_failure_unsupported,
            pbrouterbase_failure_security,
         };
         virtual void on_pbrouterbase_failure(pbrouterbase_failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_pbrouter_address_change
         //
         // Called when the Pakbus address of the router has changed.
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouter_address_change(uint2 address)
         { }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);
         
         ////////////////////////////////////////////////////////////
         // onNetSesBroken
         ////////////////////////////////////////////////////////////
         virtual void onNetSesBroken(
            Csi::Messaging::Router *rtr,
            uint4 session_no,
            uint4 reason,
            char const *why);

         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure();

      protected:
         ////////////////////////////////////////////////////////////
         // pbrouter_session
         //
         // Handle to the PakBus router session.
         ////////////////////////////////////////////////////////////
         uint4 pbrouter_session;

         ////////////////////////////////////////////////////////////
         // pbrouterbase_state
         ////////////////////////////////////////////////////////////
         enum pbrouterbase_state_type
         {
            pbrouterbase_state_standby,
            pbrouterbase_state_delegate,
            pbrouterbase_state_attach,
            pbrouterbase_state_ready
         } pbrouterbase_state;

         ////////////////////////////////////////////////////////////
         // pbrouter_address
         //
         // Stores the router address as reported by the server
         ////////////////////////////////////////////////////////////
         uint2 pbrouter_address;

      public:
         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(
            std::ostream &out, pbrouterbase_failure_type failure);
      };
   };
};


#endif
