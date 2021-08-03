/* Cora.Device.FslGetter.h

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 23 October 2020
   Last Change: Saturday 24 October 2020
   Last Commit: $Date: 2020-10-24 08:00:58 -0600 (Sat, 24 Oct 2020) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_FslGetter_h
#define Cora_Device_FslGetter_h
#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      class FslGetter;

      
      /**
       * Defines the interface that must be implemented by an application object in order to use the
       * FslGetter component.
       */
      class FslGetterClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the server transaction has been completed.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param outcome Specifies the outcome of the server transaction.
          *
          * @param labels Specifies the FSL format that was retrieved.
          */
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_failure_logon = 2,
            outcome_failure_invalid_device_name = 3,
            outcome_failure_security = 4,
            outcome_failure_session = 5,
            outcome_failure_unsupported = 6,
            outcome_failure_no_labels = 7,
            outcome_failure_invalid_format = 8
         };
         virtual void on_complete(FslGetter *sender, outcome_type outcome, StrAsc const &labels) = 0;
      };


      /**
       * Defines a component that wraps around the LoggerNet Device Get Final Storage Labels
       * transaction.  In order to use this component, the application must provide an object that
       * inherits from class FslGetterClient.  It must then create an instance of this class, set
       * the device name, and call one of the two versions of start().  When the server transacion
       * is complete, the component will call the client's on_complete() method.
       */
      class FslGetter: public DeviceBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the client for this component.
          */
         FslGetterClient *client;

         /**
          * Specifies the state for this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Specifies the desired format.  Defaults to JSON.
          */
      public:
         enum output_format_type
         {
            format_json = 1,
            format_fsl = 2
         };
      private:
         output_format_type output_format;

      public:
         /**
          * Constructor
          */
         FslGetter():
            client(0),
            state(state_standby),
            output_format(format_json)
         { }

         /**
          * Destructor
          */
         virtual ~FslGetter()
         { finish(); }

         /**
          * @return Returns the selected output format.
          */
         output_format_type get_output_format() const
         { return output_format; }

         /**
          * @param value Specifies the output formate for the labels.
          */
         void set_output_format(output_format_type value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            output_format = value;
         }
         
         /**
          * Starts the server transaction.
          *
          * @param client_ Specifies the client that will be notified of completion.
          *
          * @param router Specifies a router that is newly created and not yet connected to the
          * server.
          *
          * @param other_client Specifies another component that has an active connection to the
          * server that this component can borrow.
          */
         typedef FslGetterClient client_type;
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
          * Overloads the base class version to reset the state of this component.
          */
         virtual void finish() override
         {
            state = state_standby;
            client = 0;
            DeviceBase::finish();
         }

         /**
          * Overloads the base class version to handle async events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev) override;

         /**
          * Formats the specified outcome code to the given stream.
          *
          * @param out Specifies the stream to which the description will be written.
          *
          * @param outcome Specifies the outcome code to format
          */
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

      protected:
         /**
          * Overloads the base class version to start the server transaction.
          */
         virtual void on_devicebase_ready() override;

         /**
          * Overloads the base class version to handle a failure notification.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure) override;

         /**
          * Overloads the base class version to handle a session failure notification.
          */
         virtual void on_devicebase_session_failure() override
         { on_devicebase_failure(devicebase_failure_session); }
         
         /**
          * Overloads the base class version to handle incoming messages.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message) override;
      };
   };
};


#endif
