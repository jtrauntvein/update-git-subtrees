/* Cora.Device.ScheduledTablesMonitor.cpp

   Copyright (C) 2000, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 09 August 2000
   Last Change: Wednesday 19 December 2012
   Last Commit: $Date: 2012-12-19 11:27:58 -0600 (Wed, 19 Dec 2012) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.ScheduledTablesMonitor.h"
#include <assert.h>

namespace Cora
{
   namespace Device
   {
      namespace ScheduledTablesMonitorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         protected:
            ScheduledTablesMonitor *monitor;
            ScheduledTablesMonitorClient *client;
            friend class Cora::Device::ScheduledTablesMonitor;

         public:
            event_base(uint4 event_id,
                       ScheduledTablesMonitor *monitor_,
                       ScheduledTablesMonitorClient *client_):
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

            static void create_and_post(ScheduledTablesMonitor *monitor,
                                        ScheduledTablesMonitorClient *client);

            virtual void notify()
            { client->on_started(monitor); }
            
         private:
            event_started(ScheduledTablesMonitor *monitor,
                          ScheduledTablesMonitorClient *client):
               event_base(event_id,monitor,client)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Device::ScheduledTablesMonitor::event_started");


         void event_started::create_and_post(ScheduledTablesMonitor *monitor,
                                             ScheduledTablesMonitorClient *client)
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
            typedef ScheduledTablesMonitorClient::failure_type failure_type;
            failure_type failure;

            static void create_and_post(ScheduledTablesMonitor *monitor,
                                        ScheduledTablesMonitorClient *client,
                                        failure_type failure);

            virtual void notify()
            { client->on_failure(monitor,failure); }
            
         private:
            event_failure(ScheduledTablesMonitor *monitor,
                          ScheduledTablesMonitorClient *client,
                          failure_type failure_):
               event_base(event_id,monitor,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Device::ScheduledTablesMonitor::event_failure");


         void event_failure::create_and_post(ScheduledTablesMonitor *monitor,
                                             ScheduledTablesMonitorClient *client,
                                             failure_type failure)
         {
            try { (new event_failure(monitor,client,failure))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_table_added
         ////////////////////////////////////////////////////////////
         class event_table_added: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni table_name;
            bool scheduled;

            static void create_and_post(ScheduledTablesMonitor *monitor,
                                        ScheduledTablesMonitorClient *client,
                                        StrUni const &table_name,
                                        bool scheduled);

            virtual void notify()
            { client->on_table_added(monitor,table_name,scheduled); }
            
         private:
            event_table_added(ScheduledTablesMonitor *monitor,
                              ScheduledTablesMonitorClient *client,
                              StrUni const &table_name_,
                              bool scheduled_):
               event_base(event_id,monitor,client),
               table_name(table_name_),
               scheduled(scheduled_)
            { }
         };


         uint4 const event_table_added::event_id =
         Csi::Event::registerType("Cora::Device::ScheduledTablesMonitor::event_table_added");


         void event_table_added::create_and_post(ScheduledTablesMonitor *monitor,
                                                 ScheduledTablesMonitorClient *client,
                                                 StrUni const &table_name,
                                                 bool scheduled)
         {
            try { (new event_table_added(monitor,client,table_name,scheduled))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post
         

         ////////////////////////////////////////////////////////////
         // class event_table_deleted
         ////////////////////////////////////////////////////////////
         class event_table_deleted: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni table_name;

            static void create_and_post(ScheduledTablesMonitor *monitor,
                                        ScheduledTablesMonitorClient *client,
                                        StrUni const &table_name);

            virtual void notify()
            { client->on_table_deleted(monitor,table_name); }
            
         private:
            event_table_deleted(ScheduledTablesMonitor *monitor,
                                 ScheduledTablesMonitorClient *client,
                                 StrUni const &table_name_):
               event_base(event_id,monitor,client),
               table_name(table_name_)
            { }
         };


         uint4 const event_table_deleted::event_id =
         Csi::Event::registerType("Cora::Device::ScheduledTablesMonitor::event_table_deleted");


         void event_table_deleted::create_and_post(ScheduledTablesMonitor *monitor,
                                                    ScheduledTablesMonitorClient *client,
                                                    StrUni const &table_name)
         {
            try { (new event_table_deleted(monitor,client,table_name))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_table_scheduled
         ////////////////////////////////////////////////////////////
         class event_table_scheduled: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni table_name;
            bool scheduled;

            static void create_and_post(ScheduledTablesMonitor *monitor,
                                        ScheduledTablesMonitorClient *client,
                                        StrUni const &table_name,
                                        bool scheduled);

            virtual void notify()
            { client->on_table_scheduled(monitor,table_name,scheduled); }
            
         private:
            event_table_scheduled(ScheduledTablesMonitor *monitor,
                                  ScheduledTablesMonitorClient *client,
                                  StrUni const &table_name_,
                                  bool scheduled_):
               event_base(event_id,monitor,client),
               table_name(table_name_),
               scheduled(scheduled_)
            { }
         };


         uint4 const event_table_scheduled::event_id =
         Csi::Event::registerType("Cora::Device::ScheduledTablesMonitor::event_table_scheduled");


         void event_table_scheduled::create_and_post(ScheduledTablesMonitor *monitor,
                                                     ScheduledTablesMonitorClient *client,
                                                     StrUni const &table_name,
                                                     bool scheduled)
         {
            try { (new event_table_scheduled(monitor,client,table_name,scheduled))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post 
      };


      ////////////////////////////////////////////////////////////
      // class ScheduledTablesMonitor definitions
      ////////////////////////////////////////////////////////////

      ScheduledTablesMonitor::ScheduledTablesMonitor():
         state(state_standby),
         client(0),
         is_classic_logger(false),
         send_temporaries(true)
      {
         device_tables_lister.bind(new TablesEnumerator);
         broker_tables_lister.bind(new Broker::TableLister);
         settings.bind(new SettingsEnumerator);
      } // constructor


      ScheduledTablesMonitor::~ScheduledTablesMonitor()
      { finish(); }


      void ScheduledTablesMonitor::set_send_temporaries(bool send_temporaries_)
      {
         if(state == state_standby)
            send_temporaries = send_temporaries_;
         else
            throw exc_invalid_state();
      }


      void ScheduledTablesMonitor::start(
         ScheduledTablesMonitorClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(ScheduledTablesMonitorClient::is_valid_instance(client_))
            {
               input_location_identifiers.clear();
               is_classic_logger = false;
               client = client_;
               state = state_before_active;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void ScheduledTablesMonitor::start(
         ScheduledTablesMonitorClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(ScheduledTablesMonitorClient::is_valid_instance(client_))
            {
               input_location_identifiers.clear();
               is_classic_logger = false;
               client = client_;
               state = state_before_active;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start
      

      void ScheduledTablesMonitor::finish()
      {
         client = 0;
         state = state_standby;
         device_tables_lister->finish();
         broker_tables_lister->finish();
         tables.clear();
         settings->finish();
      } // finish

      
      void ScheduledTablesMonitor::on_devicebase_ready()
      {
         state = state_before_active;
         settings->set_device_name(get_device_name());
         settings->start(this,this);
         if(interface_version >= Csi::VersionNumber("1.2.1"))
         {
            device_tables_lister->set_device_name(get_device_name());
            device_tables_lister->start(this,this);
         }
      } // on_devicebase_ready

      
      void ScheduledTablesMonitor::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace ScheduledTablesMonitorHelpers;
         client_type::failure_type client_failure;
         
         switch(failure)
         {
         default:
            client_failure = client_type::failure_unknown;
            break;
            
         case devicebase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case devicebase_failure_session:
            client_failure = client_type::failure_session;
            break;
            
         case devicebase_failure_invalid_device_name:
            client_failure = client_type::failure_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case devicebase_failure_security:
            client_failure = client_type::failure_server_security;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_devicebase_failure

      
      void ScheduledTablesMonitor::on_devicebase_session_failure()
      {
         using namespace ScheduledTablesMonitorHelpers;
         event_failure::create_and_post(this,client,client_type::failure_session);
      } // on_devicebase_session_failure

      
      void ScheduledTablesMonitor::on_failure(SettingsEnumerator *settings,
                                              SettingsEnumeratorClient::failure_type failure)
      {
         using namespace ScheduledTablesMonitorHelpers;
         client_type::failure_type client_failure;

         switch(failure)
         {
         default:
            client_failure = client_type::failure_unknown;
            break;
            
         case SettingsEnumeratorClient::failure_connection_failed:
            client_failure = client_type::failure_session;
            break;
            
         case SettingsEnumeratorClient::failure_invalid_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case SettingsEnumeratorClient::failure_server_security_blocked:
            client_failure = client_type::failure_server_security;
            break;
            
         case SettingsEnumeratorClient::failure_device_name_invalid:
            client_failure = client_type::failure_invalid_device_name;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_failure

      
      void ScheduledTablesMonitor::on_setting_changed(
         SettingsEnumerator *settings,
         Csi::SharedPtr<Setting> &setting)
      {
         using namespace ScheduledTablesMonitorHelpers;
         if(setting->get_identifier() == Settings::input_location_labels)
         {
            input_location_identifiers = setting;
            is_classic_logger = true;
         }
         if(setting->get_identifier() == Settings::tables_to_exclude)
         {
            try
            {
               set_tables_to_exclude = setting;
               for(tables_type::iterator ti = tables.begin();
                   state == state_active && ti != tables.end();
                   ++ti)
               {
                  bool scheduled = !set_tables_to_exclude->has_name(ti->first);
                  if(scheduled != ti->second)
                     event_table_scheduled::create_and_post(this,client,ti->first,scheduled);
               }
            }
            catch(std::exception &)
            { assert(false); }

            // if in a pre-active state, we still need to get the list of tables
            if(state == state_before_active)
            {
               broker_tables_lister->set_open_broker_active_name(get_device_name());
               broker_tables_lister->set_send_temporaries(send_temporaries);
               broker_tables_lister->start(this,this);
            }
         }
      } // on_setting_changed

      
      void ScheduledTablesMonitor::on_started(TablesEnumerator *tran)
      {
         using namespace ScheduledTablesMonitorHelpers;
         state = state_active;
         event_started::create_and_post(this,client);
      } // on_started

      
      void ScheduledTablesMonitor::on_failure(TablesEnumerator *tran,
                                              TablesEnumeratorClient::failure_type failure)
      {
         using namespace ScheduledTablesMonitorHelpers;
         client_type::failure_type client_failure;

         switch(failure)
         {
         default:
            client_failure = client_type::failure_unknown;
            break;
            
         case TablesEnumeratorClient::failure_connection_failed:
            client_failure = client_type::failure_session;
            break;
            
         case TablesEnumeratorClient::failure_invalid_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case TablesEnumeratorClient::failure_server_security_blocked:
            client_failure = client_type::failure_server_security;
            break;
            
         case TablesEnumeratorClient::failure_device_name_invalid:
            client_failure = client_type::failure_invalid_device_name;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_failure

      
      void ScheduledTablesMonitor::on_table_added(TablesEnumerator *tran,
                                                  StrUni const &table_name,
                                                  bool enabled,
                                                  area_names_type const &area_names)
      {
         using namespace ScheduledTablesMonitorHelpers;
         tables[table_name] = enabled;
         event_table_added::create_and_post(this,client,table_name,enabled);
      } // on_table_added

      
      void ScheduledTablesMonitor::on_table_deleted(TablesEnumerator *tran,
                                                    StrUni const &table_name)
      {
         using namespace ScheduledTablesMonitorHelpers;
         event_table_deleted::create_and_post(this,client,table_name);
         tables.erase(table_name);
      } // on_table_deleted

      
      void ScheduledTablesMonitor::on_table_enabled(TablesEnumerator *tran,
                                                    StrUni const &table_name,
                                                    bool enabled)
      {
         using namespace ScheduledTablesMonitorHelpers;
         event_table_scheduled::create_and_post(this,client,table_name,enabled);
         tables[table_name] = enabled;
      } // on_table_enabled

      
      void ScheduledTablesMonitor::on_started(Broker::TableLister *lister)
      {
         using namespace ScheduledTablesMonitorHelpers;
         assert(state == state_before_active);
         state = state_active;
         event_started::create_and_post(this,client);
      } // on_started

      
      void ScheduledTablesMonitor::on_failure(Broker::TableLister *lister,
                                              Broker::TableListerClient::failure_type failure)
      {
         using namespace ScheduledTablesMonitorHelpers;
         client_type::failure_type client_failure;

         switch(failure)
         {
         default:
            client_failure = client_type::failure_unknown;
            break;

         case TableListerClient::failure_invalid_broker_id:
            client_failure = client_type::failure_invalid_device_name;
            break;
            
         case TableListerClient::failure_invalid_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case TableListerClient::failure_session_failed:
            client_failure = client_type::failure_session;
            break;
            
         case TableListerClient::failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case TableListerClient::failure_server_security_blocked:
            client_failure = client_type::failure_server_security;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_failure

      
      void ScheduledTablesMonitor::on_table_added(Broker::TableLister *lister,
                                                  StrUni const &table_name)
      {
         using namespace ScheduledTablesMonitorHelpers;
         assert(set_tables_to_exclude.get_rep() != 0);
         bool scheduled = !set_tables_to_exclude->has_name(table_name);
         event_table_added::create_and_post(this,client,table_name,scheduled);
         tables[table_name] = scheduled;
      } // on_table_added

      
      void ScheduledTablesMonitor::on_table_deleted(Broker::TableLister *lister,
                                                    StrUni const &table_name)
      {
         using namespace ScheduledTablesMonitorHelpers;
         event_table_deleted::create_and_post(this,client,table_name);
         tables.erase(table_name);
      } // on_table_deleted


      void ScheduledTablesMonitor::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace ScheduledTablesMonitorHelpers;
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         client_type *client(this->client);
         
         assert(event != 0);
         if(event->getType() == event_failure::event_id)
            finish();
         if(event->client == client &&
            ScheduledTablesMonitorClient::is_valid_instance(event->client))
            event->notify();
         else
            finish();
      } // receive
   };
};
