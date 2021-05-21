/* Cora.Device.LinkCloser.h

   Copyright (C) 2014, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Thursday 10 April 2014
   Last Change: Thursday 10 April 2014
   Last Commit: $Date: 2014-04-10 10:17:05 -0600 (Thu, 10 Apr 2014) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_LinkCloser_h
#define Cora_Device_LinkCloser_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      class LinkCloser;

      
      /**
       * Defines the interface that a client to the LinkCloser component must
       * implement.
       */
      class LinkCloserClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the LoggerNet transaction has been completed.
          *
          * @param component  Specifies the component reporting this event.
          *
          * @param outcome Specifies the outcome of the LoggerNet transaction.
          */
         enum outcome_type
         {
            outcome_failure_unknown,
            outcome_success,
            outcome_failure_logon,
            outcome_failure_session,
            outcome_failure_unsupported,
            outcome_failure_security,
            outcome_failure_invalid_device_name
         };
         virtual void on_complete(
            LinkCloser *component,
            outcome_type outcome) = 0;
      };


      /**
       * Defines a component that can be used by the application to force a
       * device into an off-line state.  In order to use this component, the
       * application must provide an object that inherits from class
       * LinkCloserClient.  It should then create an instance of this class, set its
       * properties including device name and, optionally, recursive, and then
       * calling one of the two versions of start().  When the LoggerNet
       * transaction is complete, the client's on_complete() method will be
       * called.
       */
      class LinkCloser: public DeviceBase, public Csi::EventReceiver
      {
      private:
         /**
          * Set to true if the closure is to be applied recursively along the
          * whole link.
          */
         bool recursive;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Specifies the client for this component.
          */
         LinkCloserClient *client;
         
      public:
         /**
          * Constructor
          */
         LinkCloser():
            state(state_standby),
            recursive(false),
            client(0)
         { }

         /**
          * Destructor
          */
         virtual ~LinkCloser()
         { finish(); }

         /**
          * Sets the recursive property.  A value should be specified if the
          * link closure is meant to happen on every device in the link.
          *
          * @param recursive_  Specifies whether the closure should be applied
          * on every device in the link.
          */
         void set_recursive(bool recursive_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            recursive = recursive_;
         }

         /**
          * @return Returns the value of the recursive property.
          */
         bool get_recursive() const
         { return recursive; }

         /**
          * Called to start the LoggerNet transaction using a messaging router
          * that has not active connection.
          *
          * @param client_  Specifies the client for this component.
          *
          * @param router  Specifies a messaging router that does not have a
          * current connection to the LoggerNet server.
          */
         typedef LinkCloserClient client_type;
         void start(client_type *client_, router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }

         /**
          * Called to start the LoggerNet transaction using the connection from
          * another client component.
          *
          * @param client_  Specifies the client for this component.
          *
          * @param other_client Specifies a client component that already has
          * an active connection to the LoggerNet server.
          */
         void start(
            client_type *client_, ClientBase *other_client)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            state = state_delegate;
            client = client_;
            DeviceBase::start(other_client);
         }

         /**
          * Called to cancel this component and return it to a finished state.
          */
         virtual void finish()
         {
            client = 0;
            state = state_standby;
            DeviceBase::finish();
         }

         /**
          * Writes a description of the outcome code to the specified stream.
          */
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

         /**
          * Implements the message receiver.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      protected:
         /**
          * Called when the connection to the device is ready.
          */
         virtual void on_devicebase_ready();

         /**
          * Called when a failure has occurred with the device.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         /**
          * Called when the session with the device has failed.
          */
         virtual void on_devicebase_session_failure()
         { on_devicebase_failure(devicebase_failure_session); }

         /**
          * Called when a message has been received.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);
      };
   };
};


#endif
