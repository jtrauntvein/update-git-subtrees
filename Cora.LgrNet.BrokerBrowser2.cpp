/* Cora.LgrNet.BrokerBrowser2.cpp

   Copyright (C) 2004, 2020 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: Monday 19 January 2004
   Last Change: Monday 24 February 2020
   Last Commit: $Date: 2020-02-24 17:09:29 -0600 (Mon, 24 Feb 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.BrokerBrowser2.h"
#include <sstream>


namespace Cora
{
   namespace LgrNet
   {
      const StrUni BrokerBrowser2::classic_inlocs_table = L"__inlocs__";

      BrokerBrowser2::BrokerBrowser2():
         client(0),
         all_started(false)
      { }


      BrokerBrowser2::~BrokerBrowser2()
      {
         broker_lister.clear();
      } // destructor

      
      void BrokerBrowser2::start(
         BrokerBrowser2Client *client_,
         router_handle &router)
      {
         all_started = false;
         if(BrokerBrowser2Client::is_valid_instance(client_))
         {
            client = client_;
            broker_lister.bind(new Cora::LgrNet::BrokerLister);
            broker_lister->set_application_name(application_name);
            broker_lister->set_logon_name(logon_name);
            broker_lister->set_logon_password(logon_password);
            broker_lister->start(this,router);
         }
         else
            throw std::invalid_argument("Invalid client pointer");
      } // start


      void BrokerBrowser2::start(
         BrokerBrowser2Client *client_,
         ClientBase *other_component)
      {
         all_started = false;
         if(BrokerBrowser2Client::is_valid_instance(client_))
         {
            client = client_;
            broker_lister.bind(new Cora::LgrNet::BrokerLister);
            broker_lister->start(this,other_component);
         }
         else
            throw std::invalid_argument("Invalid client pointer");
      } // start


      void BrokerBrowser2::on_started(BrokerLister *lister)
      {
         all_started = true;
         client->on_server_connect_started(this);
      } // on_started


      void BrokerBrowser2::on_failure(BrokerLister *lister, failure_type failure)
      {
         all_started = false;
         brokers.clear();
         client->on_server_connect_failed(this,static_cast<BrokerBrowser2Client::failure_type>(failure));
      } // on_failure


      void BrokerBrowser2::on_broker_added(
         BrokerLister *lister,
         StrUni const &broker_name,
         uint4 broker_id,
         broker_type_code type)
      {
         broker_info_type bi(new BrokerInfo);
         bi->broker_name = broker_name;
         bi->broker_id = broker_id;
         bi->broker_type = type;
         brokers[broker_name] = bi;
         if(client->get_notify_specifics())
            client->on_broker_added(broker_name);
         else if(all_started)
            client->on_brokers_changed();
      } // on_broker_added

      
      void BrokerBrowser2::on_broker_deleted(
         BrokerLister *lister,
         StrUni const &broker_name,
         uint4 broker_id,
         broker_type_code type)
      {
         brokers_type::iterator it = brokers.find(broker_name);
         if( it != brokers.end() )
         {
            brokers.erase(it);
            if(client->get_notify_specifics())
               client->on_broker_deleted(broker_name);
            else if(all_started)
               client->on_brokers_changed();
         }
      } // on_broker_deleted


      void BrokerBrowser2::on_broker_renamed(
         BrokerLister *lister,
         StrUni const &old_broker_name,
         StrUni const &new_broker_name,
         uint4 broker_id,
         broker_type_code type)
      {
         brokers_type::iterator it = brokers.find(old_broker_name);
         if( it != brokers.end() )
         {
            broker_info_type bi = it->second;
            bi->broker_id = broker_id;
            bi->broker_name = new_broker_name;
            bi->broker_type = type;
            brokers.erase(it);
            brokers[new_broker_name] = bi;
            if(client->get_notify_specifics())
            {
               client->on_broker_deleted(old_broker_name);
               client->on_broker_added(new_broker_name);
            }
            else if(all_started)
               client->on_brokers_changed();
         }
      } // on_broker_renamed


      BrokerInfo::BrokerInfo():
         client(0),
         all_started(false)
      { }


      BrokerInfo::~BrokerInfo()
      {
         tables.clear();
         device_table_lister.clear();
         other_table_lister.clear();
      } // destructor


      void BrokerInfo::start(
         BrokerBrowser2Client *client_,
         router_handle &router)
      {
         all_started = false;
         if(BrokerBrowser2Client::is_valid_instance(client_))
         {
            client = client_;
            if(broker_type == Cora::Broker::Type::active)
            {
               device_table_lister.bind(new Cora::Device::ScheduledTablesMonitor);
               device_table_lister->set_device_name(broker_name);
               device_table_lister->set_send_temporaries(false);
               device_table_lister->start(this,router);
            }
            else
            {
               other_table_lister.bind(new Cora::Broker::TableLister);
               other_table_lister->set_open_broker_active_name(broker_name);
               other_table_lister->set_send_temporaries(false);
               other_table_lister->start(this,router);
            }
         }
         else
            throw std::invalid_argument("Invalid client pointer");
      }


      void BrokerInfo::start(
         BrokerBrowser2Client *client_,
         ClientBase *other_component)
      {
         all_started = false;
         if(BrokerBrowser2Client::is_valid_instance(client_))
         {
            client = client_;
            if(broker_type == Cora::Broker::Type::active)
            {
               device_table_lister.bind(new Cora::Device::ScheduledTablesMonitor);
               device_table_lister->set_device_name(broker_name);
               device_table_lister->set_send_temporaries(false);
               device_table_lister->start(this,other_component);
            }
            else
            {
               other_table_lister.bind(new Cora::Broker::TableLister);
               other_table_lister->set_open_broker_active_name(broker_name);
               other_table_lister->set_send_temporaries(false);
               other_table_lister->start(this,other_component);
            }
         }
         else
            throw std::invalid_argument("Invalid client pointer");
      }


      void BrokerInfo::on_started(Cora::Broker::TableLister *lister)
      {
         if(client_type::is_valid_instance(client))
         {
            all_started = true;
            if(!client->get_notify_specifics())
               client->on_tables_changed(broker_name);
            else
               client->on_broker_started(broker_name);
         }
      } // on_started

      
      void BrokerInfo::on_failure(
         Cora::Broker::TableLister *lister,
         Cora::Broker::TableListerClient::failure_type failure)
      {
         if(client_type::is_valid_instance(client))
         {
            all_started = false;
            if(client->get_notify_specifics())
            {
               tables_type temp(tables);
               tables.clear();
               for(tables_type::iterator ti = temp.begin(); ti != temp.end(); ++ti)
               {
                  if(client_type::is_valid_instance(client))
                     client->on_table_deleted(broker_name, ti->first);
                  else
                     break;
               }
            }
            else
            {
               tables.clear();
               client->on_tables_changed(broker_name);
            }
         }
      } // on_failure


      void BrokerInfo::on_table_added(
         Cora::Broker::TableLister *lister,
         StrUni const &table_name)
      {
         if(client_type::is_valid_instance(client))
         {
            table_info_type ti(
               new TableInfo(
                  broker_name,
                  table_name));
            ti->table_scheduled = true;
            tables[table_name] = ti;
            if(client->get_notify_specifics())
               client->on_table_added(broker_name,table_name);
            else if(all_started)
               client->on_tables_changed(broker_name);
         }
      } // on_table_added


      void BrokerInfo::on_table_deleted(
         Cora::Broker::TableLister *lister,
         StrUni const &table_name)
      {
         if(client_type::is_valid_instance(client))
         {
            tables_type::iterator it = tables.find(table_name);
            if( it != tables.end() )
            {
               tables.erase(it);
               if(client->get_notify_specifics())
                  client->on_table_deleted(broker_name,table_name);
               else if(all_started)
                  client->on_tables_changed(broker_name);
            }
         }
      } // on_table_deleted


      void BrokerInfo::on_started(Cora::Device::ScheduledTablesMonitor *monitor)
      {
         if(client_type::is_valid_instance(client))
         {
            if(monitor->get_is_classic_logger())
            {
               table_info_type ti(
                  new TableInfo(
                     broker_name,
                     BrokerBrowser2::classic_inlocs_table,
                     monitor->get_input_location_identifiers()));
               tables[BrokerBrowser2::classic_inlocs_table] = ti;
               if(client->get_notify_specifics())
                  client->on_table_added(broker_name,BrokerBrowser2::classic_inlocs_table);
               else if(all_started)
                  client->on_tables_changed(broker_name);
            }

            if(!all_started)
            {
               all_started = true;
               if(client->get_notify_specifics())
                  client->on_broker_started(broker_name);
               else
                  client->on_tables_changed(broker_name);
            }
         }
      } // on_started
      

      void BrokerInfo::on_failure(
         Cora::Device::ScheduledTablesMonitor *monitor,
         Cora::Device::ScheduledTablesMonitorClient::failure_type failure)
      {
         if(client_type::is_valid_instance(client))
         {
            all_started = false;
            if(client->get_notify_specifics())
            {
               tables_type temp(tables);
               tables.clear();
               for(tables_type::iterator ti = temp.begin(); ti != temp.end(); ++ti)
               {
                  if(client_type::is_valid_instance(client))
                     client->on_table_deleted(broker_name, ti->first);
                  else
                     break;
               }
            }
            else
            {
               tables.clear();
               client->on_tables_changed(broker_name);
            }
         }
      } // on_failure


      void BrokerInfo::on_table_added(
         Cora::Device::ScheduledTablesMonitor *monitor,
         StrUni const &table_name,
         bool scheduled)
      {
         if(!client_type::is_valid_instance(client))
           return;
         if(monitor->get_is_classic_logger())
         {
            tables_type::iterator it = tables.find(BrokerBrowser2::classic_inlocs_table);
            if(it != tables.end())
            {
               if(client->get_notify_specifics())
               {
                  if(it->second->used_generic_inlocs)
                  {
                     it->second->used_generic_inlocs = false;
                     client->on_table_deleted(broker_name,BrokerBrowser2::classic_inlocs_table);
                     client->on_table_added(broker_name,BrokerBrowser2::classic_inlocs_table);
                  }
               }
            }
         }
         else if(client->get_notify_specifics())
         {
            tables_type::iterator ti = tables.find(table_name);
            if(ti != tables.end())
               client->on_table_deleted(broker_name, table_name);
         }

         table_info_type ti(
            new TableInfo(
               broker_name,
               table_name));
         ti->table_scheduled = scheduled;
         tables[table_name] = ti;
         if(client->get_notify_specifics())
         {
            client->on_table_added(broker_name,table_name);
         }
         else
         {
            if(all_started)
               client->on_tables_changed(broker_name);
         }
      }


      void BrokerInfo::on_table_deleted(
         Cora::Device::ScheduledTablesMonitor *monitor,
         StrUni const &table_name)
      {
         if(!client_type::is_valid_instance(client))
            return;
         tables_type::iterator it = tables.find(table_name);
         if( it != tables.end() )
         {
            tables.erase(it);
            if(client->get_notify_specifics())
            {
               client->on_table_deleted(broker_name,table_name);
            }
            else
            {
               if(all_started)
                  client->on_tables_changed(broker_name);
            }
         }
      }
      

      void BrokerInfo::on_table_scheduled(
         Cora::Device::ScheduledTablesMonitor *monitor,
         StrUni const &table_name,
         bool scheduled)
      {
         if(!client_type::is_valid_instance(client))
           return;
         if(client->get_notify_specifics())
         {
            tables_type::iterator it = tables.find(table_name);
            if( it != tables.end() )
            {
               table_info_type ti = it->second;
               ti->table_name = table_name;
               ti->broker_name = broker_name;
               ti->table_scheduled = scheduled;
               if(all_started)
                  client->on_tables_changed(broker_name);
            }
         }
      }


      uint4 input_location_labels_ready_event = Csi::Event::registerType("Cora::LgrNet::TableInfo::InputLocationsReady");


      TableInfo::TableInfo(
         StrUni const &broker_name_,
         StrUni const &table_name_,
         input_location_identifiers_type input_location_identifiers_ ):
         broker_name(broker_name_),
         table_name(table_name_),
         input_location_identifiers(input_location_identifiers_),
         client(0),
         all_started(false),
         used_generic_inlocs(false)
      {
         if(input_location_identifiers != 0)
         {
            table_scheduled = true;
            table_defs.bind(new Cora::Broker::TableDesc(table_name));
            table_defs->set_interval(1);
            table_defs->set_num_records(1);
            table_defs->set_original_num_records(1);
            if(input_location_identifiers->size() == 0)
            { //Create 24 generic inlocs
               used_generic_inlocs = true;
               for( int i = 1; i <= 28; i++ )
               {
                  std::ostringstream field_name;
                  field_name << "InputLocation_" << i;
                  Csi::SharedPtr<Cora::Broker::ColumnDesc> column(new Cora::Broker::ColumnDesc);
                  column->set_name(field_name.str().c_str());
                  column->set_data_type(CsiFp4);
                  column->set_modifying_command(276);
                  table_defs->add_column_desc(column);
               }
            }
            else
            {
               std::list<Cora::Device::SettingInlocIds::record_type>::const_iterator it = input_location_identifiers->begin();
               while(it != input_location_identifiers->end())
               {
                  Csi::SharedPtr<Cora::Broker::ColumnDesc> column(new Cora::Broker::ColumnDesc);
                  column->set_name(it->field_name);
                  column->set_data_type(CsiFp4);
                  column->set_modifying_command(276);
                  table_defs->add_column_desc(column);
                  ++it;
               }
            }
         }
      }


      TableInfo::~TableInfo()
      {
         table_defs_getter.clear();
      }


      void TableInfo::start(
         BrokerBrowser2Client *client_,
         router_handle &router)
      {
         all_started = false;
         if(BrokerBrowser2Client::is_valid_instance(client_))
         {
            client = client_;
            if(table_name != BrokerBrowser2::classic_inlocs_table)
            {
               table_defs_getter.bind(new Cora::Broker::TableDefsGetter);
               table_defs_getter->set_open_broker_active_name(broker_name);
               table_defs_getter->set_table_name(table_name);
               table_defs_getter->start(this,router);
            }
            else
            {
               Csi::Event *ev = Csi::Event::create(input_location_labels_ready_event,this);
               ev->post();
            }
         }
         else
            throw std::invalid_argument("Invalid client pointer");
    }
      
      
      void TableInfo::start(
         BrokerBrowser2Client *client_,
         ClientBase *other_component)
      {
         all_started = false;
         if(BrokerBrowser2Client::is_valid_instance(client_))
         {
            client = client_;
            if(table_name != BrokerBrowser2::classic_inlocs_table)
            {
               table_defs_getter.bind(new Cora::Broker::TableDefsGetter);
               table_defs_getter->set_open_broker_active_name(broker_name);
               table_defs_getter->set_table_name(table_name);
               table_defs_getter->start(this,other_component);
            }
            else
            {
               Csi::Event *ev = Csi::Event::create(input_location_labels_ready_event,this);
               ev->post();
            }
         }
         else
            throw std::invalid_argument("Invalid client pointer");
      }


      void TableInfo::on_complete(
         Cora::Broker::TableDefsGetter *getter,
         Cora::Broker::TableDefsGetterClient::outcome_type outcome,
         Cora::Broker::TableDefsGetterClient::table_defs_type &table_defs_)
      {
         if(client->get_notify_specifics())
         {
            if(outcome == Cora::Broker::TableDefsGetterClient::outcome_success)
            {
               table_defs = table_defs_;
               all_started = true;
               client->on_table_defs_updated(broker_name,table_name);
            }
            else
            {
               all_started = false;
               client->on_table_deleted(broker_name,table_name);
               table_defs.clear();
            }
         }
         else
         {
            if(outcome == Cora::Broker::TableDefsGetterClient::outcome_success)
            {
               table_defs = table_defs_;
               all_started = true;
            }
            else
            {
               table_defs.clear();
               all_started = false;
            }
            client->on_table_defs_updated(broker_name,table_name);
         }
      }


      void TableInfo::receive(Csi::SharedPtr<Csi::Event> &event)
      {
         if(event->getType() == input_location_labels_ready_event)
         {
            all_started = true;
            client->on_table_defs_updated(broker_name,table_name);
         }
      }
   };
};
