/* Cora.Sec2.StatusMonitor.h

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 23 December 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2019-10-29 16:13:19 -0600 (Tue, 29 Oct 2019) $ 
   Last Changed by: $Author: amortenson $

*/

#ifndef Cora_Sec2_StatusMonitor_h
#define Cora_Sec2_StatusMonitor_h

#include "Cora.Sec2.Sec2Base.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Sec2
   {
      //@group class forward declarations
      class StatusMonitor;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class StatusMonitorClient
      ////////////////////////////////////////////////////////////
      class StatusMonitorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called after the server transaction has been successfully started.
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            StatusMonitor *monitor,
            bool security_enabled)
         { }

         ////////////////////////////////////////////////////////////
         // on_security_changed
         //
         // Called when the security status of the LoggerNet server has changed.
         ////////////////////////////////////////////////////////////
         virtual void on_security_changed(
            StatusMonitor *monitor,
            bool security_enabled)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_connection_failed = 1,
            failure_logon = 2,
            failure_insufficient_access = 3,
            failure_unsupported = 4,
         };
         virtual void on_failure(
            StatusMonitor *monitor,
            failure_type failure) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class StatusMonitor
      //
      // Defines a component that allows an application to monitor whether security is enabled or
      // disabled on a LoggerNet server.  In order to use this component, an application must
      // provide an object derived from class StatusMonitorClient and create an instance of this
      // class.  Once the application has set up the logon parameters by calling set_logon_name()
      // and set_logon_password(), the application can then call one of the two versions of
      // start().  If the server transaction is started, the client object's on_started() method
      // will be called.  Thereafter, the client object's on_security_changed() method will be
      // invoked each time that a change in the LoggerNet server status occurs.
      //
      // If, at any time, the server transaction fails, the client objects on_failure() method will
      // be invoked and the component returned to a standby state.  The component can also be
      // placed into a standby state at any time by calling finish().
      ////////////////////////////////////////////////////////////
      class StatusMonitor:
         public Sec2Base,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         StatusMonitor();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~StatusMonitor();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef StatusMonitorClient client_type;
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

         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(std::ostream &out, client_type::failure_type failure);

      protected:
         ////////////////////////////////////////////////////////////
         // on_sec2base_ready
         ////////////////////////////////////////////////////////////
         virtual void on_sec2base_ready();

         ////////////////////////////////////////////////////////////
         // on_sec2base_failure
         ////////////////////////////////////////////////////////////
         virtual void on_sec2base_failure(sec2base_failure_type failure);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

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
            state_active
         } state;
      };
   };
};


#endif
