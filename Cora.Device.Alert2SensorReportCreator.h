/* Cora.Device.Alert2SensorReportCreator.h

   Copyright (C) 2016, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 14 July 2016
   Last Change: Wednesday 20 July 2016
   Last Commit: $Date: 2016-09-14 15:26:16 -0600 (Wed, 14 Sep 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_Alert2SensorReportCreator_h
#define Cora_Device_Alert2SensorReportCreator_h
#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      class Alert2SensorReportCreator;


      /**
       * Defines the application interface to the Alert2SensorReportCreator component.  The
       * application must provide an object that extends this class in order to use the component.
       */
      class Alert2SensorReportCreatorClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the server transaction has been completed.
          *
          * @param sender Specifies the component that is calling this method.
          *
          * @param outcome Specifies the outcome code for the server transaction.
          */
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_failure_logon = 2,
            outcome_failure_session = 3,
            outcome_failure_security = 4,
            outcome_failure_unsupported = 5,
            outcome_failure_invalid_device_name = 6,
            outcome_failure_invalid_report_name = 7
         };
         virtual void on_complete(
            Alert2SensorReportCreator *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines a component that can be used to add a sensor report collect area to an ALERT2
       * station device.  In order to use this component, the application must provide an object
       * that extends class Alert2SensorReportCreatorClient.  It should then create an instance of
       * this class, set appropriate properties including device name and area name, and call one of
       * the two versions of start().  When the server transaction is complete, the client's
       * on_complete() method will be called.
       */
      class Alert2SensorReportCreator: public DeviceBase, public Csi::EventReceiver
      {
         /**
          * Specifies the name of the collect area that will be created.
          */
         StrUni report_name;

         /**
          * Specifies the sensor ID for this report.
          */
         uint2 sensor_id;

         /**
          * Specifies the client to this component.
          */
         Alert2SensorReportCreatorClient *client;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;
         
      public:
         /**
          * Default constructor
          */
         Alert2SensorReportCreator():
            client(0),
            state(state_standby),
            sensor_id(0)
         { }

         /**
          * Destructor
          */
         virtual ~Alert2SensorReportCreator()
         { finish(); }

         /**
          * @return Returns the report name
          */
         StrUni const &get_report_name() const
         { return report_name; }

         /**
          * @param value Specifies the report name.
          */
         void set_report_name(StrUni const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            report_name = value;
         }

         /**
          * @return Returns the sensor ID.
          */
         uint2 get_sensor_id() const
         { return sensor_id; }

         /**
          * @param value Specifies the sensor id.
          */
         void set_sensor_id(uint2 value)
         {
            if(state == state_standby)
               sensor_id = value;
            else
               throw exc_invalid_state();
         }

         /**
          * Called to start the server transaction.
          *
          * @param client_ Specifies the application object that will receive notification on
          * completion.
          *
          * @param router Specifies a router for a newly established connection.
          *
          * @param other_component Specifies the component with which we should share the connection
          * and logon information.
          */
         typedef Alert2SensorReportCreatorClient client_type;
         void start(client_type *client_, router_handle router)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            if(state != state_standby)
               throw exc_invalid_state();
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }
         void start(client_type *client_, ClientBase *other_component)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            if(state != state_standby)
               throw exc_invalid_state();
            client = client_;
            state = state_delegate;
            DeviceBase::start(other_component);
         }

         /**
          * Called to release any resources.
          */
         virtual void finish()
         {
            state = state_standby;
            client = 0;
            DeviceBase::finish();
         }

         /**
          * Overloads the base class version to send the start message.
          */
         virtual void on_devicebase_ready();

         /**
          * Overloads the base class version to handle a report of failure.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         /**
          * Overloads the notification of session failure.
          */
         virtual void on_devicebase_session_failure()
         { on_devicebase_failure(devicebase_failure_session); }

         /**
          * Overloads the incoming message handler.
          */
         virtual void onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message);

         /**
          * Overloads the event handler.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Formats the specified outcome to the specified stream.
          *
          * @param out Specifies the stream to which the message will be formatted.
          *
          * @param outcome Specifies the outcome to format.
          */
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);
      };
   };
};


#endif

