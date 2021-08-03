/* Cora.LgrNet.CollectionStatusMonitor.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 09 August 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.CollectionStatusMonitor.h"
#include "Cora.Device.ScheduledTablesMonitor.h"
#include "Cora.Broker.DataAdvisor.h"
#include "Cora.Broker.Value.h"
#include <assert.h>


namespace Cora
{
   namespace LgrNet
   {
      namespace CollectionStatusMonitorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         protected:
            CollectionStatusMonitor *monitor;
            CollectionStatusMonitorClient *client;
            friend class Cora::LgrNet::CollectionStatusMonitor;

         public:
            event_base(uint4 event_id,
                       CollectionStatusMonitor *monitor_,
                       CollectionStatusMonitorClient *client_):
               Event(event_id,monitor_),
               monitor(monitor_),
               client(client_)
            { }

            virtual void notify() = 0;
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public event_base
         {
         public:
            static uint4 const event_id;

            static void create_and_post(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client);

            virtual void notify()
            { client->on_started(monitor); }

         private:
            event_started(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client):
               event_base(event_id,monitor,client)
            { } 
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::LgrNet::CollectionStatusMonitor::event_started");


         void event_started::create_and_post(
            CollectionStatusMonitor *monitor,
            CollectionStatusMonitorClient *client)
         {
            try { (new event_started(monitor,client))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            static uint4 const event_id;
            typedef CollectionStatusMonitorClient::failure_type failure_type;
            failure_type failure;

            static void create_and_post(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               failure_type failure);

            virtual void notify()
            { client->on_failure(monitor,failure); }

         private:
            event_failure(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               failure_type failure_):
               event_base(event_id,monitor,client),
               failure(failure_)
            { } 
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::LgrNet::CollectionStatusMonitor::event_failure");


         void event_failure::create_and_post(
            CollectionStatusMonitor *monitor,
            CollectionStatusMonitorClient *client,
            failure_type failure)
         {
            try { (new event_failure(monitor,client,failure))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_station_added
         ////////////////////////////////////////////////////////////
         class event_station_added: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni station_name;
            bool is_scheduled;

            static void create_and_post(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               StrUni const &station_name,
               bool is_scheduled);

            virtual void notify()
            { client->on_station_added(monitor,station_name,is_scheduled); }

         private:
            event_station_added(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               StrUni const &station_name_,
               bool is_scheduled_):
               event_base(event_id,monitor,client),
               station_name(station_name_),
               is_scheduled(is_scheduled_)
            { } 
         };


         uint4 const event_station_added::event_id =
         Csi::Event::registerType("Cora::LgrNet::CollectionStatusMonitor::event_station_added");


         void event_station_added::create_and_post(
            CollectionStatusMonitor *monitor,
            CollectionStatusMonitorClient *client,
            StrUni const &station_name,
            bool is_scheduled)
         {
            try { (new event_station_added(monitor,client,station_name,is_scheduled))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_station_deleted
         ////////////////////////////////////////////////////////////
         class event_station_deleted: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni station_name;

            static void create_and_post(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               StrUni const &station_name);

            virtual void notify()
            { client->on_station_deleted(monitor,station_name); }

         private:
            event_station_deleted(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               StrUni const &station_name_):
               event_base(event_id,monitor,client),
               station_name(station_name_)
            { } 
         };


         uint4 const event_station_deleted::event_id =
         Csi::Event::registerType("Cora::LgrNet::CollectionStatusMonitor::event_station_deleted");


         void event_station_deleted::create_and_post(
            CollectionStatusMonitor *monitor,
            CollectionStatusMonitorClient *client,
            StrUni const &station_name)
         {
            try { (new event_station_deleted(monitor,client,station_name))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_station_scheduled
         ////////////////////////////////////////////////////////////
         class event_station_scheduled: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni station_name;
            bool is_scheduled;

            static void create_and_post(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               StrUni const &station_name,
               bool is_scheduled);

            virtual void notify()
            { client->on_station_scheduled(monitor,station_name,is_scheduled); }

         private:
            event_station_scheduled(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               StrUni const &station_name_,
               bool is_scheduled_):
               event_base(event_id,monitor,client),
               station_name(station_name_),
               is_scheduled(is_scheduled_)
            { } 
         };


         uint4 const event_station_scheduled::event_id =
         Csi::Event::registerType("Cora::LgrNet::CollectionStatusMonitor::event_station_scheduled");


         void event_station_scheduled::create_and_post(
            CollectionStatusMonitor *monitor,
            CollectionStatusMonitorClient *client,
            StrUni const &station_name,
            bool is_scheduled)
         {
            try { (new event_station_scheduled(monitor,client,station_name,is_scheduled))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_table_added
         ////////////////////////////////////////////////////////////
         class event_table_added: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni station_name;
            bool is_scheduled;
            StrUni table_name;

            static void create_and_post(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               StrUni const &station_name,
               StrUni const &table_name,
               bool is_scheduled);

            virtual void notify()
            { client->on_table_added(monitor,station_name,table_name,is_scheduled); }

         private:
            event_table_added(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               StrUni const &station_name_,
               StrUni const &table_name_,
               bool is_scheduled_):
               event_base(event_id,monitor,client),
               station_name(station_name_),
               table_name(table_name_),
               is_scheduled(is_scheduled_)
            { } 
         };


         uint4 const event_table_added::event_id =
         Csi::Event::registerType("Cora::LgrNet::CollectionStatusMonitor::event_table_added");


         void event_table_added::create_and_post(
            CollectionStatusMonitor *monitor,
            CollectionStatusMonitorClient *client,
            StrUni const &station_name,
            StrUni const &table_name,
            bool is_scheduled)
         {
            try
            {
               (new event_table_added(monitor,client,station_name,table_name,is_scheduled))->post();
            }
            catch(Csi::Event::BadPost &)
            { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_table_scheduled
         ////////////////////////////////////////////////////////////
         class event_table_scheduled: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni station_name;
            bool is_scheduled;
            StrUni table_name;

            static void create_and_post(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               StrUni const &station_name,
               StrUni const &table_name,
               bool is_scheduled);

            virtual void notify()
            { client->on_table_scheduled(monitor,station_name,table_name,is_scheduled); }

         private:
            event_table_scheduled(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               StrUni const &station_name_,
               StrUni const &table_name_,
               bool is_scheduled_):
               event_base(event_id,monitor,client),
               station_name(station_name_),
               table_name(table_name_),
               is_scheduled(is_scheduled_)
            { } 
         };


         uint4 const event_table_scheduled::event_id =
         Csi::Event::registerType("Cora::LgrNet::CollectionStatusMonitor::event_table_scheduled");


         void event_table_scheduled::create_and_post(
            CollectionStatusMonitor *monitor,
            CollectionStatusMonitorClient *client,
            StrUni const &station_name,
            StrUni const &table_name,
            bool is_scheduled)
         {
            try
            {
               (new event_table_scheduled(monitor,
                                          client,
                                          station_name,
                                          table_name,
                                          is_scheduled))->post();
            }
            catch(Csi::Event::BadPost &)
            { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_table_deleted
         ////////////////////////////////////////////////////////////
         class event_table_deleted: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni station_name;
            StrUni table_name;

            static void create_and_post(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               StrUni const &station_name,
               StrUni const &table_name);

            virtual void notify()
            { client->on_table_deleted(monitor,station_name,table_name); }

         private:
            event_table_deleted(
               CollectionStatusMonitor *monitor,
               CollectionStatusMonitorClient *client,
               StrUni const &station_name_,
               StrUni const &table_name_):
               event_base(event_id,monitor,client),
               station_name(station_name_),
               table_name(table_name_)
            { } 
         };


         uint4 const event_table_deleted::event_id =
         Csi::Event::registerType("Cora::LgrNet::CollectionStatusMonitor::event_table_deleted");


         void event_table_deleted::create_and_post(
            CollectionStatusMonitor *monitor,
            CollectionStatusMonitorClient *client,
            StrUni const &station_name,
            StrUni const &table_name)
         {
            try
            {
               (new event_table_deleted(monitor,client,station_name,table_name))->post();
            }
            catch(Csi::Event::BadPost &)
            { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class Station
         //
         // Represents the information from a single station in the network.
         ////////////////////////////////////////////////////////////
         class Station:
            public Device::ScheduledTablesMonitorClient,
            public Broker::DataAdvisorClient
         {
         public:
            ////////// constructor
            Station(CollectionStatusMonitor *monitor_,
                    StrUni const &station_name);

            ////////// destructor
            virtual ~Station();

         protected:
            //@group class ScheduledTablesMonitorClient overloaded methods
            ////////// on_started
            virtual void on_started(
               Device::ScheduledTablesMonitor *monitor);

            ////////// on_failure
            virtual void on_failure(
               Device::ScheduledTablesMonitor *monitor,
               Device::ScheduledTablesMonitorClient::failure_type failure);

            ////////// on_table_added
            virtual void on_table_added(
               Device::ScheduledTablesMonitor *monitor,
               StrUni const &table_name,
               bool scheduled);

            ////////// on_table_deleted
            virtual void on_table_deleted(
               Device::ScheduledTablesMonitor *monitor,
               StrUni const &table_name);

            ////////// on_table_scheduled
            virtual void on_table_scheduled(
               Device::ScheduledTablesMonitor *monitor,
               StrUni const &table_name,
               bool scheduled);
            //@endgroup

            //@group class DataAdviseClient overloaded methods
            ////////// on_advise_ready
            virtual void on_advise_ready(
               Broker::DataAdvisor *advisor);

            ////////// on_advise_failure
            virtual void on_advise_failure(
               Broker::DataAdvisor *advisor,
               Broker::DataAdvisorClient::failure_type failure);

            ////////// on_advise_record
            virtual void on_advise_record(
               Broker::DataAdvisor *advisor);
            //@endgroup

         private:
            ////////// monitor
            // Pointer to the main monitor object
            CollectionStatusMonitor *monitor;

            ////////// station_name
            StrUni station_name;

            ////////// station_monitor
            // reference to the object responsible for monitoring the station
            Csi::SharedPtr<Device::ScheduledTablesMonitor> station_monitor;

            ////////// stat_advisor
            // The object that is used to read the station statistics to report on whether the
            // station is enabled
            Csi::SharedPtr<Broker::DataAdvisor> stat_advisor;

            ////////// state
            // Keeps track of the initialisation state of this object
            enum state_type
            {
               state_waiting_for_stats,
               state_starting_monitor,
               state_active
            } state;

            ////////// is_scheduled
            // Reflects the value of the scheduleEnabled statistic
            bool is_scheduled;

            ////////////////////////////////////////////////////////////
            // table_states
            //
            // A mapping of table names to table enabled values that keeps track whether a given
            // table is enabled for scheduled collection.
            ////////////////////////////////////////////////////////////
            typedef std::map<StrUni, bool> table_states_type;
            table_states_type table_states;
         };


         Station::Station(CollectionStatusMonitor *monitor_,
                          StrUni const &station_name_):
            monitor(monitor_),
            station_name(station_name_),
            is_scheduled(false)
         {
            // initialise the station monitor
            station_monitor.bind(new Device::ScheduledTablesMonitor);
            station_monitor->set_device_name(station_name);

            // initialise the statistics advisor
            StrUni stat_table_name(station_name + L"_std");

            stat_advisor.bind(new Broker::DataAdvisor);
            stat_advisor->set_open_broker_active_name("__statistics__");
            stat_advisor->set_table_name(stat_table_name);

            // we will get the statistics first so that they are always up to date
            stat_advisor->start(this,monitor);
            state = state_waiting_for_stats;
         } // constructor

         
         Station::~Station()
         {
            station_monitor->finish();
            stat_advisor->finish();
         } // destructor


         void Station::on_started(
            Device::ScheduledTablesMonitor *station_monitor)
         {
            state = state_active;
            monitor->on_station_active(station_name);
         } // on_started

         
         void Station::on_failure(
            Device::ScheduledTablesMonitor *station_monitor,
            Device::ScheduledTablesMonitorClient::failure_type failure)
         {
            typedef CollectionStatusMonitorClient client_type;
            typedef Device::ScheduledTablesMonitorClient this_client_type;
            if(failure != this_client_type::failure_session)
            {
               client_type::failure_type client_failure; 
               switch(failure)
               {
               default:
                  client_failure = client_type::failure_unknown;
                  break;
                  
               case this_client_type::failure_invalid_logon:
                  client_failure = client_type::failure_invalid_logon;
                  break;
                  
               case this_client_type::failure_server_security:
                  client_failure = client_type::failure_server_security;
                  break;
                  
               case this_client_type::failure_unsupported:
                  client_failure = client_type::failure_unsupported;
                  break;
               }
               event_failure::create_and_post(monitor,monitor->client,client_failure);
            }
         } // on_failure

         
         void Station::on_table_added(
            Device::ScheduledTablesMonitor *station_monitor,
            StrUni const &table_name,
            bool scheduled)
         {
            table_states[table_name] = scheduled;
            event_table_added::create_and_post(
               monitor,
               monitor->client,
               station_name,
               table_name,
               scheduled && this->is_scheduled);
         } // on_table_added

         
         void Station::on_table_deleted(
            Device::ScheduledTablesMonitor *station_monitor,
            StrUni const &table_name)
         {
            table_states.erase(table_name);
            event_table_deleted::create_and_post(
               monitor,
               monitor->client,
               station_name,
               table_name);
         } // on_table_deleted

         
         void Station::on_table_scheduled(
            Device::ScheduledTablesMonitor *station_monitor,
            StrUni const &table_name,
            bool scheduled)
         {
            table_states[table_name] = scheduled;
            event_table_scheduled::create_and_post(
               monitor,
               monitor->client,
               station_name,
               table_name,
               scheduled && this->is_scheduled);
         } // on_table_scheduled

         
         void Station::on_advise_ready(
            Broker::DataAdvisor *advisor)
         { }

         
         void Station::on_advise_failure(
            Broker::DataAdvisor *advisor,
            Broker::DataAdvisorClient::failure_type failure)
         {
            typedef CollectionStatusMonitorClient client_type;
            typedef Broker::DataAdvisorClient this_client_type;
            if(failure != this_client_type::failure_table_deleted &&
               failure != this_client_type::failure_connection_failed)
            {
               client_type::failure_type client_failure;
               
               switch(failure)
               {
               default:
                  client_failure = client_type::failure_unknown;
                  break;
                  
               case this_client_type::failure_invalid_logon:
                  client_failure = client_type::failure_invalid_logon;
                  break;
                  
               case this_client_type::failure_server_security:
                  client_failure = client_type::failure_server_security;
                  break;
                  
               case this_client_type::failure_unsupported:
                  client_failure = client_type::failure_unsupported;
                  break;
               }
               event_failure::create_and_post(monitor,monitor->client,client_failure);
            }
         } // on_advise_failure

         
         void Station::on_advise_record(
            Broker::DataAdvisor *advisor)
         {
            // search for the value that has the proper name
            Broker::DataAdvisor::record_handle record(advisor->get_record());
            for(Broker::Record::iterator i = record->begin();
                i != record->end();
                ++i)
            {
               Broker::Record::value_handle value(*i);
               if(value->get_name() == L"Collection Enabled" ||
                  value->get_name() == L"Collection_Enabled")
               {
                  // extract the value from the statistic
                  byte *v = reinterpret_cast<byte *>(value->get_pointer());
                  bool station_enabled = (*v ? true : false);

                  // determine if we need to start the station monitor
                  if(state == state_waiting_for_stats)
                  {
                     is_scheduled = station_enabled ? true : false;
                     event_station_added::create_and_post(
                        monitor,
                        monitor->client,
                        station_name,
                        is_scheduled);
                     station_monitor->start(this,monitor);
                     state = state_starting_monitor;
                  }
                  else if(station_enabled != is_scheduled)
                  {
                     is_scheduled = station_enabled;
                     event_station_scheduled::create_and_post(
                        monitor,
                        monitor->client,
                        station_name,
                        is_scheduled);
                     for(table_states_type::iterator tsi = table_states.begin();
                         tsi != table_states.end();
                         ++tsi)
                     {
                        event_table_scheduled::create_and_post(
                           monitor,
                           monitor->client,
                           station_name,
                           tsi->first,
                           is_scheduled && tsi->second);
                     }
                  }
                  break;
               }
            }
            advisor->get_next_record();
         } // on_advise_record 
      };


      ////////////////////////////////////////////////////////////
      // class CollectionStatusMonitor definitions
      ////////////////////////////////////////////////////////////

      CollectionStatusMonitor::CollectionStatusMonitor():
         client(0),
         state(state_standby),
         wait_for_start_count(0)
      {
         lister.bind(new BrokerLister);
         lister->set_broker_mask(BrokerLister::broker_mask_active);
      } // constructor


      CollectionStatusMonitor::~CollectionStatusMonitor()
      { finish(); }

      
      void CollectionStatusMonitor::start(
         CollectionStatusMonitorClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(CollectionStatusMonitorClient::is_valid_instance(client_))
            {
               state = state_before_active;
               wait_for_start_count = 0;
               client = client_;
               ClientBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void CollectionStatusMonitor::start(
         CollectionStatusMonitorClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(CollectionStatusMonitorClient::is_valid_instance(client_))
            {
               state = state_before_active;
               wait_for_start_count = 0;
               client = client_;
               ClientBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void CollectionStatusMonitor::finish()
      {
         lister->finish();
         state = state_standby;
         stations.clear();
         ClientBase::finish();
      } // finish

      
      void CollectionStatusMonitor::on_corabase_ready()
      {
         lister->start(this,this);
      } // on_corabase_ready

      
      void CollectionStatusMonitor::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace CollectionStatusMonitorHelpers;
         client_type::failure_type client_failure;

         switch(failure)
         {
         default:
            client_failure = client_type::failure_unknown;
            break;
            
         case corabase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case corabase_failure_session:
            client_failure = client_type::failure_session;
            break;
            
         case corabase_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case corabase_failure_security:
            client_failure = client_type::failure_server_security;
            break;
         } 
         event_failure::create_and_post(this,client,client_failure);
      } // on_corabase_failure

      
      void CollectionStatusMonitor::on_corabase_session_failure()
      {
         using namespace CollectionStatusMonitorHelpers;
         event_failure::create_and_post(this,client,client_type::failure_session);
      } // on_corabase_session_failed

      
      void CollectionStatusMonitor::on_started(BrokerLister *lister)
      {
         using namespace CollectionStatusMonitorHelpers;
         if(stations.empty())
         {
            state = state_active;
            event_started::create_and_post(this,client);
         }
      } // on_started

      
      void CollectionStatusMonitor::on_failure(BrokerLister *lister,
                                               BrokerListerClient::failure_type failure)
      {
         using namespace CollectionStatusMonitorHelpers;
         typedef BrokerListerClient this_client_type;
         client_type::failure_type client_failure;

         switch(failure)
         {
         default:
            client_failure = client_type::failure_unknown;
            break;

         case this_client_type::failure_connection_failed:
            client_failure = client_type::failure_session;
            break;

         case this_client_type::failure_invalid_logon:
            client_failure = client_type::failure_invalid_logon;
            break;

         case this_client_type::failure_server_security:
            client_failure = client_type::failure_server_security;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_failure

      
      void CollectionStatusMonitor::on_broker_added(BrokerLister *lister,
                                                    StrUni const &broker_name,
                                                    uint4 broker_id,
                                                    broker_type_code type)
      {
         using namespace CollectionStatusMonitorHelpers;
         // we will count on the filter in the broker lister to show us only active brokers.
         station_handle station(new Station(this,broker_name));
         stations[broker_name] = station;
         if(state == state_before_active)
            ++wait_for_start_count;
      } // on_broker_added

      
      void CollectionStatusMonitor::on_broker_deleted(
         BrokerLister *lister,
         StrUni const &broker_name,
         uint4 broker_id,
         broker_type_code type)
      {
         using namespace CollectionStatusMonitorHelpers;
         stations.erase(broker_name);
         event_station_deleted::create_and_post(this,client,broker_name);
      } // on_broker_deleted


      void CollectionStatusMonitor::on_broker_renamed(
         BrokerLister *lister,
         StrUni const &broker_name,
         StrUni const &old_name,
         uint4 broker_id,
         broker_type_code type)
      {
         using namespace CollectionStatusMonitorHelpers;
         station_handle station(new Station(this,broker_name));
         stations.erase(old_name);
         stations[broker_name] = station;
      } // on_broker_renamed

      
      void CollectionStatusMonitor::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace CollectionStatusMonitorHelpers;
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         assert(event != 0);

         if(event->getType() == event_failure::event_id)
            finish();
         if(client_type::is_valid_instance(event->client))
            event->notify();
         else
            finish();
      } // receive

      
      void CollectionStatusMonitor::on_station_active(StrUni const &station_name)
      {
         using namespace CollectionStatusMonitorHelpers;
         --wait_for_start_count;
         if(wait_for_start_count == 0)
         {
            state = state_active;
            event_started::create_and_post(this,client);
         }
      } // on_station_active 
   };
};
