/* Cora.Sec2.Locker.h

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 31 December 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Sec2_Locker_h
#define Cora_Sec2_Locker_h

#include "Cora.Sec2.Sec2Base.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Sec2
   {
      //@group class forward declarations
      class Locker;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class LockerClient
      ////////////////////////////////////////////////////////////
      class LockerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            Locker *locker)
         { }

         ////////////////////////////////////////////////////////////
         // on_stopped
         ////////////////////////////////////////////////////////////
         enum stopped_reason_type
         {
            stopped_reason_unknown = 0,
            stopped_reason_requested = 1,
            stopped_reason_connection_failed = 2,
            stopped_reason_invalid_logon = 3,
            stopped_reason_insufficient_access = 4,
            stopped_reason_unsupported = 5,
            stopped_reason_already_locked = 6,
         };
         virtual void on_stopped(
            Locker *locker,
            stopped_reason_type reason) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class Locker
      //
      // Defines a component that allows an application to maintain a lock on
      // the security interface for LoggerNet servers that support the Security2
      // interface.  In order to use this component, an application must provide
      // an object that inherits from class LockerClient (this class is
      // typedefed as client_type within this class definition).  It must then
      // create an instance of this class, call set_logon_name() and
      // set_logon_password() as appropriate, and then call one of the two
      // versions of start().  The client object's on_started() method will be
      // called after the server transaction has been successfully started.  If
      // the server transaction could not be started or if, after being started,
      // it could not continue, the client object's on_stopped() method will be
      // invoked.
      //
      // The client can cancel the lock by calling finish(), cancel(), or by
      // deleting the Locker instance.  Calling finish() or deleting the locker
      // will prevent any notifications from reaching the client.  If the
      // application calls cancel(), the on_stopped() notification will be sent
      // to the client.
      ////////////////////////////////////////////////////////////
      class Locker:
         public Sec2Base,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Locker();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Locker();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef LockerClient client_type;

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
         // cancel
         ////////////////////////////////////////////////////////////
         virtual bool cancel();

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

      protected:
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
            state_active,
            state_cancel,
         } state;

         ////////////////////////////////////////////////////////////
         // lock_tran_no
         ////////////////////////////////////////////////////////////
         uint4 lock_tran_no;
      };
   };
};


#endif
