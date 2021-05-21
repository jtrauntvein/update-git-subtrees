/* Cora.Broker.BrokerBase.h

   Copyright (C) 2000, 2007 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 25 May 2000
   Last Change: Thursday 29 November 2007
   Last Commit: $Date: 2007-11-29 15:04:32 -0600 (Thu, 29 Nov 2007) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Broker_BrokerBase_h
#define Cora_Broker_BrokerBase_h


#include "Cora.ClientBase.h"
#include "Cora.Broker.Defs.h"


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class BrokerBase
      //
      // Defines a base class on which to build client classes that access data broker functionality
      // on the cora server. The cora server provides two transactions for accessing data brokers:
      //
      //  1 - Open brokers by ID: This can open any broker object that is defined by the cora
      //                          server. Support for this transaction is provided by the
      //                          open_broker_id property.
      //
      //  2 - Open active broker by name: This transaction allows an active or the statistics
      //                          counter broker to be opened using the name assigned to the
      //                          broker. Support for this transaction is provided through the
      //                          open_broker_active_name property.
      ////////////////////////////////////////////////////////////
      class BrokerBase: public ClientBase
      {
      private:
         //@group Properties definition
         ////////////////////////////////////////////////////////////
         // open_broker_active_name
         //
         // The name of the active data broker (or statistics counter) that should be opened. If
         // this string is empty, the open_broker_id property will be used.
         ////////////////////////////////////////////////////////////
         StrUni open_broker_active_name;

         ////////////////////////////////////////////////////////////
         // open_broker_id
         //
         // The identifier for the broker that should be opened. If this value is zero and the value
         // for the open_broker_active_name property is an empty string, a std::invalid_arguments
         // exception will be thrown by start().
         ////////////////////////////////////////////////////////////
         uint4 open_broker_id;

         ////////////////////////////////////////////////////////////
         // broker_name
         //
         // Holds the name of the broker that has been opened.  This will be the same as the active
         // broker name if the client is started that way or will be name associated with the
         // specified broker id if the client is started with that.  This value cannot be directly
         // written to from the client but can be read using get_broker_name
         ////////////////////////////////////////////////////////////
         StrUni broker_name;

         ////////////////////////////////////////////////////////////
         // broker_access_level
         ////////////////////////////////////////////////////////////
         uint4 broker_access_level;
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         BrokerBase();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~BrokerBase();

         //@group Properties access
         ////////////////////////////////////////////////////////////
         // set_open_broker_active_name
         //
         // Sets the open_broker_name property to the specified string. If the specified string is
         // non-empty, the open_broker_id property will be set to zero as well.
         ////////////////////////////////////////////////////////////
         void set_open_broker_active_name(
            StrUni const &open_broker_active_name_);

         ////////////////////////////////////////////////////////////
         // set_open_broker_id
         //
         // Sets the open_broker_id property to the specified value. If the specified value is
         // non-zero, the open_broker_name and broker_name properties will be set to empty strings
         // unless the name of the broker is provided in the optional second parameter.
         ////////////////////////////////////////////////////////////
         void set_open_broker_id(
            uint4 open_broker_id_,
            StrUni const &broker_name_ = L"");
         //@endgroup

         ////////////////////////////////////////////////////////////
         // get_broker_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_broker_name() const { return broker_name; }

         ////////////////////////////////////////////////////////////
         // get_open_broker_active_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_open_broker_active_name() const { return open_broker_active_name; }

         ////////////////////////////////////////////////////////////
         // get_open_broker_id
         ////////////////////////////////////////////////////////////
         uint4 get_open_broker_id() const { return open_broker_id; }

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
         // get_broker_access_level
         ////////////////////////////////////////////////////////////
         uint4 get_broker_access_level() const
         { return broker_access_level; }

      protected:
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
            char const *msg);

         //@group events to derived classes
         ////////////////////////////////////////////////////////////
         // on_brokerbase_ready
         //
         // Called when the session has been successfully established with the broker.
         ////////////////////////////////////////////////////////////
         virtual void on_brokerbase_ready() = 0;

         ////////////////////////////////////////////////////////////
         // on_brokerbase_failure
         //
         // Called when a failure has prevented the logon or broker open transactions from
         // executing.
         ////////////////////////////////////////////////////////////
         enum brokerbase_failure_type
         {
            brokerbase_failure_unknown,
            brokerbase_failure_logon,
            brokerbase_failure_session,
            brokerbase_failure_invalid_id,
            brokerbase_failure_unsupported,
            brokerbase_failure_security
         };
         virtual void on_brokerbase_failure(brokerbase_failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(
            std::ostream &out, brokerbase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_brokerbase_session_failure
         //
         // Called when the broker session has failed while in a ready state.
         ////////////////////////////////////////////////////////////
         virtual void on_brokerbase_session_failure() = 0;
         //@endgroup

         //@group Event notifications overloaded from class ClientBase
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

      protected:
         ////////////////////////////////////////////////////////////
         // state
         //
         // Records the current object state associated with this class. Note that this name
         // overloads a member of the same name (but different type) in the base class.
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            brokerbase_state_standby,
            brokerbase_state_delegate,
            brokerbase_state_attach,
            brokerbase_state_ready,
         } state;

         ////////////////////////////////////////////////////////////
         // broker_session
         //
         // The session number for the broker
         ////////////////////////////////////////////////////////////
         uint4 broker_session;

      private:
         ////////////////////////////////////////////////////////////
         // on_open_broker_ses_ack
         ////////////////////////////////////////////////////////////
         void on_open_broker_ses_ack(Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_broker_exception
         ////////////////////////////////////////////////////////////
         void on_broker_exception(Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_enum_brokers_not
         ////////////////////////////////////////////////////////////
         void on_enum_brokers_not(Csi::Messaging::Message *msg);
      };
   };
};

#endif
