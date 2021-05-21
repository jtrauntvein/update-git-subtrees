/* Cora.DataSources.LgrNetSymbols.cpp

   Copyright (C) 2008, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 August 2008
   Last Change: Friday 14 February 2020
   Last Commit: $Date: 2020-05-27 16:12:00 -0600 (Wed, 27 May 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.LgrNetSymbols.h"
#include "Cora.DataSources.LgrNetSource.h"
#include "Cora.DataSources.SymbolBrowser.h"
#include "Csi.SocketConnection.h"


namespace Cora
{
   namespace DataSources
   {
      namespace
      {
         /**
          * Defines an object that represents a value symbol in a table.
          */
         class ValueSymbol: public SymbolBase
         {
         public:
            typedef Cora::Broker::ColumnDesc desc_type;
            typedef Csi::SharedPtr<desc_type> desc_handle;
            ValueSymbol(
               LgrNetSource *source,
               StrUni const &name,
               SymbolBase *parent,
               desc_handle const &description_,
               uint4 offset_ = 0):
               SymbolBase(source, name, parent),
               description(description_),
               offset(offset_)
            { }

            virtual Cora::ClientBase *get_loggernet_component()
            { return parent->get_loggernet_component(); }

            virtual symbol_type_code get_symbol_type() const
            { return type_scalar; }

            virtual bool is_read_only() const
            { return description->get_modifying_command() == 0; }
            
            virtual bool can_expand() const
            { return false; }

            virtual void format_uri(std::ostream &out) const
            {
               // if the parent is an array, we need to skip over the parent symbol so we can
               // substitute our own name
               if(parent->get_symbol_type() == type_array)
               {
                  SymbolBase const *avo(parent->get_parent());
                  avo->format_uri(out);
                  out << "." << get_name();
               }
               else
                  SymbolBase::format_uri(out);
            }
            virtual void format_uri(std::wostream &out) const
            {
               // if the parent is an array, we need to skip over the parent symbol so we can
               // substitute our own name
               if(parent->get_symbol_type() == type_array)
               {
                  SymbolBase const *avo(parent->get_parent());
                  avo->format_uri(out);
                  out << L"." << get_name();
               }
               else
                  SymbolBase::format_uri(out);
            }
            
            virtual bool has_data_type() const
            { return true; }
            virtual CsiDbTypeCode get_data_type() const
            { return description->get_data_type(); }

            virtual bool has_units() const
            { return true; }
            virtual StrUni get_units() const
            { return description->get_units(); }

            virtual bool has_process() const
            { return true; }
            virtual StrUni get_process() const
            { return description->get_process(); }

            virtual bool has_description() const
            { return true; }
            virtual StrUni get_description() const
            { return description->get_description(); }

         private:
            desc_handle const description;
            uint4 offset;
         };

         
         /**
          * Defines a symbol object that represents an array of symbols.
          */
         class ArraySymbol: public SymbolBase
         {
         public:
            typedef Cora::Broker::ColumnDesc desc_type;
            typedef Csi::SharedPtr<desc_type> desc_handle;
            ArraySymbol(
               LgrNetSource      *source,
               StrAsc const      &name,
               SymbolBase        *parent,
               desc_handle const &description_):
               SymbolBase(source, name, parent),
               description(description_)
            {
               // we need to create value symbols for every value represented by this array
               Csi::OStrAscStream name_str;
               typedef std::vector<uint4> index_type;
               Csi::ArrayDimensions const &dimensions = description->get_dimensions();
               index_type index(dimensions.size());
               
               for(desc_type::const_iterator pi = description->begin();
                   pi != description->end();
                   ++pi)
               {
                  for(uint4 i = 0; i < pi->num_elements; ++i)
                  {
                     name_str.str("");
                     dimensions.to_index(index.begin(), pi->start_index + i);
                     name_str << name;
                     for(index_type::iterator ii = index.begin(); ii != index.end(); ++ii)
                     {
                        if(ii == index.begin())
                           name_str << "(";
                        else
                           name_str << ",";
                        name_str << (*ii);
                     }
                     name_str << ")";
                     children.push_back(
                        new ValueSymbol(
                           source, name_str.str(), this, description, pi->start_index + i));
                     children.back()->set_browser(get_browser());
                  }
               }
            }

            virtual symbol_type_code get_symbol_type() const
            { return type_array; }
            
            virtual Cora::ClientBase *get_loggernet_component()
            {
               // the broker browser owned by the source level component will be the loggernet
               // component.  We will rely on the parent objects chaining until the source is
               // reached. 
               return parent->get_loggernet_component();
            }

            virtual bool can_expand() const
            { return true; }

            virtual bool is_read_only() const
            { return description->get_modifying_command() == 0; } 

            virtual bool has_data_type() const
            { return true; }
            virtual CsiDbTypeCode get_data_type() const
            { return description->get_data_type(); }

            virtual bool has_units() const
            { return true; }
            virtual StrUni get_units() const
            { return description->get_units(); }

            virtual bool has_process() const
            { return true; }
            virtual StrUni get_process() const
            { return description->get_process(); }

            virtual bool has_description() const
            { return true; }
            virtual StrUni get_description() const
            { return description->get_description(); }

         private:
            desc_handle const description;
         };


         /**
          * Defines an symbol object that represents a table.
          */
         class TableSymbol:
            public SymbolBase,
            public Cora::LgrNet::BrokerBrowser2Client
         {
         public:
            typedef Cora::LgrNet::TableInfo table_info_type;
            typedef Csi::SharedPtr<table_info_type> table_info_handle;
            TableSymbol(
               SourceBase *source,
               StrAsc const &name,
               SymbolBase *parent,
               table_info_handle &table_info_):
               SymbolBase(source, name, parent),
               table_info(table_info_),
               expansion_started(false),
               table_scheduled(table_info_->table_scheduled)
            { }
            
            virtual ~TableSymbol()
            { table_info.clear(); }
            
            virtual Cora::ClientBase *get_loggernet_component()
            { return parent->get_loggernet_component(); }
            
            virtual symbol_type_code get_symbol_type() const
            { return type_table; }
            
            virtual bool can_expand() const
            { return true; }
            
            virtual void start_expansion()
            {
               if(!expansion_started)
               {
                  Cora::ClientBase *component = get_loggernet_component();
                  if(component)
                  {
                     expansion_started = true;
                     if(!table_info->get_all_started())
                        table_info->start(this, get_loggernet_component());
                  }
               }
            }

            virtual bool is_enabled() const
            { return table_scheduled; }

            void on_table_change()
            {
               if(SymbolBrowser::is_valid_instance<Csi::EventReceiver>(browser))
               {
                  if(table_scheduled != table_info->table_scheduled)
                  {
                     table_scheduled = table_info->table_scheduled;
                     browser->send_symbol_enabled_change(this);
                  }
               }
            }

            typedef Cora::LgrNet::BrokerBrowser2 browser_type;
            virtual void on_server_connect_started(
               browser_type *browser)
            { }

            virtual void on_server_connect_failed(
               browser_type *browser, browser_type::client_type::failure_type failure)
            { }

            virtual void on_brokers_changed()
            { }
            
            virtual void on_tables_changed(StrUni const &broker_name)
            { }

            virtual void on_table_defs_updated(
               StrUni const &broker_name, StrUni const &table_name)
            {
               if(SymbolBrowser::is_valid_instance<Csi::EventReceiver>(browser))
               {
                  using Cora::Broker::TableDesc;
                  TableDesc const &table_defs = *table_info->table_defs;
                  Csi::OStrAscStream column_name;
                  
                  while(!children.empty())
                  {
                     children_type::value_type child(children.front());
                     children.pop_front();
                     browser->send_symbol_removed(child, SymbolBrowserClient::remove_table_changed);
                  }
                  for(TableDesc::const_iterator ci = table_defs.begin(); ci != table_defs.end(); ++ci)
                  {
                     TableDesc::value_type const &column = *ci;
                     value_type symbol;
                     column_name.str("");
                     column_name << column->get_name();
                     column_name.str().replace(".", "\\.");
                     if(column->is_scalar())
                     {
                        symbol.bind(
                           new ValueSymbol(
                              static_cast<LgrNetSource *>(source), column_name.str(), this, column));
                     }
                     else
                     {
                        symbol.bind(
                           new ArraySymbol(
                              static_cast<LgrNetSource *>(source), column_name.str(), this, column));
                     }
                     if(symbol != 0)
                     {
                        symbol->set_browser(browser);
                        children.push_back(symbol);
                        browser->send_symbol_added(symbol);
                     }
                  }
                  browser->send_expansion_complete(this);
               }
            }
            
         private:
            table_info_handle table_info;
            bool expansion_started;
            bool table_scheduled;
         };


         /**
          * Defines a symbol object that represents a data broker.
          */
         class BrokerSymbol:
            public SymbolBase,
            public Cora::LgrNet::BrokerBrowser2Client
         {
         public:
            typedef Cora::LgrNet::BrokerInfo broker_info_type;
            typedef Csi::SharedPtr<broker_info_type> broker_info_handle;
            BrokerSymbol(
               LgrNetSource *source,
               StrAsc const &name,
               LgrNetSourceSymbol *parent,
               broker_info_handle broker_info_):
               SymbolBase(source, name, parent),
               broker_info(broker_info_),
               expansion_started(false)
            { }

            virtual ~BrokerSymbol()
            { broker_info.clear(); }

            virtual symbol_type_code get_symbol_type() const
            {
               symbol_type_code rtn = type_station;
               if(broker_info->broker_type != Cora::Broker::Type::active)
                  rtn = type_statistics_broker;
               return rtn;
            }
            
            virtual bool can_expand() const
            { return true; }

            virtual void start_expansion()
            {
               if(!expansion_started)
               {
                  Cora::ClientBase *component = get_loggernet_component();
                  if(component)
                  {
                     expansion_started = true;
                     broker_info->start(this, component);
                  }
               }
            }

            virtual Cora::ClientBase *get_loggernet_component()
            { return parent->get_loggernet_component(); }
            
            typedef Cora::LgrNet::BrokerBrowser2 broker_browser_type;
            virtual void on_server_connect_started(
               broker_browser_type *browser)
            { }

            virtual void on_server_connect_failed(
               broker_browser_type *browser,
               broker_browser_type::client_type::failure_type failure)
            { }

            virtual void on_brokers_changed()
            { }

            virtual void on_broker_started(StrUni const &broker_name)
            {
               browser->send_expansion_complete(this);
            }

            virtual void on_table_defs_updated(
               StrUni const &broker_name,
               StrUni const &table_name)
            { }

            virtual bool get_notify_specifics()
            { return true; }

            virtual void on_table_added(
               StrUni const &broker_name, StrUni const &table_name)
            {
               if(SymbolBrowser::is_valid_instance<Csi::EventReceiver>(browser))
               {
                  broker_info_type::iterator ti = broker_info->find(table_name);
                  if(ti != broker_info->end())
                  {
                     StrAsc table_name_mb(table_name.to_utf8());
                     value_type symbol;
                     table_name_mb.replace(".", "\\.");
                     symbol.bind(
                        new TableSymbol(
                           static_cast<LgrNetSource *>(source), table_name_mb, this, ti->second));
                     symbol->set_browser(browser);
                     children.insert(
                        std::upper_bound(
                           children.begin(), children.end(), symbol, symbol_name_less()),
                        symbol);
                     browser->send_symbol_added(symbol);
                  }
               }
            } // on_table_added

            virtual void on_table_deleted(
               StrUni const &broker_name, StrUni const &table_name_)
            {
               if(SymbolBrowser::is_valid_instance<Csi::EventReceiver>(browser))
               {
                  // we need to look up this symbol
                  StrAsc table_name(table_name_.to_utf8());
                  iterator si;
                  
                  si = std::find_if(begin(), end(), symbol_has_name(table_name));
                  if(si != end())
                  {
                     value_type symbol(*si);
                     children.erase(si);
                     browser->send_symbol_removed(symbol, SymbolBrowserClient::remove_table_deleted);
                  }
               }
            } // on_table_deleted

            virtual void on_tables_changed(
               StrUni const &broker_name)
            {
               for(children_type::iterator ci = children.begin(); ci != children.end(); ++ci)
               {
                  TableSymbol *symbol = static_cast<TableSymbol *>(ci->get_rep());
                  symbol->on_table_change();
               }
            }
            
         private:
            broker_info_handle broker_info;
            bool expansion_started;
         };
      };

      
      LgrNetSourceSymbol::LgrNetSourceSymbol(
         LgrNetSource *source_, StrUni const &name):
         SymbolBase(source_, name),
         source(source_),
         brokers_are_started(false),
         retry_id(0),
         retries_count(-1)
      { }

      
      LgrNetSourceSymbol::~LgrNetSourceSymbol()
      {
         brokers.clear();
      } // destructor


      Cora::ClientBase *LgrNetSourceSymbol::get_loggernet_component()
      {
         Cora::ClientBase *rtn = 0;
         if(source->is_connected())
            rtn = source->get_loggernet_component();
         else if(brokers_are_started)
            rtn = brokers->get_connection();
         return rtn;
      } // get_loggernet_component


      void LgrNetSourceSymbol::start_expansion()
      {
         if(brokers == 0 && retry_id == 0)
         {
            try
            {
               brokers.bind(new brokers_type);
               brokers->set_logon_name(source->get_logon_name());
               brokers->set_logon_password(source->get_logon_password());
               if(source->is_started())
                  brokers->start(this, source->get_loggernet_component());
               else
               {
                  Csi::SharedPtr<Csi::Messaging::Router> router(
                     new Csi::Messaging::Router(
                        new Csi::SocketConnection(
                           source->get_server_address().c_str(),
                           source->get_server_port()))); 
                  brokers->start(this, router);
               }
            }
            catch(std::exception &)
            {
               uint4 retry_interval(LgrNetSource::std_retry_interval);
               brokers.clear();
               if(++retries_count == 0)
                  retry_interval = 100;
               retry_id = source->get_timer()->arm(this, retry_interval);
            }
         }
      } // start_expansion


      bool LgrNetSourceSymbol::is_connected() const
      { return brokers_are_started; }

      
      void LgrNetSourceSymbol::on_server_connect_started(
         brokers_type *brokers)
      {
         if(SymbolBrowser::is_valid_instance<Csi::EventReceiver>(browser))
         {
            if(!brokers_are_started)
            {
               brokers_are_started = true;
               browser->send_symbol_connected_change(this);
            }
            browser->send_expansion_complete(this);
         }
      } // on_server_connect_started


      void LgrNetSourceSymbol::on_server_connect_failed(
         brokers_type *brokers_,
         brokers_type::client_type::failure_type failure)
      {
         if(SymbolBrowser::is_valid_instance<Csi::EventReceiver>(browser))
         {
            uint4 retry_interval(LgrNetSource::std_retry_interval);
            if(++retries_count == 0)
               retry_interval = 100;
            if(brokers_are_started)
            {
               brokers_are_started = false;
               browser->send_symbol_connected_change(this);
            }
            while(!children.empty())
            {
               browser->send_symbol_removed(children.front(), SymbolBrowserClient::remove_server_connection_lost);
               children.pop_front();
            }
            brokers.clear();
            retry_id = source->get_timer()->arm(this, retry_interval);
         }
      } // on_server_connect_failed


      void LgrNetSourceSymbol::on_broker_added(
         StrUni const &broker_name_)
      {
         if(SymbolBrowser::is_valid_instance<Csi::EventReceiver>(browser))
         {
            StrAsc broker_name;
            brokers_type::iterator bi = brokers->find(broker_name_);
            bool send_expansion_complete(!brokers_are_started);
            
            if(!brokers_are_started)
            {
               brokers_are_started = true;
               browser->send_symbol_connected_change(this);
            }
            if(bi != brokers->end())
            {
               broker_name = broker_name_.to_utf8();
               broker_name.replace(".", "\\.");
               value_type symbol(new BrokerSymbol(source, broker_name, this, bi->second));
               symbol->set_browser(browser);
               children.insert(
                  std::upper_bound(
                     children.begin(),
                     children.end(),
                     symbol,
                     symbol_name_less()),
                  symbol);
               browser->send_symbol_added(symbol);
            }
         }
      } // on_broker_added


      void LgrNetSourceSymbol::on_broker_deleted(
         StrUni const &broker_name_)
      {
         if(SymbolBrowser::is_valid_instance<Csi::EventReceiver>(browser))
         {
            StrAsc broker_name(broker_name_.to_utf8());
            broker_name.replace(".", "\\.");
            iterator ci(std::find_if(begin(), end(), symbol_has_name(broker_name)));
            if(ci != end())
            {
               value_type broker(*ci);
               children.erase(ci);
               browser->send_symbol_removed(broker, SymbolBrowserClient::remove_station_deleted);
            }
         }
      } // on_broker_deleted


      void LgrNetSourceSymbol::onOneShotFired(uint4 id)
      {
         if(!is_valid_instance(static_cast<OneShotClient*>(source)))
            return;

         if(id == retry_id)
         {
            retry_id = 0;
            start_expansion();
         }
      } // onOneShotFired
   };
};

