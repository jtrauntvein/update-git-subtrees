/* Cora.LgrNet.UdpDiscoverer.h

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 20 May 2015
   Last Change: Friday 31 July 2015
   Last Commit: $Date: 2015-07-31 10:54:17 -0600 (Fri, 31 Jul 2015) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_UdpDiscoverer_h
#define Cora_LgrNet_UdpDiscoverer_h
#include "Cora.ClientBase.h"
#include "Csi.DevConfig.UdpDiscoverer.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace LgrNet
   {
      class UdpDiscoverer;

      
      /**
       * Defines the interface that must be implemented by an application object that uses the
       * UdpDiscoverer cora component.
       */
      class UdpDiscovererClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Specifies the method that will carry the information about a device that has been
          * discovered.
          *
          * @param sender Specifies the component that is sending this notification.
          *
          * @param device Specifies a handle to an object that describes the device.
          */
         typedef Csi::DevConfig::UdpDiscovererHelpers::Device device_type;
         typedef Csi::SharedPtr<device_type> device_handle;
         virtual void on_device_added(
            UdpDiscoverer *sender, device_handle const &device) = 0;

         /**
          * Called to inform the client that the component has stopped.
          *
          * @param sender Specifis the component that is sending this notification.
          *
          * @param reason Specifies the reason why the component was stopped.
          */
         enum failure_type
         {
            failure_unknown,
            failure_logon,
            failure_session,
            failure_unsupported,
            failure_security,
            failure_network,
            failure_shut_down
         };
         virtual void on_failure(
            UdpDiscoverer *sender, failure_type failure) = 0;
      };


      /**
       * Defines an object that can be used to query the LoggerNet server for devices accessible on
       * the server's local area network.  In order to use this component, the application must
       * provide an object that inherits from class UdpDiscovererClient.  It should then create an
       * instance of this class, optionally set properties, and call one of the two versions of
       * start().  After the component has been started, the component will call the client's
       * on_device_added() when a new device has been detected.  If the transaction cannot continue
       * or be started, the client's on_failure() method will be called.
       */
      class UdpDiscoverer: public ClientBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the client to this component.
          */
         UdpDiscovererClient *client;

         /**
          * Specifies the UDP port that should be used for searching.
          */
         uint2 discover_port;

         /**
          * Specifies the device type mask.
          */
         uint2 device_mask;

         /**
          * Specifies the state of this transaction.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Specifies the number used to start the transaction.
          */
         uint4 server_tran;

      public:
         /**
          * Default constructor.
          */
         UdpDiscoverer():
            client(0),
            device_mask(0xffff),
            discover_port(6785),
            state(state_standby)
         { }

         /**
          * Destructor
          */
         virtual ~UdpDiscoverer()
         { finish(); }

         /**
          * @return Returns the UDP port used for discovery.
          */
         uint4 get_discover_port() const
         { return discover_port; }

         /**
          * @param value Specifies the UDP port used for discovery.  The default value for this
          * property is 6785.
          *
          * @throw exc_invalid_state Throws this exception if the component is already started.
          */
         void set_discover_port(uint2 value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            discover_port = value;
         }

         /**
          * @return Returns the device type mask.
          */
         uint2 get_device_mask() const
         { return device_mask; }

         /**
          * @param value Specifies the value for the device type mask.  A value of 0xffff(the
          * default) will accept all responding devices.
          *
          * @throw exc_invalid_state Throws this exception if the component is already started.
          */
         void set_device_mask(uint2 value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            device_mask = value;
         }

         /**
          * Called to start this component using a newly created router.
          *
          * @param client_ Specifies the client for this component.
          *
          * @param router Specifies a newly created (unattached) messaging router.
          *
          * @throw exc_invalid_state Thrown if the component is not in a state to be started.
          *
          * @throw std::invalid_argument Thrown if the client pointer is invalid.
          */
         typedef UdpDiscovererClient client_type;
         void start(client_type *client_, router_handle router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            state = state_delegate;
            client = client_;
            ClientBase::start(router);
         }

         /**
          * Called to start this component using the connection from another component.
          *
          * @param client_ Specifies the client for this component.
          *
          * @param other_component Specifies a component that has an active connection.
          *
          * @throw exc_invalid_state Thrown if the component is not in a state to be started.
          *
          * @throw std::invalid_argument Thrown if the client pointer is invalid.
          */
         void start(client_type *client_, ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            state = state_delegate;
            client = client_;
            ClientBase::start(other_component);
         }

         /**
          * Cancels the server transaction and releases any resources.
          */
         virtual void finish()
         {
            state = state_standby;
            client = 0;
            ClientBase::finish();
         }

         /**
          * Overloads the base class version to handle any received events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Formats the specified failure code to the specified stream.
          *
          * @param out Specifies the stream where the description will be written.
          *
          8 @param failure Specifies the failure to be formatted.
         */
         static void format_failure(std::ostream &out, client_type::failure_type failure);

      protected:
         /**
          * Overloads the base class version start the transaction.
          */
         virtual void on_corabase_ready();

         /**
          * Overloads the base class version to handle a failure.
          */
         virtual void on_corabase_failure(corabase_failure_type failure);

         /**
          * Overloads the base class version to handle a session failure.
          */
         virtual void on_corabase_session_failure()
         { on_corabase_failure(corabase_failure_session); }

         /**
          * Overloads the base class version to handle an incoming message.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);
      };
   };
};



#endif
