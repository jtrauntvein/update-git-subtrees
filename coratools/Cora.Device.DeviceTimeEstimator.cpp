/* Cora.Device.DeviceTimeEstimator.cpp

   Copyright (C) 2001, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 26 April 2001
   Last Change: Monday 02 August 2010
   Last Commit: $Date: 2011-10-31 16:06:25 -0600 (Mon, 31 Oct 2011) $ 
   Committed by: $Author: jon $
   
*/

#include "Cora.Device.DeviceTimeEstimator.h"
#include "Cora.Broker.ValueTypes.h"
#include "Csi.Utils.h"
#include "Csi.MaxMin.h"
#include "Cora.CommonSettingTypes.h"
#include "Cora.Device.DeviceSettingFactory.h"
#include "Cora.CommonSettingTypes.h"
#include "coratools.strings.h"
#include "Csi.PolySharedPtr.h"
#include <sstream>


namespace Cora
{
   namespace Device
   {
      namespace DeviceTimeEstimatorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public Csi::Event
         {
         public:
            static uint4 const event_id;
            DeviceTimeEstimatorClient *client;
            
         private:
            event_started(
               DeviceTimeEstimator *estimator,
               DeviceTimeEstimatorClient *client_):
               Event(event_id,estimator),
               client(client_)
            { }

         public:
            static void create_and_post(
               DeviceTimeEstimator *estimator,
               DeviceTimeEstimatorClient *client)
            {
               try { (new event_started(estimator,client))->post(); }
               catch(Event::BadPost &) { }
            }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Device::DeviceTimeEstimator::event_started");


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public Csi::Event
         {
         public:
            static uint4 const event_id;
            DeviceTimeEstimatorClient *client;
            typedef DeviceTimeEstimatorClient::failure_type failure_type;
            failure_type failure;

         private:
            event_failure(
               DeviceTimeEstimator *estimator,
               DeviceTimeEstimatorClient *client_,
               failure_type failure_):
               Event(event_id,estimator),
               client(client_),
               failure(failure_)
            { }

         public:
            static void create_and_post(
               DeviceTimeEstimator *estimator,
               DeviceTimeEstimatorClient *client,
               failure_type failure)
            {
               try { (new event_failure(estimator,client,failure))->post(); }
               catch(Event::BadPost &) { }
            }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Device::DeviceTimeEstimator::event_failure");


         ////////////////////////////////////////////////////////////
         // class ZoneBiasFactory
         ////////////////////////////////////////////////////////////
         class ZoneBiasFactory: public DeviceSettingFactory
         {
         public:
            virtual Cora::Setting *make_setting(uint4 setting_id)
            {
               Cora::Setting *rtn = 0;
               if(setting_id == Settings::time_zone_difference)
                  rtn = DeviceSettingFactory::make_setting(setting_id);
               return rtn; 
            }
         };
      };


      ////////////////////////////////////////////////////////////
      // class DeviceTimeEstimator definitions
      ////////////////////////////////////////////////////////////
      DeviceTimeEstimator::DeviceTimeEstimator():
         state(state_standby),
         comm_failure_count(0),
         client(0),
         last_logger_time_base(0),
         minimum_refresh_interval(15 * Csi::LgrDate::msecPerMin),
         maximum_comm_failure_tolerance(5),
         allow_auto_clock_check(true),
         waiting_for_check(false),
         station_bias(0)
      { 
         clock_checker.bind(new ClockSetter); 
         server_time_estimator.bind(new Cora::LgrNet::ServerTimeEstimator);
         data_advisor.bind(new Cora::Broker::DataAdvisor);
         settings_lister.bind(new SettingsEnumerator);
         settings_lister->set_setting_factory(new DeviceTimeEstimatorHelpers::ZoneBiasFactory);
      } // constructor

      
      DeviceTimeEstimator::~DeviceTimeEstimator()
      { 
         finish(); 
      } // destructor

      
      void DeviceTimeEstimator::set_device_name(StrUni const &device_name_)
      {
         if(state == state_standby)
            device_name = device_name_;
         else
            throw exc_invalid_state();
      }


      void DeviceTimeEstimator::set_minimum_refresh_interval(uint4 interval_)
      {
         if(state == state_standby)
            minimum_refresh_interval = interval_;
         else
            throw exc_invalid_state();
      }

      
      void DeviceTimeEstimator::set_maximum_comm_failure_tolerance(uint4 tolerance)
      {
         if(state == state_standby)
            maximum_comm_failure_tolerance = tolerance;
         else
            throw exc_invalid_state();
      }


      void DeviceTimeEstimator::set_allow_auto_clock_check(bool allow)
      {
         if(state == state_standby)
            allow_auto_clock_check = allow;
         else
            throw exc_invalid_state();
      }


      void DeviceTimeEstimator::start(
         DeviceTimeEstimatorClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(DeviceTimeEstimatorClient::is_valid_instance(client_))
            {
               client = client_;
               state = state_waiting_server_time;
               server_time_estimator->start(this,router);
            }
            else
               throw std::invalid_argument("invalid client pointer specified");
         }
         else
            throw exc_invalid_state();
      }


      void DeviceTimeEstimator::start(
         DeviceTimeEstimatorClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(DeviceTimeEstimatorClient::is_valid_instance(client_))
            {
               client = client_;
               state = state_waiting_server_time;
               server_time_estimator->start(this,other_component);
            }
            else
               throw std::invalid_argument("invalid client pointer specified");
         }
         else
            throw exc_invalid_state();
      }


      void DeviceTimeEstimator::on_started(
         SettingsEnumerator *lister)
      {
         try
         {
            state = state_waiting_statistics_advise;
            data_advisor->set_open_broker_active_name("__Statistics__");
            data_advisor->set_table_name(device_name + L"_std");
            data_advisor->add_column(L"Last Clock Check");
            data_advisor->add_column(L"Last Clock Set");
            data_advisor->add_column(L"Last Clock Diff");
            data_advisor->start(this,server_time_estimator.get_rep());
         }
         catch(std::exception &)
         {
            DeviceTimeEstimatorHelpers::event_failure::create_and_post(
               this,
               client,
               client_type::failure_unknown);
         }
      } // on_started


      void DeviceTimeEstimator::on_failure(
         SettingsEnumerator *lister,
         SettingsEnumeratorClient::failure_type reason)
      {
         client_type::failure_type failure;
         switch(reason)
         {
         case SettingsEnumeratorClient::failure_connection_failed:
            failure = client_type::failure_session;
            break;
               
         case SettingsEnumeratorClient::failure_invalid_logon:
            failure = client_type::failure_invalid_logon;
            break;
               
         case SettingsEnumeratorClient::failure_server_security_blocked:
            failure = client_type::failure_server_security_blocked;
            break;
            
         case SettingsEnumeratorClient::failure_device_name_invalid:
            failure = client_type::failure_invalid_device_name;
            break;
            
            
         default:
            failure = client_type::failure_unknown;
            break;
         }
         DeviceTimeEstimatorHelpers::event_failure::create_and_post(this,client,failure);
      } // on_failure


      void DeviceTimeEstimator::on_setting_changed(
         SettingsEnumerator *lister,
         Csi::SharedPtr<Setting> &setting)
      {
         try
         {
            if(setting->get_identifier() == Cora::Device::Settings::time_zone_difference)
            {
               Csi::PolySharedPtr<Cora::Setting, Cora::SettingInt4> bias_setting(setting);
               station_bias = bias_setting->get_value() * Csi::LgrDate::nsecPerMSec;
            }
         }
         catch(std::exception &)
         { }
      } // on_setting_changed

      
      void DeviceTimeEstimator::on_started(
         Cora::LgrNet::ServerTimeEstimator *estimator)
      {
         try
         {
            state = state_waiting_settings;
            settings_lister->set_device_name(device_name);
            settings_lister->start(this,server_time_estimator.get_rep());
         }
         catch(std::exception &)
         {
            DeviceTimeEstimatorHelpers::event_failure::create_and_post(
               this,
               client,
               client_type::failure_unknown);
         }
      }


      void DeviceTimeEstimator::on_failure(
         Cora::LgrNet::ServerTimeEstimator *estimator,
         Cora::LgrNet::ServerTimeEstimatorClient::failure_type failure)
      {
         client_type::failure_type client_failure;
         switch(failure)
         {
            case Cora::LgrNet::ServerTimeEstimatorClient::failure_invalid_logon:
               client_failure = client_type::failure_invalid_logon;
               break;
               
            case Cora::LgrNet::ServerTimeEstimatorClient::failure_session_broken:
               client_failure = client_type::failure_session;
               break;
               
            case Cora::LgrNet::ServerTimeEstimatorClient::failure_unsupported:
               client_failure = client_type::failure_unsupported;
               break;
               
            case Cora::LgrNet::ServerTimeEstimatorClient::failure_server_security_blocked:
               client_failure = client_type::failure_server_security_blocked;
               break;
               
            default:
               client_failure = client_type::failure_unknown;
               break;
         }
         DeviceTimeEstimatorHelpers::event_failure::create_and_post(this,client,client_failure);
      }


      void DeviceTimeEstimator::on_advise_record(
         Cora::Broker::DataAdvisor *tran)
      {
         try
         {
            Cora::Broker::Record &record = *tran->get_record();
            using namespace Cora::Broker::ValueTypes;
            VStamp *clock_val;
            VInt8 *diff_val;

            clock_val = dynamic_cast<VStamp *>(record[0].get_rep());
            if(clock_val)
               last_clock_check = clock_val->get_value();
            clock_val = dynamic_cast<VStamp *>(record[1].get_rep());
            if(clock_val)
               last_clock_set = clock_val->get_value();
            diff_val = dynamic_cast<VInt8 *>(record[2].get_rep());
            if(diff_val)
               last_clock_diff = diff_val->get_value();
            tran->get_next_record();
            if(state != state_active)
            {
               if(!needs_clock_check())
               {
                  state = state_active;
                  DeviceTimeEstimatorHelpers::event_started::create_and_post(
                     this,
                     client);
               }
            }
         }
         catch(std::exception &)
         {
            DeviceTimeEstimatorHelpers::event_failure::create_and_post(this,client,client_type::failure_unknown);
         }
      }


      void DeviceTimeEstimator::on_advise_failure(
         Cora::Broker::DataAdvisor *tran,
         Cora::Broker::DataAdvisorClient::failure_type failure)
      {
         client_type::failure_type client_failure;
         switch(failure)
         {
            case Cora::Broker::DataAdvisorClient::failure_invalid_station_name:
               client_failure = client_type::failure_invalid_device_name;
               break;
               
            case Cora::Broker::DataAdvisorClient::failure_connection_failed:
               client_failure = client_type::failure_communication_failed;
               break;
               
            case Cora::Broker::DataAdvisorClient::failure_unsupported:
               client_failure = client_type::failure_communication_failed;
               break;
               
            default:
               client_failure = client_type::failure_unsupported;
               break;
         }
         DeviceTimeEstimatorHelpers::event_failure::create_and_post(this,client,client_failure);
      }


      void DeviceTimeEstimator::on_complete(
         ClockSetter *setter,
         outcome_type outcome,
         Csi::LgrDate const &logger_time,
         int8 nanoseconds_difference)
      {
         using namespace DeviceTimeEstimatorHelpers;
         waiting_for_check = false;
         if(outcome == outcome_success_clock_checked ||
            outcome == outcome_success_clock_set)
         {
            if(outcome == outcome_success_clock_checked)
            {
               last_clock_check = logger_time;
               last_clock_diff = nanoseconds_difference;
            }
            else
               last_clock_set = logger_time;
         }
         else
         {
            // map the failure into our own type
            client_type::failure_type failure;
            switch(outcome)
            {
               case outcome_session_failed:
                  failure = client_type::failure_session;
                  break;
                  
               case outcome_invalid_logon:
                  failure = client_type::failure_invalid_logon;
                  break;
               
               case outcome_server_security_blocked:
                  failure = client_type::failure_server_security_blocked;
                  break;
                  
               case outcome_communication_failed:
                  failure = client_type::failure_communication_failed;
                  break;
                  
               case outcome_communication_disabled:
                  failure = client_type::failure_communication_disabled;
                  break;
                  
               case outcome_logger_security_blocked:
                  failure = client_type::failure_logger_security_blocked;
                  break;
                  
               case outcome_invalid_device_name:
                  failure = client_type::failure_invalid_device_name;
                  break;
                  
               case outcome_unsupported:
                  failure = client_type::failure_unsupported;
                  break;
                  
               default:
                  failure = client_type::failure_unknown;
                  break;
            }

            // if this is the first attempt, any failure will result in a complete failure
            if(state != state_active ||
               failure != client_type::failure_communication_failed)
               DeviceTimeEstimatorHelpers::event_failure::create_and_post(this,client,failure);
            else if(failure == client_type::failure_communication_failed)
            {
               if(++comm_failure_count >= maximum_comm_failure_tolerance)
                  DeviceTimeEstimatorHelpers::event_failure::create_and_post(this,client,failure);
            }
         }
      } // on_complete


      bool DeviceTimeEstimator::needs_clock_check()
      {
         bool rtn = allow_auto_clock_check;
         if(rtn)
         {
            Csi::LgrDate last_event_time = Csi::csimax(
               last_clock_check,
               last_clock_set);
            Csi::LgrDate current_server_time =
               server_time_estimator->get_server_time() + station_bias;
            int8 diff = (current_server_time - last_event_time).get_nanoSec();
            if(diff < 0)
               diff *= -1;
            if(diff <= (minimum_refresh_interval * Csi::LgrDate::nsecPerMSec))
               rtn = false; 
         }
         if(rtn && !waiting_for_check)
         {
            clock_checker->set_device_name(device_name);
            clock_checker->start(this,server_time_estimator.get_rep());
            waiting_for_check = true;
         }
         return rtn;
      } // needs_clock_check

      
      Csi::LgrDate DeviceTimeEstimator::get_estimate()
      {
         Csi::LgrDate rtn;
         if(state == state_active)
         {
            // check to see if we need sync our clock
            needs_clock_check(); 
            rtn = server_time_estimator->get_server_time() - last_clock_diff + station_bias;
         }
         else
            throw exc_invalid_state();
         return rtn;
      }


      void DeviceTimeEstimator::manual_clock_check()
      {
         clock_checker->set_device_name(device_name);
         clock_checker->start(this,server_time_estimator.get_rep());
         waiting_for_check = true;
      }

      
      void DeviceTimeEstimator::finish()
      {
         clock_checker->finish();
         data_advisor->finish();
         server_time_estimator->finish();
         state = state_standby;
         waiting_for_check = false;
      } // finish


      void DeviceTimeEstimator::describe_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_session:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_session);
            break;
            
         case client_type::failure_invalid_logon:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_logon);
            break;
            
         case client_type::failure_server_security_blocked:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_security);
            break;
            
         case client_type::failure_communication_failed:
            out << common_strings[common_comm_failed];
            break;
            
         case client_type::failure_communication_disabled:
            out << common_strings[common_comm_disabled];
            break;
            
         case client_type::failure_logger_security_blocked:
            out << common_strings[common_logger_security_blocked];
            break;
            
         case client_type::failure_invalid_device_name:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_invalid_device_name);
            break;
            
         default:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_unknown);
            break;
         }
      } // describe_failure

      
      void DeviceTimeEstimator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace DeviceTimeEstimatorHelpers; 
         if(ev->getType() == DeviceTimeEstimatorHelpers::event_failure::event_id)
         {
            DeviceTimeEstimatorHelpers::event_failure *event = static_cast<event_failure *>(ev.get_rep());
            finish();
            if(client_type::is_valid_instance(event->client))
               event->client->on_failure(this,event->failure);
         }
         else if(ev->getType() == DeviceTimeEstimatorHelpers::event_started::event_id)
         {
            DeviceTimeEstimatorHelpers::event_started *event = static_cast<DeviceTimeEstimatorHelpers::event_started *>(ev.get_rep());
            if(client_type::is_valid_instance(event->client))
               event->client->on_start(this);
            else
               finish();
         }
      }
   };
};
