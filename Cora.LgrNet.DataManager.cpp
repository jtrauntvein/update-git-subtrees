/* Cora.LgrNet.DataManager.cpp

   Copyright (C) 2004, 2013 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: Monday 19 January 2004
   Last Change: Tuesday 04 June 2013
   Last Commit: $Date: 2014-08-06 09:48:48 -0600 (Wed, 06 Aug 2014) $
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header

#include "Cora.LgrNet.DataManager.h"
#include "Cora.LgrNet.BrokerBrowser2.h"
#include <sstream>
#include <time.h>
#include <stdlib.h>

#define RESTART_INTERVAL 10000

namespace Cora
{
   namespace LgrNet
   {
      namespace DataManagerHelpers
      {
         ////////////////////////////////////////////////////////////
         // class EventAdviseReady
         ////////////////////////////////////////////////////////////
         class EventAdviseReady:
            public Csi::Event
         {
         public:
            TableDataAdvisor *advisor;
            static uint4 const event_id;
            StrUni broker_name;
            StrUni table_name;
            typedef Csi::SharedPtr<Cora::Broker::Record> record_type;
            record_type record;
            DataManagerAdviseClient *client;

            static void create_and_post(
               TableDataAdvisor *advisor,
               DataManagerAdviseClient *client,
               StrUni const &broker_name,
               StrUni const &table_name,
               record_type &record)
            {
               try { (new EventAdviseReady(advisor,client,broker_name,table_name,record))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            virtual void notify()
            {
               if(DataManagerAdviseClient::is_valid_instance(client))
                  client->on_advise_ready(advisor,broker_name,table_name,record); 
            }

         private:
            EventAdviseReady(
               TableDataAdvisor *advisor_,
               DataManagerAdviseClient *client_,
               StrUni const &broker_name_,
               StrUni const &table_name_,
               record_type &record_):
               Event(event_id,advisor_),
               advisor(advisor_),
               client(client_),
               broker_name(broker_name_),
               table_name(table_name_),
               record(record_)
            { }
         };


         uint4 const EventAdviseReady::event_id =
         Csi::Event::registerType("Cora::LgrNet::DataManagerHelpers::EventAdviseReady");


         ////////////////////////////////////////////////////////////
         // class EventAdviseRecord
         ////////////////////////////////////////////////////////////
         class EventAdviseRecord:
            public Csi::Event
         {
         public:
            static uint4 const event_id;
            TableDataAdvisor *advisor;
            StrUni broker_name;
            StrUni table_name;
            typedef Csi::SharedPtr<Cora::Broker::Record> record_type;
            typedef std::deque<record_type> records_type;
            records_type records;
            DataManagerAdviseClient *client;

            static void create_and_post(
               TableDataAdvisor *advisor,
               DataManagerAdviseClient *client,
               StrUni const &broker_name,
               StrUni const &table_name,
               records_type &records)
            {
               try { (new EventAdviseRecord(advisor,client,broker_name,table_name,records))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            virtual void notify()
            {
               if(DataManagerAdviseClient::is_valid_instance(client))
                  client->on_advise_record(advisor,broker_name,table_name,records);
            }

         private:
            EventAdviseRecord(
               TableDataAdvisor *advisor_,
               DataManagerAdviseClient *client_,
               StrUni const &broker_name_,
               StrUni const &table_name_,
               records_type &records_):
               Event(event_id,advisor_),
               advisor(advisor_),
               client(client_),
               broker_name(broker_name_),
               table_name(table_name_),
               records(records_)
            { }
         };


         uint4 const EventAdviseRecord::event_id =
            Csi::Event::registerType("Cora::LgrNet::DataManagerHelpers::EventAdviseRecord");


         ////////////////////////////////////////////////////////////
         // class EventAdviseFailure
         ////////////////////////////////////////////////////////////
         class EventAdviseFailure:
            public Csi::Event
         {
         public:
            static uint4 const event_id;
            TableDataAdvisor *advisor;
            StrUni broker_name;
            StrUni table_name;
            DataManagerAdviseClient::failure_type failure;
            DataManagerAdviseClient *client;

            static void create_and_post(
               TableDataAdvisor *advisor,
               DataManagerAdviseClient *client,
               StrUni const &broker_name,
               StrUni const &table_name,
               DataManagerAdviseClient::failure_type failure)
            {
               try { (new EventAdviseFailure(advisor,client,broker_name,table_name,failure))->post(); }
               catch(Csi::Event::BadPost &) { }
            }


            virtual void notify()
            {
               if(DataManagerAdviseClient::is_valid_instance(client))
                  client->on_advise_failure(advisor,broker_name,table_name,failure); 
            }

         private:
            EventAdviseFailure(
               TableDataAdvisor *advisor_,
               DataManagerAdviseClient *client_,
               StrUni const &broker_name_,
               StrUni const &table_name_,
               DataManagerAdviseClient::failure_type failure_):
               Event(event_id,advisor_),
               advisor(advisor_),
               client(client_),
               broker_name(broker_name_),
               table_name(table_name_),
               failure(failure_)
            { }
         };


         uint4 const EventAdviseFailure::event_id =
         Csi::Event::registerType("Cora::LgrNet::DataManagerHelpers::EventAdviseFailure");
      };


      DataManager::DataManager():
         other_client(0),
         started(false)
      {
         oneshot.bind(new OneShot);
      }


      DataManager::DataManager(oneshot_handle &oneshot_):
         other_client(0),
         oneshot(oneshot_)
      { }


      DataManager::~DataManager()
      {
         finish();
      }

      
      void DataManager::set_value_factory(value_factory_type value_factory_)
      {
         if(started)
            throw ClientBase::exc_invalid_state();
         value_factory = value_factory_;
         brokers_type::iterator it = brokers.begin();
         while(it != brokers.end())
         {
            it->second->set_value_factory(value_factory);
            ++it;
         }
      } // set_value_factory


      StrUni DataManager::get_inlocs_table_name()
      { return BrokerBrowser2::classic_inlocs_table; }


      void DataManager::start(router_handle &router_)
      {
         if(started)
            throw ClientBase::exc_invalid_state();
         started = true;
         router = router_;
         brokers_type::iterator it = brokers.begin();
         while(it != brokers.end())
         {
            it->second->start(router);
            ++it;
         }
      }


      void DataManager::start(ClientBase *other_client_)
      {
         if(started)
            throw ClientBase::exc_invalid_state();
         started = true;
         other_client = other_client_;
         brokers_type::iterator it = brokers.begin();
         while(it != brokers.end())
         {
            it->second->start(other_client);
            ++it;
         }
      }

      
      void DataManager::finish()
      {
         brokers_type::iterator it = brokers.begin();
         while(it != brokers.end())
         {
            it->second->finish();
            ++it;
         }
         brokers.clear();
         other_client = 0;
         router.clear();
         started = false;
      }


      void DataManager::add_data_request(
         StrUni const &broker_name,
         StrUni const &table_name,
         StartInfo const &start_info,
         DataManagerAdviseClient *client,
         StrUni const &column_name)
      {
         if(broker_name != L"" &&
            table_name != L"")
         {
            if(DataManagerAdviseClient::is_valid_instance(client))
            {
               broker_type broker;
               brokers_type::iterator it = brokers.find(broker_name);
               if(it != brokers.end())
               {
                  broker = it->second;
               }
               else
               {
                  broker.bind(new BrokerDataManager(oneshot,broker_name));
                  broker->set_value_factory(value_factory);
                  brokers[broker_name] = broker;
                  if(other_client != 0)
                     broker->start(other_client);
                  else if(router != 0)
                     broker->start(router);
               }
               broker->add_data_request(table_name,start_info,client,column_name);
            }
            else
               throw std::invalid_argument("Invalid DataManagerAdviseClient @ DataManager::add_data_request()");
         }
         else
         {
            if(!broker_name.length())
               throw InvalidBrokerException();
            else
               throw InvalidTableException();
         }
      }

      
      void DataManager::remove_data_request(
         StrUni const &broker_name,
         StrUni const &table_name,
         StartInfo const &start_info,
         DataManagerAdviseClient *client,
         StrUni const &column_name)
      {
         if(DataManagerAdviseClient::is_valid_instance(client)&&
            broker_name != L"" &&
            table_name != L"")
         {
            brokers_type::iterator it = brokers.find(broker_name);
            if(it != brokers.end())
            {
               it->second->remove_data_request(table_name,start_info,client,column_name);
            }
         }
      }


      void DataManager::get_actual_inloc_table_name(
         StrUni const &broker_name,
         StrUni &table_name)
      {
         brokers_type::iterator it = brokers.find(broker_name);
         if(it != brokers.end())
         {
            it->second->get_actual_inloc_table_name(table_name);
         }
      }


      BrokerDataManager::BrokerDataManager(
         oneshot_handle &oneshot_,
         StrUni const &broker_name_):
         broker_name(broker_name_),
         other_client(0),
         oneshot(oneshot_)
      { }


      BrokerDataManager::~BrokerDataManager()
      {
         tables.clear();
      }


      void BrokerDataManager::set_value_factory(value_factory_type &value_factory_)
      {
         value_factory = value_factory_;
         tables_type::iterator it = tables.begin();
         while(it != tables.end())
         {
            it->second->set_value_factory(value_factory);
            ++it;
         }
      }


      void BrokerDataManager::start(router_handle &router_)
      {
         router = router_;
         tables_type::iterator it = tables.begin();
         while(it != tables.end())
         {
            it->second->start(router);
            ++it;
         }
      }


      void BrokerDataManager::start(ClientBase *other_client_)
      {
         other_client = other_client_;
         tables_type::iterator it = tables.begin();
         while(it != tables.end())
         {
            it->second->start(other_client);
            ++it;
         }
      }


      void BrokerDataManager::finish()
      {
         tables_type::iterator it = tables.begin();
         while(it != tables.end())
         {
            it->second->finish();
            ++it;
         }
      }


      void BrokerDataManager::add_data_request(
         StrUni const &table_name,
         DataManager::StartInfo const &start_info,
         DataManagerAdviseClient *client,
         StrUni const &column_name)
      {
         table_type table;
         tables_type::iterator it = tables.find(table_name);
         if(it != tables.end())
         {
            table = it->second;
         }
         else
         {
            table.bind(new TableDataManager(oneshot,broker_name,table_name));
            table->set_value_factory(value_factory);
            tables[table_name] = table;
            if(other_client != 0)
               table->start(other_client);
            else if(router != 0)
               table->start(router);
         }
         table->add_data_request(start_info,client,column_name);
      }


      void BrokerDataManager::remove_data_request(
         StrUni const &table_name,
         DataManager::StartInfo const &start_info,
         DataManagerAdviseClient *client,
         StrUni const &column_name)
      {
         tables_type::iterator it = tables.find(table_name);
         if(it != tables.end())
         {
            it->second->remove_data_request(start_info,client,column_name);
         }
      }


      void BrokerDataManager::get_actual_inloc_table_name(StrUni &table_name)
      {
         tables_type::iterator it = tables.find(DataManager::get_inlocs_table_name());
         if(it != tables.end())
         {
            table_name = it->second->get_table_name();
         }
      }


      TableDataManager::TableDataManager(
         oneshot_handle &oneshot_,
         StrUni const &broker_name_,
         StrUni const &table_name_):
         broker_name(broker_name_),
         table_name(table_name_),
         state(state_standby),
         other_client(0),
         is_classic_inloc_table(false),
         oneshot(oneshot_),
         maintainer_restart_id(0),
         settings_getter_restart_id(0),
         max_inlocs_per_request(62),
         auto_restart(true),
         auto_get_next_record_block(true)
      {
         if(table_name == DataManager::get_inlocs_table_name())
         {
            is_classic_inloc_table = true;
            table_name = "Inlocs";
            settings_getter.bind(new Cora::Device::SettingsEnumerator);
            settings_getter->set_device_name(broker_name);

            maintainer.bind(new Cora::Device::CollectArea::CollectAreaMaintainer(oneshot));
            maintainer->set_device_name(broker_name);
            maintainer->set_collect_area_name(table_name,true);
         }
      }


      TableDataManager::~TableDataManager()
      { }


      void TableDataManager::set_auto_restart(bool auto_restart_)
      {
         auto_restart = auto_restart_;
         table_advisors_type::iterator it = advisors.begin();
         while(it != advisors.end())
         {
            (*it)->set_auto_restart(auto_restart);
            ++it;
         }
         maintainer_restart_id = 0;
         settings_getter_restart_id = 0;
      }


      void TableDataManager::set_auto_get_next_record_block(bool auto_get_next_record_block_)
      {
         auto_get_next_record_block = auto_get_next_record_block_;
         table_advisors_type::iterator it = advisors.begin();
         while(it != advisors.end())
         {
            (*it)->set_auto_get_next_record_block(auto_get_next_record_block);
            ++it;
         }
      }


      void TableDataManager::set_value_factory(value_factory_type &value_factory_)
      {
         value_factory = value_factory_;
         table_advisors_type::iterator it = advisors.begin();
         while(it != advisors.end())
         {
            (*it)->set_value_factory(value_factory);
            ++it;
         }
      }


      void TableDataManager::start(router_handle &router_)
      {
         router = router_;
         if(is_classic_inloc_table)
         {
            restart_settings_getter();
         }
         else
         {
            table_advisors_type::iterator it = advisors.begin();
            while(it != advisors.end())
            {
               (*it)->start(router);
               ++it;
            }
         }
      }


      void TableDataManager::start(ClientBase *other_client_)
      {
         other_client = other_client_;
         if(is_classic_inloc_table)
         {
            restart_settings_getter();
         }
         else
         {
            table_advisors_type::iterator it = advisors.begin();
            while(it != advisors.end())
            {
               (*it)->start(other_client);
               ++it;
            }
         }
      }


      void TableDataManager::finish()
      {
         table_advisors_type::iterator it = advisors.begin();
         while(it != advisors.end())
         {
            (*it)->finish();
            ++it;
         }
      }


      void TableDataManager::on_started(
         Cora::Device::SettingsEnumerator *enumerator)
      {
         if(is_classic_inloc_table)
         {
            // Handle any pending adds here will prevent a lot of collect area setting
            // changes after the maintainer has already started.  Any pendings added after
            // this point will have no choice but to muck with the already running maintainer.
            inloc_ids_type::iterator inloc_it = inloc_ids.begin();
            while(inloc_it != inloc_ids.end() &&
                     (pending_tables.size() > 0 ||
                     pending_columns.size() > 0) &&
                     maintainer->get_field_count() < max_inlocs_per_request)
            {
               //Add all of the pending tables
               pending_tables_type::iterator table_it = pending_tables.begin();
               while(table_it != pending_tables.end())
               {
                  maintainer->add_field(inloc_it->first,inloc_it->second);
                  ++table_it;
               }
               
               //Add of the pending columns
               pending_columns_type::iterator column_it = pending_columns.begin();
               while(column_it != pending_columns.end())
               {
                  if(column_it->first == inloc_it->second)
                     maintainer->add_field(inloc_it->first,inloc_it->second);
                  ++column_it;
               }
               ++inloc_it;
            }

            pending_tables_type::iterator table_it = pending_tables.begin();
            while(table_it != pending_tables.end())
            {
               pending_clients.push_back(*table_it);
               ++table_it;
            }
            pending_columns_type::iterator column_it = pending_columns.begin();
            while(column_it != pending_columns.end())
            {
               pending_clients.push_back(column_it->second);
               ++column_it;
            }
            pending_tables.clear();
            pending_columns.clear();

            restart_maintainer();
         }
      }


      void TableDataManager::on_setting_changed(
         Cora::Device::SettingsEnumerator *enumerator,
         Csi::SharedPtr<Cora::Setting> &setting)
      {
         switch(setting->get_identifier())
         {
            case Cora::Device::Settings::input_location_labels:
            {
               inloc_ids.clear();
               Csi::PolySharedPtr<Cora::Setting, Cora::Device::SettingInlocIds> inlocs(setting);
               if(inlocs->size() > 0)
               {
                  Cora::Device::SettingInlocIds::iterator it = inlocs->begin();
                  while(it != inlocs->end())
                  {
                     inloc_ids[it->logger_id] = it->field_name;
                     ++it;
                  }
               }
               else //No associated DLD, so add InputLocation_1 - InputLocation_28
               {
                  for(int i = 1; i <= 28; i++)
                  {
                     std::ostringstream inloc;
                     inloc << "InputLocation_" << i;
                     inloc_ids[i] = inloc.str().c_str();
                  }
               }
               break;
            }
            case Cora::Device::Settings::max_inlocs_per_request:
            {
               Csi::PolySharedPtr<Cora::Setting,Cora::SettingUInt4> max_inlocs(setting);
               max_inlocs_per_request = max_inlocs->get_value();
               break;
            }
            default:
               break;
         }
      }


      void TableDataManager::on_failure(
         Cora::Device::SettingsEnumerator *enumerator,
         Cora::Device::SettingsEnumeratorClient::failure_type failure)
      {
         trace("TableDataManager::on_failure(Cora::Device::SettinsEnumerator)");
         if(auto_restart)
            settings_getter_restart_id = oneshot->arm(this,RESTART_INTERVAL);
      }


      void TableDataManager::on_started(
         Cora::Device::CollectArea::CollectAreaMaintainer *creator)
      {
         state = state_advise_started;
         inloc_ids_type::iterator inloc_it = inloc_ids.begin();
         while(inloc_it != inloc_ids.end() &&
                  (pending_tables.size() > 0 ||
                   pending_columns.size() > 0) &&
                   maintainer->get_field_count() < max_inlocs_per_request)
         {
            //Add all of the pending tables
            pending_tables_type::iterator table_it = pending_tables.begin();
            while(table_it != pending_tables.end())
            {
               maintainer->add_field(inloc_it->first,inloc_it->second);
               ++table_it;
            }
            
            //Add of the pending columns
            pending_columns_type::iterator column_it = pending_columns.begin();
            while(column_it != pending_columns.end())
            {
               if(column_it->first == inloc_it->second)
                  maintainer->add_field(inloc_it->first,inloc_it->second);
               ++column_it;
            }
            ++inloc_it;
         }
         pending_tables_type::iterator table_it = pending_tables.begin();
         while(table_it != pending_tables.end())
         {
            pending_clients.push_back(*table_it);
            ++table_it;
         }
         pending_columns_type::iterator column_it = pending_columns.begin();
         while(column_it != pending_columns.end())
         {
            pending_clients.push_back(column_it->second);
            ++column_it;
         }
         pending_tables.clear();
         pending_columns.clear();

         table_name = creator->get_collect_area_name();
         DataManager::StartInfo start_info;
         table_advisor_type table_advisor;
         bool found = false;
         table_advisors_type::iterator adv_it = advisors.begin();
         while(adv_it != advisors.end() && !found)
         {
            if((*adv_it)->get_table_name() == table_name )
            {
               found = true;
               table_advisor = *adv_it;
            }
            ++adv_it;
         }

         if(!found)
         {
            table_advisor.bind(new TableDataAdvisor(this,oneshot,broker_name,table_name,start_info,true));
            table_advisor->set_auto_restart(auto_restart);
            table_advisor->set_auto_get_next_record_block(auto_get_next_record_block);
            table_advisor->set_value_factory(value_factory);
            advisors.push_back(table_advisor);
         }

         //Add all of the clients to the new advisor
         pending_tables_type::iterator client_it = pending_clients.begin();
         while(client_it != pending_clients.end())
         {
            table_advisor->add_client(*client_it);
            ++client_it;
         }
         pending_clients.clear();
         if(other_client != 0)
            table_advisor->start(other_client);
         else if(router != 0)
            table_advisor->start(router);
      }


      void TableDataManager::on_failure(
         Cora::Device::CollectArea::CollectAreaMaintainer *creator,
         Cora::Device::CollectArea::CollectAreaMaintainerClient::failure_type failure)
      {
         trace("TableDataManager::on_failure(Cora::Device::CollectArea::CollectAreaMaintainer)");
         maintainer->finish();
         if(auto_restart)
            maintainer_restart_id = oneshot->arm(this,RESTART_INTERVAL);
      }


      void TableDataManager::add_data_request(
         DataManager::StartInfo const &start_info,
         DataManagerAdviseClient *client,
         StrUni const &column_name)
      {
         if(is_classic_inloc_table)
         {  //handle special case
            if(state == state_advise_started)
            {  //if advise is started, we can simply modify the maintainer and pass the client on to
               //the table advisor.
               inloc_ids_type::iterator inloc_it = inloc_ids.begin();
               while(inloc_it != inloc_ids.end())
               {
                  if(column_name == L"")
                  {  //add all the columns
                     bool add = true;
                     if(maintainer->get_field_count() >= max_inlocs_per_request &&
                        !maintainer->exists(inloc_it->first,inloc_it->second))
                        add = false;
                     if(add)
                        maintainer->add_field(inloc_it->first,inloc_it->second);
                     else 
                        break;
                  }
                  else if(inloc_it->second == column_name)
                  {  //only add this specific column
                     bool add = true;
                     if(maintainer->get_field_count() >= max_inlocs_per_request &&
                        !maintainer->exists(inloc_it->first,column_name))
                        add = false;
                     if(add)
                        maintainer->add_field(inloc_it->first,column_name);
                     break;
                  }
                  ++inloc_it;
               }
               table_advisors_type::iterator it = advisors.begin();
               if(it != advisors.end())
               {
                  (*it)->add_client(client);
               }
               else
               {
                  table_advisor_type table_advisor(new TableDataAdvisor(this,oneshot,broker_name,table_name,start_info,is_classic_inloc_table));
                  table_advisor->set_auto_restart(auto_restart);
                  table_advisor->set_auto_get_next_record_block(auto_get_next_record_block);
                  table_advisor->set_value_factory(value_factory);
                  table_advisor->add_client(client);
                  advisors.push_back(table_advisor);
                  if(other_client != 0)
                     table_advisor->start(other_client);
                  else if(router != 0)
                     table_advisor->start(router);
               }
            }
            else
            {  //Keep track of any attempted adds until we are in a steady state with
               //the collect area maintainer.
               if(column_name == L"")
               {
                  pending_tables.push_back(client);
               }
               else
               {
                  req_type request(column_name,client);
                  pending_columns.push_back(request);
               }
            }
         }
         else
         {
            table_advisors_type::iterator it = advisors.begin();
            bool found_match = false;
            while(found_match == false && it != advisors.end())
            {
               if(state == state_standby)
               {
                  if((*it)->get_advisor()->get_start_option() == start_info.start_option &&
                     (*it)->get_advisor()->get_order_option() == start_info.order_option &&
                     (*it)->get_advisor()->get_start_record_no() == start_info.record_no &&
                     (*it)->get_advisor()->get_start_file_mark_no() == start_info.file_mark_no &&
                     (*it)->get_advisor()->get_start_date() == start_info.start_date &&
                     (*it)->get_advisor()->get_start_interval() == start_info.start_interval &&
                     (*it)->get_advisor()->get_start_record_offset() == start_info.start_record_offset)
                  {
                     (*it)->add_client(client);
                     found_match = true;
                  }
               }
               else
               {
                  if(start_info.order_option == Cora::Broker::DataAdvisor::order_real_time)
                  {
                     if((*it)->get_advisor()->get_order_option() == Cora::Broker::DataAdvisor::order_real_time)
                     {
                        (*it)->add_client(client);
                        found_match = true;
                     }
                  }
               }
               ++it;
            }

            if(!found_match)
            {
               table_advisor_type table_advisor(new TableDataAdvisor(this,oneshot,broker_name,table_name,start_info,is_classic_inloc_table));
               table_advisor->set_auto_restart(auto_restart);
               table_advisor->set_auto_get_next_record_block(auto_get_next_record_block);
               table_advisor->set_value_factory(value_factory);
               table_advisor->add_client(client);
               advisors.push_back(table_advisor);
               if(other_client != 0)
                  table_advisor->start(other_client);
               else if(router != 0)
                  table_advisor->start(router);
            }
         }
      }


      void TableDataManager::remove_data_request(
         DataManager::StartInfo const &start_info,
         DataManagerAdviseClient *client,
         StrUni const &column_name)
      {
         if(is_classic_inloc_table)
         {
            if(state == state_advise_started)
            {  //if advise is started, we can simply modify the maintainer and pass the client on to
               //the table advisor.
               table_advisors_type::iterator it = advisors.begin();
               if(it != advisors.end())
               {
                  inloc_ids_type::iterator inloc_it = inloc_ids.begin();
                  while(inloc_it != inloc_ids.end())
                  {
                     if(column_name == L"")
                     {  //remove all the columns
                        maintainer->remove_field(inloc_it->first,inloc_it->second);
                     }
                     else if(inloc_it->second == column_name)
                     {  //remove add this specific column
                        maintainer->remove_field(inloc_it->first,column_name);
                        break;
                     }
                     ++inloc_it;
                  }
                  if((*it)->remove_client(client))
                     advisors.erase(it);
               }
            }
            else
            {  //Remove the client if a match is found pending
               if(column_name == L"")
               {
                  pending_tables.remove(client);
               }
               else
               {
                  pending_columns_type::iterator column_it = pending_columns.begin();
                  while(column_it != pending_columns.end())
                  {
                     if(column_it->second == client &&
                        column_it->first == column_name)
                     {
                        pending_columns_type::iterator del_it = column_it++;
                        pending_columns.erase(del_it);
                     }
                     else
                        ++column_it;
                  }
               }
            }
         }
         else
         {
            table_advisors_type::iterator it = advisors.begin();
            while(it != advisors.end())
            {
               if((*it)->remove_client(client))
               {
                  advisors.erase(it);
                  break;
               }
               ++it;
            }
         }
      }


      void TableDataManager::onOneShotFired(uint4 id)
      {
         if(id == settings_getter_restart_id)
         {
            settings_getter_restart_id = 0;
            restart_settings_getter();
         }
         else if(id == maintainer_restart_id && is_classic_inloc_table)
         {
            maintainer_restart_id = 0;
            restart_maintainer();
         }
      }


      void TableDataManager::restart_settings_getter()
      {
         try
         {
            if(settings_getter_restart_id != 0)
               settings_getter_restart_id = 0;
            settings_getter->finish();
            state = state_waiting_for_settings;
            if(other_client != 0)
               settings_getter->start(this,other_client);
            else if(router != 0)
               settings_getter->start(this,router);
         }
         catch(std::exception &)
         {
            if(auto_restart)
               settings_getter_restart_id = oneshot->arm(this,RESTART_INTERVAL);
         }
      }


      void TableDataManager::restart_maintainer()
      {
         if(is_classic_inloc_table)
         {
            try
            {
               if(maintainer_restart_id != 0)
                  maintainer_restart_id = 0;
               state = state_waiting_for_collect_area;
               if(other_client != 0)
                  maintainer->start(this,other_client);
               else if(router != 0)
                  maintainer->start(this,router);
            }
            catch(std::exception &)
            {
               if(auto_restart)
                  maintainer_restart_id = oneshot->arm(this,RESTART_INTERVAL);
            }
         }
      }


      TableDataAdvisor::TableDataAdvisor(
         TableDataManager *table_data_manager_,
         oneshot_handle &oneshot_,
         StrUni const &broker_name_,
         StrUni const &table_name_,
         DataManager::StartInfo const &start_info,
         bool is_classic_inloc_table_):
         table_data_manager(table_data_manager_),
         broker_name(broker_name_),
         table_name(table_name_),
         is_started(false),
         records_received(false),
         is_classic_inloc_table(is_classic_inloc_table_),
         other_client(0),
         oneshot(oneshot_),
         data_advise_restart_id(0),
         auto_restart(true),
         auto_get_next_record_block(true)
      {
         advisor.bind(new Cora::Broker::DataAdvisor);
         advisor->set_open_broker_active_name(broker_name);
         advisor->set_table_name(table_name);
         advisor->set_cache_size_controller(start_info.cache_size_controller);
         advisor->set_order_option(start_info.order_option);
         advisor->set_start_date(start_info.start_date);
         advisor->set_start_file_mark_no(start_info.file_mark_no);
         advisor->set_start_interval(start_info.start_interval);
         advisor->set_start_option(start_info.start_option);
         advisor->set_start_record_no(start_info.record_no);
         advisor->set_start_record_offset(start_info.start_record_offset);
      }


      TableDataAdvisor::~TableDataAdvisor()
      {
         advisor->finish();
      }


      void TableDataAdvisor::set_auto_restart(bool auto_restart_)
      {
         auto_restart = auto_restart_;
         if(auto_restart == false)
            data_advise_restart_id = 0;
      }


      void TableDataAdvisor::set_auto_get_next_record_block(bool auto_get_next_record_block_)
      {
         auto_get_next_record_block = auto_get_next_record_block_;
      }


      void TableDataAdvisor::set_value_factory(value_factory_type &value_factory)
      {
         if(value_factory != 0)
            advisor->set_value_factory(value_factory);
      }


      void TableDataAdvisor::start(router_handle &router_)
      {
         router = router_;
         restart();
      }


      void TableDataAdvisor::start(ClientBase *other_client_)
      {
         other_client = other_client_;
         restart();
      }


      void TableDataAdvisor::finish()
      {
         advisor->finish();
      }


      long TableDataAdvisor::add_client(DataManagerAdviseClient *client)
      {
         clients.insert(client);
         if(is_started)
         {
            try
            {
               Cora::Broker::DataAdvisor::record_handle record = advisor->get_record();
               if(last_record != 0)
                  record = last_record;
               if(record != 0)
               {
                  StrUni the_table = table_name;
                  if(is_classic_inloc_table)
                     the_table = DataManager::get_inlocs_table_name();
                  DataManagerHelpers::EventAdviseReady::create_and_post(
                     this,
                     client,
                     broker_name,
                     the_table,
                     record);
                  if(records_received)
                  {
                     DataManagerAdviseClient::records_type records;
                     records.push_back(record);
                     DataManagerHelpers::EventAdviseRecord::create_and_post(
                        this,
                        client,
                        broker_name,
                        the_table,
                        records);
                  }
               }
            }
            catch(std::exception &)
            { /*errors are to be expected here, just ignore*/ }
         }
         return static_cast<long>(clients.size());
      }


      bool TableDataAdvisor::remove_client(DataManagerAdviseClient *client)
      {
         bool rtn = false;
         clients_type::iterator it = clients.find(client);
         if(it != clients.end())
         {
            clients.erase(it);
         }
         if(clients.size() == 0)
            rtn = true;
         return rtn;
      }


      void TableDataAdvisor::on_advise_ready(
         Cora::Broker::DataAdvisor *tran)
      {
         StrUni the_table = table_name;
         record_handle record;
         clients_type::iterator it = clients.begin();
         
         if(is_classic_inloc_table)
            the_table = DataManager::get_inlocs_table_name();
         try
         { record = tran->get_record(); }
         catch(std::exception &)
         { record.clear(); }
         while(it != clients.end() && record != 0)
         {
            if(DataManagerAdviseClient::is_valid_instance(*it))
            {
               DataManagerHelpers::EventAdviseReady::create_and_post(
                  this,
                  *it,
                  broker_name,
                  the_table,
                  tran->get_record());
               ++it;
            }
            else
            {
               clients_type::iterator ci = it++;
               clients.erase(ci);
            }
         }
         is_started = true;
         records_received = false;
      } // on_advise_ready


      void TableDataAdvisor::on_advise_record(
         Cora::Broker::DataAdvisor *tran)
      {
         if(tran->get_unread_records().empty())
         {
            trace("tran->get_unread_records().empty()");
            return;
         }

         StrUni the_table = table_name;
         if(is_classic_inloc_table)
            the_table = DataManager::get_inlocs_table_name(); 
         clients_type::iterator it = clients.begin();
         records_received = true;
         while(it != clients.end())
         {
            if(DataManagerAdviseClient::is_valid_instance(*it))
            {
               DataManagerHelpers::EventAdviseRecord::create_and_post(
                  this,
                  *it,
                  broker_name,
                  the_table,
                  tran->get_unread_records());
               ++it;
            }
            else
            {
               clients_type::iterator ci = it++;
               clients.erase(ci);
            }
         }
         
         if(!tran->get_unread_records().empty())
         {
            try
            {
               if(last_record == 0)
               {
                  last_record.bind(
                     new Cora::Broker::Record(
                        *(tran->get_unread_records().rbegin()->get_rep())));
               }
               else
                  *last_record = *(tran->get_unread_records().rbegin()->get_rep());
            }
            catch(std::exception &)
            { }
         }
         
         if(auto_get_next_record_block)
            tran->get_next_block();
      }


      void TableDataAdvisor::on_advise_failure(
         Cora::Broker::DataAdvisor *tran,
         Cora::Broker::DataAdvisorClient::failure_type failure)
      {
         is_started = false;
         last_record.clear();
         StrUni the_table = table_name;
         if(is_classic_inloc_table)
            the_table = DataManager::get_inlocs_table_name();
         clients_type::iterator it = clients.begin();
         while(it != clients.end())
         {
            if(DataManagerAdviseClient::is_valid_instance(*it))
            {
               DataManagerHelpers::EventAdviseFailure::create_and_post(
                  this,
                  *it,
                  broker_name,
                  the_table,
                  static_cast<DataManagerAdviseClient::failure_type>(failure));
               ++it;
            }
            else
            {
               clients_type::iterator ci = it++;
               clients.erase(ci);
            }
         }
         if(auto_restart)
            data_advise_restart_id = oneshot->arm(this,RESTART_INTERVAL);
      }


      void TableDataAdvisor::restart()
      {
         try
         {
            if(data_advise_restart_id != 0)
               data_advise_restart_id = 0;
            advisor->finish();
            if(other_client != 0)
               advisor->start(this,other_client);
            else if(router != 0)
               advisor->start(this,router);
         }
         catch(std::exception &)
         {
            if(auto_restart)
               data_advise_restart_id = oneshot->arm(this,RESTART_INTERVAL);
         }
      }


      void TableDataAdvisor::onOneShotFired(uint4 id)
      {
         if(id == data_advise_restart_id)
         {
            data_advise_restart_id = 0;
            restart();
         }
      }


      void TableDataAdvisor::receive(Csi::SharedPtr<Csi::Event> &event)
      {
         if(event->getType() == DataManagerHelpers::EventAdviseReady::event_id)
         {
            (static_cast<DataManagerHelpers::EventAdviseReady*>(event.get_rep()))->notify();
         }
         else if(event->getType() == DataManagerHelpers::EventAdviseRecord::event_id)
         {
            (static_cast<DataManagerHelpers::EventAdviseRecord*>(event.get_rep()))->notify();
         }
         else if(event->getType() == DataManagerHelpers::EventAdviseFailure::event_id)
         {
            (static_cast<DataManagerHelpers::EventAdviseFailure*>(event.get_rep()))->notify();
         }
      }
   };
};
