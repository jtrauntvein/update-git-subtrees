/* Cora.LgrNet.ServerTimeEstimator.h

   Copyright (C) 2001, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 17 April 2001
   Last Change: Thursday 31 December 2020
   Last Commit: $Date: 2020-12-31 12:54:32 -0600 (Thu, 31 Dec 2020) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_LgrNet_ServerTimeEstimator_h
#define Cora_LgrNet_ServerTimeEstimator_h


#include "Cora.ClientBase.h"
#include "Csi.InstanceValidator.h"
#include "Csi.LgrDate.h"


namespace Cora
{
   namespace LgrNet
   {
      class ServerTimeEstimator;


      /**
       * Defines the interface that an application object must implement in order to use the
       * ServerTimeEstirmator component type.
       */
      class ServerTimeEstimatorClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the connection to the server and this component can make estimates of the
          * server time.
          *
          * @param sender Specifies the component calling this method.
          */
         virtual void on_started(ServerTimeEstimator *estimator) = 0;

         /**
          * Called when the connection to the server has failed.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param failure Specifies a code that gives the reason for the failure.
          */
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_logon = 1,
            failure_session_broken = 2,
            failure_unsupported = 3,
            failure_server_security_blocked = 4,
            failure_access_token_invalid = 5,
            failure_access_token_expired = 6
         };
         virtual void on_failure(ServerTimeEstimator *estimator, failure_type failure) = 0;
      };


      /**
       * Defines a component that can be used to maintain a connection to the LoggerNet server and
       * to provide estimates of the current LoggerNet server time.  It does so by periodically
       * polling the server clock and basing future estimates upon the local clock and the
       * difference between the local clock and the server clock when the server clock was last
       * polled.
       *
       * In order to use this component, the application must provide an object that inherits from
       * class ServerTimeEstimatorClient.  It should then create an instance of this component, set
       * logon properties such as logon_name and logon_password, and then call one of the two
       * versions of start().  When the component has successfully connected to the server and
       * polled its clock for the first time, the client's on_started() method will be called.  If
       * the transaction ever fails, the client's on_failure() method will be called.
       */
      class ServerTimeEstimator:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the minimum number of milliseconds that should elapse between server clock
          * polls.
          */
         uint4 check_clock_interval;

      public:
         /**
          * Constructor
          */
         ServerTimeEstimator();

         /**
          * Destructor
          */
         virtual ~ServerTimeEstimator();

         /**
          * @param check_clock_interval_ Specifies the minimum amount of milliseconds that must
          * elapse between server clock polls.
          */
         void set_check_clock_interval(uint4 check_clock_interval_);

         /**
          * @return Returns the minimum amount of milliseconds that must elapse between server clock
          * polls.
          */
         uint4  get_check_clock_interval() const
         { return check_clock_interval; }

         /**
          * Starts the first transaction to poll the server's clock.
          *
          * @param client_ Specifies the application object that will receive notifications from
          * this component.
          *
          * @param router Specifies a messaging router that has not yet been connected.
          *
          * @param other_component Specifies a component that has a connection to the server that
          * this component can share.
          *
          * @param use_own_logon Set to true if this component is sharing a connection with another
          * component but must use its own logon information.
          */
         typedef ServerTimeEstimatorClient client_type;
         void start(client_type *client_, router_handle &router);
         void start(
            client_type *client_, ClientBase *other_component, bool use_own_logon = false);

         /**
          * @return Returns an estimate of the server time based upon the local clock, the value of
          * the server clock when it was last polled, and the difference between the server clock
          * and the local clock when the server clock was last polled.
          */
         Csi::LgrDate get_server_time();

         /**
          * Releases all resources and returns this component to a standby state.
          */
         virtual void finish() override;

         /**
          * Formats the specified failure code to the given stream.
          */
         static void format_failure(std::ostream &out, client_type::failure_type failure);

      protected:
         /**
          * Handles incoming server messages.
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *message) override;

         /**
          * Handles asynch messages.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev) override;

         /**
          * Handles the event when the server connection has been made.
          */
         virtual void on_corabase_ready() override;

         /**
          * Handles a component failure.
          */
         virtual void on_corabase_failure(corabase_failure_type failure) override;

         /**
          * Handles a componnet connection failure.
          */
         virtual void on_corabase_session_failure() override;

      private:
         /**
          * Handles a message that reports the server clock poll outcome.
          */
         void on_get_server_time_ack(Csi::Messaging::Message *message);

         /**
          * Handles a message that reports the server global settings change event.
          */
         void on_server_settings_not(Csi::Messaging::Message *message);
         
      private:
         /**
          * Specifies the application objects that will receive notifications from this component.
          */
         client_type *client;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_before_active,
            state_active
         } state;

         /**
          * Specifies the last clock value that was polled from the server.
          */
         Csi::LgrDate last_server_time;

         /**
          * Used to calculate that time since the server clock was last polled.
          */
         uint4 last_server_time_base;
      };
   };
};


#endif
