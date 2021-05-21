/* Cora.Device.CollectArea.CollectAreaDeleter.h

   Copyright (C) 2016, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 14 September 2016
   Last Change: Wednesday 14 September 2016
   Last Commit: $Date: 2016-09-14 15:26:16 -0600 (Wed, 14 Sep 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_CollectArea_CollectAreaDeleter_h
#define Cora_Device_CollectArea_CollectAreaDeleter_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         class CollectAreaDeleter;


         /**
          * Defines the interface that must be implemented by an application object in order to use
          * the CollectAreaDeleter component.
          */
         class CollectAreaDeleterClient: public Csi::InstanceValidator
         {
         public:
            /**
             * Called when the component transaction has been deleted.
             *
             * @param sender Specifies the component reporting this event.
             *
             * @param outcome Specifies the outcome of the transaction.
             */
            enum outcome_type
            {
               outcome_failure_unknown,
               outcome_success,
               outcome_failure_logon,
               outcome_failure_session,
               outcome_failure_security,
               outcome_failure_unsupported,
               outcome_failure_invalid_device_name,
               outcome_failure_invalid_area_name
            };
            virtual void on_complete(CollectAreaDeleter *sender, outcome_type outcome) = 0;
         };


         /**
          * Defines a component that can be used to delete a collect areas from a LoggerNet device.
          * In order to use this component, the application must provide an object derived from
          * class CollectAreaDeleterClient.  It should then create an instance of this class, set
          * its properties including device name and collect area name, and call one of the two
          * versions of start().  When the LoggerNet transaction is complete, the client's
          * on_complete() method will be called.
          */
         class CollectAreaDeleter: public DeviceBase, public Csi::EventReceiver
         {
         private:
            /**
             * Specifies the name of the collect area to be deleted.
             */
            StrUni area_name;

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
             * Specifies the application object that will receive notice when the component is
             * complete.
             */
            CollectAreaDeleterClient *client;

         public:
            /**
             * Default Constructor
             */
            CollectAreaDeleter():
               state(state_standby),
               client(0)
            { }

            /**
             * Destructor
             */
            virtual ~CollectAreaDeleter()
            { finish(); }

            /**
             * @param value Specifies the name of the collect area to be deleted.
             */
            void set_area_name(StrUni const &value)
            {
               if(state != state_standby)
                  throw exc_invalid_state();
               area_name = value;
            }

            /**
             * Initiates the connection to the device.
             *
             * @param client_ Specifies the application object that will receive notice of
             * completion.
             *
             * @param router Specifies a router object that has not previously had a connection to
             * the server
             *
             * @param other_client Specifies a client component that already has an active
             * connection to the server.
             */
            typedef CollectAreaDeleterClient client_type;
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
            void start(client_type *client_, ClientBase *other_component)
            {
               if(state != state_standby)
                  throw exc_invalid_state();
               if(!client_type::is_valid_instance(client_))
                  throw std::invalid_argument("invalid client pointer");
               client = client_;
               state = state_delegate;
               DeviceBase::start(other_component);
            }

            /**
             * Overloads the base class version to clean up this instance
             */
            virtual void finish()
            {
               client = 0;
               state = state_standby;
               DeviceBase::finish();
            }

            /**
             * Overloads the base class version to send the command message to the server.
             */
            virtual void on_devicebase_ready();

            /**
             * Overloads the base class version to report a failure.
             */
            virtual void on_devicebase_failure(devicebase_failure_type failure);

            /**
             * Overloads the base class version to report a session failure.
             */
            virtual void on_devicebase_session_failure()
            { on_devicebase_failure(devicebase_failure_session); }

            /**
             * Overloads the base class version to handle incoming session messages.
             */
            virtual void onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message);

            /**
             * Overloads the base class version to handle the completion event.
             */
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

            /**
             * Formats the specified outcome code to the specified stream.
             */
            static void format_outcome(std::ostream &out, client_type::outcome_type outcome);
         };
      };
   };
};


#endif
