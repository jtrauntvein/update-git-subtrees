/* Cora.Device.DeviceTimeEstimator.h

   Copyright (C) 2001, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 26 April 2001
   Last Change: Monday 02 August 2010
   Last Commit: $Date: 2010-08-02 12:09:54 -0600 (Mon, 02 Aug 2010) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_DeviceTimeEstimator_h
#define Cora_Device_DeviceTimeEstimator_h

#include "Cora.Device.ClockSetter.h"
#include "Cora.Device.SettingsEnumerator.h"
#include "Cora.LgrNet.ServerTimeEstimator.h"
#include "Cora.Broker.DataAdvisor.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class DeviceTimeEstimator;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class DeviceTimeEstimatorClient
      //
      // The application that uses a DeviceTimeEstimator object must provide an object derived from
      // this class to receive notifications regarding estimator events.
      ////////////////////////////////////////////////////////////
      class DeviceTimeEstimatorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when the estimator has successfully connected to the server and received the
         // response from the device clock check command.
         //
         // The application cannot successfully get the time estimate from the estimator object
         // until this method has been invoked.
         ////////////////////////////////////////////////////////////
         virtual void on_start(
            DeviceTimeEstimator *estimator) = 0;

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called when the estimator cannot be started properly (connect to the server, execute the
         // device clock check, etc) or the number of failures has exceeded the estimator's
         // maximum_failure_tolerance property. This can also be called if the connection to the
         // server or the device is lost.
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_session = 1,
            failure_invalid_logon = 2,
            failure_server_security_blocked = 3,
            failure_communication_failed = 4,
            failure_communication_disabled = 5,
            failure_logger_security_blocked = 6,
            failure_invalid_device_name = 7,
            failure_unsupported = 8
         };
         virtual void on_failure(
            DeviceTimeEstimator *estimator,
            failure_type failure) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class DeviceTimeEstimator
      //
      // Defines a component that attempts to make estimates of a device's time based upon
      // previously read clock values.
      //
      // An application can use this component by creating an instance of DeviceTimeEstimator and
      // supplying a client object derived from DeviceTimeEstimatorClient to receive notifications
      // from the component.
      //
      // This component differs from other components in that it relies completely on a ClockSetter
      // component to communicate with the server and to estimate the time. Properties such as logon
      // name, password, and device name will be delegated to the ClockSetter component.
      ////////////////////////////////////////////////////////////
      class DeviceTimeEstimator:
         public ClockSetterClient,
         public Csi::EventReceiver,
         public Cora::LgrNet::ServerTimeEstimatorClient,
         public Cora::Broker::DataAdvisorClient,
         public SettingsEnumeratorClient
      {
         typedef Cora::Device::DeviceBase::router_handle router_handle;
         typedef Cora::Broker::DataAdvisor::unread_records_type records_type;

      private:
         //@group component properties
         ////////////////////////////////////////////////////////////
         // device_name
         ////////////////////////////////////////////////////////////
         StrUni device_name;
         
         ////////////////////////////////////////////////////////////
         // minimum_refresh_interval
         //
         // Specifies the minimum interval, in milli-seconds, that the component should use to read
         // the datalogger clock. After the first clock value has been read, the component will not
         // check the datalogger clock again until this interval has elapsed. The default value for
         // this property is 15 minutes.
         ////////////////////////////////////////////////////////////
         uint4 minimum_refresh_interval;

         ////////////////////////////////////////////////////////////
         // maximum_comm_failure_tolerance
         //
         // Specifies the maximum number of failed clock check attempts due to communication
         // failures that this component should tolerate before it reports an error to the client. 
         ////////////////////////////////////////////////////////////
         uint4 maximum_comm_failure_tolerance;

         ////////////////////////////////////////////////////////////
         // allow_auto_clock_check
         //
         // This property will disable the auto checking of the data logger
         // clock based on the minimum_refresh_interval.
         ////////////////////////////////////////////////////////////
         bool allow_auto_clock_check;
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         DeviceTimeEstimator();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~DeviceTimeEstimator();

         //@group property access methods
         ////////////////////////////////////////////////////////////
         // set_device_name
         ////////////////////////////////////////////////////////////
         void set_device_name(StrUni const &device_name_);

         ////////////////////////////////////////////////////////////
         // get_device_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_device_name() const 
         { return device_name; }

         ////////////////////////////////////////////////////////////
         // set_minimum_refresh_interval
         ////////////////////////////////////////////////////////////
         void set_minimum_refresh_interval(uint4 interval_);

         ////////////////////////////////////////////////////////////
         // get_minimum_refresh_interval
         ////////////////////////////////////////////////////////////
         uint4 get_minimum_refresh_interval() const 
         { return minimum_refresh_interval; }

         ////////////////////////////////////////////////////////////
         // set_maximum_comm_failure_tolerance
         ////////////////////////////////////////////////////////////
         void set_maximum_comm_failure_tolerance(uint4 tolerance);

         ////////////////////////////////////////////////////////////
         // get_maximum_comm_failure_tolerance
         ////////////////////////////////////////////////////////////
         uint4 get_maximum_comm_failure_tolerance() const 
         { return maximum_comm_failure_tolerance; }

         ////////////////////////////////////////////////////////////
         // set_allow_auto_clock_check
         ////////////////////////////////////////////////////////////
         void set_allow_auto_clock_check(bool allow);

         ////////////////////////////////////////////////////////////
         // get_allow_auto_clock_check
         ////////////////////////////////////////////////////////////
         bool get_allow_auto_clock_check()
         { return allow_auto_clock_check; }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // start
         //
         // Should be called by the application while this component is in a finished state. This
         // will cause the component to create the appropriate connections to the server and attempt
         // a clock check with the station specified by the device_name property. If these
         // operations succeed, this component will invoke the client's on_ready() method. If a
         // failure should occur during this initialisation or at any time following, the component
         // will invoke the client's on_failure() method and return to a standby state.
         //
         // While this component is in an active state (indicated by the client receiving a
         // on_started() notification), the client can invoke the get_estimate() method.
         ////////////////////////////////////////////////////////////
         typedef DeviceTimeEstimatorClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // get_estimate
         //
         // Returns the current estimate of the device's time if this component is in an active
         // state. If the component is not in an active state, an std::exception derived object will
         // be thrown.
         //
         // The component will evaluate the refresh interval each time this method is invoked to see
         // if it needs to refresh its base time value from the device.
         ////////////////////////////////////////////////////////////
         Csi::LgrDate get_estimate();

         ////////////////////////////////////////////////////////////
         // manual_clock_check
         //
         // Forces the datalogger clock to be checked now
         ////////////////////////////////////////////////////////////
         void manual_clock_check();

         ////////////////////////////////////////////////////////////
         // finish
         //
         // Called by the application to return this component to a standby state.
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // describe_failure
         ////////////////////////////////////////////////////////////
         static void describe_failure(std::ostream &out, client_type::failure_type failure);
         
      private:
         //@group Csi::EvReceiver
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(
            Csi::SharedPtr<Csi::Event> &ev);
         //@endgroup

         //@group device settings lister

         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            SettingsEnumerator *lister);

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            SettingsEnumerator *lister,
            SettingsEnumeratorClient::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_setting_changed
         ////////////////////////////////////////////////////////////
         virtual void on_setting_changed(
            SettingsEnumerator *lister,
            Csi::SharedPtr<Setting> &setting);

         //@endgroup

         //@group Cora::LgrNet::ServerTimeEstimator
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when the estimator is ready to make estimates of the server time. Once this
         // method is called, the client will be able to call the estimator's get_server_time()
         // synchronously at any time following. 
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            Cora::LgrNet::ServerTimeEstimator *estimator);

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called if a failure occurs that prevents the estimator from making reliable estimates. 
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            Cora::LgrNet::ServerTimeEstimator *estimator,
            Cora::LgrNet::ServerTimeEstimatorClient::failure_type failure);
         //@endgroup

         //@group Cora::Broker::DataAdvisorClient
         ////////////////////////////////////////////////////////////
         // on_advise_ready
         //
         // Called when the advise transaction has been successfully begun
         ////////////////////////////////////////////////////////////
         virtual void on_advise_ready(
            Cora::Broker::DataAdvisor *tran)
         { }

         ////////////////////////////////////////////////////////////
         // on_advise_failure
         //
         // Called when a failure has occurred at some point in the advise
         // transaction
         ////////////////////////////////////////////////////////////
         virtual void on_advise_failure(
            Cora::Broker::DataAdvisor *tran,
            Cora::Broker::DataAdvisorClient::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_advise_record
         // 
         // Called when a new data record has been received
         ////////////////////////////////////////////////////////////
         virtual void on_advise_record(
            Cora::Broker::DataAdvisor *tran);
         //@endgroup

         //@group ClockSetterClient
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         virtual void on_complete(
            ClockSetter *setter,
            outcome_type outcome,
            Csi::LgrDate const &logger_time,
            int8 nanoseconds_difference);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // needs_clock_check
         //
         // Evaluates based upon the statistics whether the clock needs to be
         // checked.  If the clock needs to be checked and a clock chgeck is
         // not already pending, a clock check will be initiated with the
         // device. 
         ////////////////////////////////////////////////////////////
         bool needs_clock_check();

      private:
         ////////////////////////////////////////////////////////////
         // settings_lister
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<SettingsEnumerator> settings_lister;
         
         ////////////////////////////////////////////////////////////
         // clock_checker
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<ClockSetter> clock_checker;

         ////////////////////////////////////////////////////////////
         // server_time_estimator
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<Cora::LgrNet::ServerTimeEstimator> server_time_estimator;

         ////////////////////////////////////////////////////////////
         // data_advisor
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<Cora::Broker::DataAdvisor> data_advisor;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_waiting_server_time,
            state_waiting_settings,
            state_waiting_statistics_advise,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // comm_failure_count
         ////////////////////////////////////////////////////////////
         uint4 comm_failure_count;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // last_logger_time
         ////////////////////////////////////////////////////////////
         Csi::LgrDate last_logger_time;

         ////////////////////////////////////////////////////////////
         // last_logger_time_base
         ////////////////////////////////////////////////////////////
         uint4 last_logger_time_base;

         ////////////////////////////////////////////////////////////
         // waiting_for_check
         //
         // Set to true if this component is waiting for a check.
         ////////////////////////////////////////////////////////////
         bool waiting_for_check;

         ////////////////////////////////////////////////////////////
         // last_clock_check
         ////////////////////////////////////////////////////////////
         Csi::LgrDate last_clock_check;

         ////////////////////////////////////////////////////////////
         // last_clock_set
         ////////////////////////////////////////////////////////////
         Csi::LgrDate last_clock_set;

         ////////////////////////////////////////////////////////////
         // last_clock_diff
         ////////////////////////////////////////////////////////////
         int8 last_clock_diff;

         ////////////////////////////////////////////////////////////
         // station_bias
         //
         // the milli-second bias for this station from the server time.
         ////////////////////////////////////////////////////////////
         int8 station_bias;

         ////////////////////////////////////////////////////////////
         // class exc_invalid_state
         //
         // Defines the class of object that will be thrown (as an exception) when an operation (such
         // as property set) is attempted on an object of class derived from this class when the state
         // is invalid.
         ////////////////////////////////////////////////////////////
         class exc_invalid_state: public std::exception
         {
         public:
            exc_invalid_state()
            { trace("exc_invalid_state constructor"); }
            
            virtual char const *what() const throw ()
            { return "Invalid state for attempted operation"; }
         };
      };
   };
};


#endif
