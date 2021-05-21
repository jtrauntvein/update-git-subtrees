/* Cora.Device.CollectArea.CollectAreaRenamer.h

   Copyright (C) 2016, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Friday 09 September 2016
   Last Change: Friday 09 September 2016
   Last Commit: $Date: 2016-09-12 11:05:12 -0600 (Mon, 12 Sep 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_CollectArea_CollectAreaRenamer_h
#define Cora_Device_CollectArea_CollectAreaRenamer_h
#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         class CollectAreaRenamer;


         /**
          * Defines a base class for a client to the CollectAreaRenamer component.
          */
         class CollectAreaRenamerClient: public Csi::InstanceValidator
         {
         public:
            /**
             * Called by the component after the device transaction has been completed.
             *
             * @param sender Specifies the component reporting this event.
             *
             * @param outcome Specifies the outcome of the transaction,
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
               outcome_failure_invalid_area_name,
               outcome_failure_invalid_new_area_name
            };
            virtual void on_complete(CollectAreaRenamer *sender, outcome_type outcome) = 0;
         };


         /**
          * Defines a component that can be used to rename a client created collect area in the
          * LoggerNet server.  In order to use this component, the application must provide an
          * object that derives from class CollectAreaRenamerClient that will receive notification
          * of completion.  It should create an instance of this class, set its properties including
          * device name, collect area name, and new collect area name, and call one of the two
          * versions of start().  When the LoggerNet transaction is comolete, the client will be
          * notified via its on_complete() method.
          */
         class CollectAreaRenamer: public DeviceBase, public Csi::EventReceiver
         {
         private:
            /**
             * Specifies the collect area that should be renamed.
             */
            StrUni area_name;

            /**
             * Specifies the new name for the collect area.
             */
            StrUni new_name;

            /**
             * Specifies the current state for this component.
             */
            enum state_type
            {
               state_standby,
               state_delegate,
               state_active
            } state;

            /**
             * Specifies the client to the transaction.
             */
            CollectAreaRenamerClient *client;

         public:
            /**
             * Constructor
             */
            CollectAreaRenamer():
               client(0),
               state(state_standby)
            { }

            /**
             * Destructor
             */
            virtual ~CollectAreaRenamer()
            { }

            /**
             * @return Returns the collect area name.
             */
            StrUni const &get_area_name() const
            { return area_name; }

            /**
             * @param value Specifies the value for the collect area name.
             */
            void set_area_name(StrUni const &value)
            {
               if(state != state_standby)
                  throw exc_invalid_state();
               area_name = value;
            }

            /**
             * @return Returns the collect area new name.
             */
            StrUni const &get_new_name() const
            { return new_name; }

            /**
             * @param value Specifies the value of the collect area new name parameter.
             */
            void set_new_name(StrUni const &value)
            {
               if(state != state_standby)
                  throw exc_invalid_state();
               new_name = value;
            }

            /**
             * Starts the LoggerNet transaction.
             *
             * @param client_ Specifies the application object that will receive notification of
             * completion.
             *
             * @param router Specifies a router that has not been previously connected.
             *
             * @param other_client Specifies a client object that already has a valid connection to
             * the LoggerNet server.
             */
            typedef CollectAreaRenamerClient client_type;
            void start(client_type *client_, router_handle router)
            {
               if(state != state_standby)
                  throw exc_invalid_state();
               if(!client_type::is_valid_instance(client_))
                  throw std::invalid_argument("invalid client pointer");
               client = client_;
               state = state_delegate;
               DeviceBase::start(router);
            }
            void start(client_type *client_, ClientBase *other_client)
            {
               if(state != state_standby)
                  throw exc_invalid_state();
               if(!client_type::is_valid_instance(client_))
                  throw std::invalid_argument("invalid client pointer");
               client = client_;
               state = state_delegate;
               DeviceBase::start(other_client);
            }

            /**
             * Returns this component to a standby state.
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
