/* Cora.PbRouter.Pinger.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 24 January 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_PbRouter_Pinger_h
#define Cora_PbRouter_Pinger_h

#include "Cora.PbRouter.PbRouterBase.h"
#include "Cora.PbRouter.PakctrlMessageSender.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace PbRouter
   {
      //@group class forward declarations
      class Pinger;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class PingerClient
      ////////////////////////////////////////////////////////////
      class PingerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the server transaction has completed.  The value of
         // response_time should be ignored if the response code does not
         // indicate a success.
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_invalid_router_id = 3,
            outcome_server_permission_denied = 4,
            outcome_server_session_failed = 5,
            outcome_communication_failed = 6,
            outcome_unsupported = 7, 
            outcome_invalid_pakbus_address = 8,
            outcome_corrupt_echo = 9,
            outcome_unreachable = 10
         };
         virtual void on_complete(
            Pinger *pinger,
            outcome_type outcome,
            uint4 response_time_msec,
            uint2 packet_size_used) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class Pinger
      //
      // Defines a component that can ask the server to ping a PakBus node in
      // one of its networks using the PakCtrl Echo transaction.  In order to
      // use this component, an application must provide an object derived from
      // class PingerClient.  It should then create an instance of class,
      // Pinger, and set appropriate properties including router_id and
      // pakbus_address.  The server transaction will begin after the
      // application has called start() and the application will be notified
      // when the transaction is complete by a call to the client's
      // on_complete() method.
      ////////////////////////////////////////////////////////////
      class Pinger:
         public PbRouterBase,
         public PakctrlMessageSenderClient,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // pakbus_address
         //
         // The address of the node that should be contacted.  The default
         // value is zero.
         ////////////////////////////////////////////////////////////
         uint2 pakbus_address;

         ////////////////////////////////////////////////////////////
         // packet_size
         //
         // Specifies the minimal size for the packet.  If this is greater than
         // the legal limit for PakBus packets, the server will send a packet
         // the size of the legal limit.  The default value for this is 1000
         ////////////////////////////////////////////////////////////
         uint2 packet_size;

         ////////////////////////////////////////////////////////////
         // ping_from_address
         //
         // Optionally specifies the address from which the node should be
         // pinged.  If set to zero (the default), the node will be pinged by
         // the server.  This feature is not supported by all PakBus operating
         // systems. 
         ////////////////////////////////////////////////////////////
         uint2 ping_from_address;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Pinger();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Pinger();

         ////////////////////////////////////////////////////////////
         // set_pakbus_address
         ////////////////////////////////////////////////////////////
         void set_pakbus_address(uint2 pakbus_address_);

         ////////////////////////////////////////////////////////////
         // get_pakbus_address
         ////////////////////////////////////////////////////////////
         uint2 get_pakbus_address() const
         { return pakbus_address; }

         ////////////////////////////////////////////////////////////
         // set_packet_size
         ////////////////////////////////////////////////////////////
         void set_packet_size(uint2 packet_size_);

         ////////////////////////////////////////////////////////////
         // get_packet_size
         ////////////////////////////////////////////////////////////
         uint2 get_packet_size() const
         { return packet_size; }

         ////////////////////////////////////////////////////////////
         // set_ping_from_address
         ////////////////////////////////////////////////////////////
         void set_ping_from_address(uint2 ping_from_address_);

         ////////////////////////////////////////////////////////////
         // get_ping_from_address
         ////////////////////////////////////////////////////////////
         uint2 get_ping_from_address() const
         { return ping_from_address; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef PingerClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

      protected:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         virtual void on_complete(
            PakctrlMessageSender *sender,
            PakctrlMessageSenderClient::outcome_type outcome,
            response_type &response,
            uint4 round_trip_time);
         
         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_ready();

         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_failure(pbrouterbase_failure_type);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router,
            Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // init_sender
         ////////////////////////////////////////////////////////////
         void init_sender();
         
      private:
         ////////////////////////////////////////////////////////////
         // sender
         //
         // Used to send the remote echo message if the ping_from_address
         // member is set to a non-zero value.
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<PakctrlMessageSender> sender;
         
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
