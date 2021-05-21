/* Cora.LgrNet.BrokerLister.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 28 July 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_LgrNet_BrokerLister_h
#define Cora_LgrNet_BrokerLister_h

#include "Cora.ClientBase.h"
#include "Csi.InstanceValidator.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class BrokerLister;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class BrokerListerClient
      //
      // Declares the interface for a client to an object of class BrokerLister.
      ////////////////////////////////////////////////////////////
      class BrokerListerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called after the initial set of broker names have been received and the transaction is
         // in a steady state.
         //////////////////////////////////////////////////////////// 
         virtual void on_started(BrokerLister *lister)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called at any time after start() has been invoked and returned to report an
         // unrecoverable failure
         //////////////////////////////////////////////////////////// 
         enum failure_type
         {
            failure_unknown = 0,
            failure_connection_failed = 1,
            failure_invalid_logon = 2,
            failure_server_security = 3,
         };
         virtual void on_failure(BrokerLister *lister, failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_broker_added
         //
         // called when the lister has become aware of a new broker object that matches the mask
         //////////////////////////////////////////////////////////// 
         typedef Cora::Broker::Type::Code broker_type_code;
         virtual void on_broker_added(
            BrokerLister *lister,
            StrUni const &broker_name,
            uint4 broker_id,
            broker_type_code type)
         { }

         ////////////////////////////////////////////////////////////
         // on_broker_deleted
         //
         // Called when the lister has become aware that a databroker object has been deleted
         //////////////////////////////////////////////////////////// 
         virtual void on_broker_deleted(
            BrokerLister *lister,
            StrUni const &broker_name,
            uint4 broker_id,
            broker_type_code type)
         { }

         ////////////////////////////////////////////////////////////
         // on_broker_renamed
         ////////////////////////////////////////////////////////////
         virtual void on_broker_renamed(
            BrokerLister *lister,
            StrUni const &old_broker_name,
            StrUni const &new_broker_name,
            uint4 broker_id,
            broker_type_code type)
         { }
      };

      
      ////////////////////////////////////////////////////////////
      // class BrokerLister
      //
      // Defines an object that lists the names of data brokers that are defined on a server. This
      // is similar to the BrokerBrowser however, this class is lighter weight in that it does not
      // load the entire set of tables for each broker.
      //
      // A client can use this class by creating an instance of it, invoking various set_xxx()
      // methods to set up the properties, and invoking start(). If the transaction can continue,
      // the client's on_broker_added() may be invoked several times until the on_started() method
      // is invoked. At this point, the client will be up to date with all brokers defined by the
      // server. If, at any point following that, a broker is added or deleted, the client will be
      // notified through invocations of its on_broker_added() or on_broker_deleted() methods.
      //
      // An unrecoverable error will be reported to the client via its on_failure() method at which
      // time the lister will have returned to a standby state.
      ////////////////////////////////////////////////////////////
      class BrokerLister: public ClientBase, public Csi::EvReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // enum broker_mask_type
         //
         // Defines the masks that control which classes of brokers will be reported.
         //////////////////////////////////////////////////////////// 
         enum broker_mask_type
         {
            broker_mask_active = 0x01,
            broker_mask_backup = 0x02,
            broker_mask_client_defined = 0x04,
            broker_mask_statistics = 0x08,
         };

      private:
         //@group properties declarations
         ////////////////////////////////////////////////////////////
         // broker_mask
         //
         // Controls which brokers (and associated tables, etc) will be exposed by this transaction.
         //////////////////////////////////////////////////////////// 
         byte broker_mask;
         //@endgroup
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         BrokerLister();

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~BrokerLister();

         //@group properties set methods
         // These methods will succeed only while the transaction is in a standby (before start)
         // state. 

         ////////////////////////////////////////////////////////////
         // add_broker_type
         //
         // Enables brokers of the type specified by mask to be reported
         //////////////////////////////////////////////////////////// 
         void add_broker_type(broker_mask_type mask);

         ////////////////////////////////////////////////////////////
         // remove_broker_type
         //
         // Disables brokers of the type specified by mask from being reported.
         //////////////////////////////////////////////////////////// 
         void remove_broker_type(broker_mask_type mask);

         ////////////////////////////////////////////////////////////
         // set_broker_mask
         //
         // Directly sets the broker mask. Values supplied should be the results of and'ing and or'ing
         // broker_mask_type values together.
         //////////////////////////////////////////////////////////// 
         void set_broker_mask(byte broker_mask_);
         //@endgroup
         
         ////////////////////////////////////////////////////////////
         // start
         //
         // Called to start the transaction. The client and the router must be valid. The value of
         // default_net_session must either be zero (the default) or must be a legitimate opened
         // session. This method must also be called while the transaction objet is in a standby
         // state. If any of these conditions is not met, a std::invalid_argument or exc_invalid_state
         // exception will be thrown..
         ////////////////////////////////////////////////////////////
         typedef BrokerListerClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         //
         // Called to end the transaction and place it back into a standby state. If the transaction
         // is already in a standby state, this method will have no effect
         //////////////////////////////////////////////////////////// 
         virtual void finish();

      protected:
         //@group Methods overloaded from class ClientBase
         ////////////////////////////////////////////////////////////
         // onNetMessage
         //////////////////////////////////////////////////////////// 
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

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
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         //////////////////////////////////////////////////////////// 
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // on_enum_not
         //////////////////////////////////////////////////////////// 
         void on_enum_not(Csi::Messaging::Message *message);

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
            state_before_active,
            state_active,
         } state;

         ////////////////////////////////////////////////////////////
         // brokers
         //
         // Stores the name associated with each unique broker ID
         ////////////////////////////////////////////////////////////
         typedef std::map<uint4, StrUni> brokers_type;
         brokers_type brokers;
      };
   };
};

#endif
