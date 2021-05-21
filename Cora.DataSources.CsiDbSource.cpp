/* Cora.DataSources.CsiDbSource.cpp

   Copyright (C) 2009, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 06 February 2009
   Last Change: Thursday 19 January 2017
   Last Commit: $Date: 2020-07-14 15:41:32 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.CsiDbSource.h"
#include "Cora.DataSources.CsiDbHelpers.Wart.h"
#include "Cora.DataSources.Manager.h"


namespace Cora
{
   namespace DataSources
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // std_retry_interval
         ////////////////////////////////////////////////////////////
         uint4 const std_retry_interval(10000);

         
         ////////////////////////////////////////////////////////////
         // class Uri
         ////////////////////////////////////////////////////////////
         class Uri
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            Uri(StrUni const &uri)
            { parse(uri); }

            ////////////////////////////////////////////////////////////
            // parse
            ////////////////////////////////////////////////////////////
            void parse(StrUni const &uri);

            ////////////////////////////////////////////////////////////
            // get_source_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_source_name() const
            { return source_name; }

            ////////////////////////////////////////////////////////////
            // get_table_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_table_name() const
            { return table_name; }

            ////////////////////////////////////////////////////////////
            // get_column_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_column_name() const
            { return column_name; }

         private:
            ////////////////////////////////////////////////////////////
            // source_name
            ////////////////////////////////////////////////////////////
            StrUni source_name;

            ////////////////////////////////////////////////////////////
            // table_name
            ////////////////////////////////////////////////////////////
            StrUni table_name;

            ////////////////////////////////////////////////////////////
            // column_name
            ////////////////////////////////////////////////////////////
            StrUni column_name;
         };


         void Uri::parse(StrUni const &uri)
         {
            enum state_type
            {
               state_before_source,
               state_in_source,
               state_in_table,
               state_in_table_quoted,
               state_in_field,
               state_in_field_quoted,
               state_complete
            } state = state_before_source;

            source_name.cut(0);
            table_name.cut(0);
            column_name.cut(0);
            for(size_t i = 0; state != state_complete && i < uri.length(); ++i)
            {
               wchar_t ch = uri[i];
               switch(state)
               {
               case state_before_source:
                  if(!isspace(ch))
                  {
                     if(ch != '\"')
                     {
                        source_name.append(ch);
                        state = state_in_source;
                     }
                  }
                  break;
                  
               case state_in_source:
                  if(ch == ':')
                     state = state_in_table;
                  else if(ch != '\"')
                     source_name.append(ch);
                  else
                     state = state_complete;
                  break;
                     
               case state_in_table:
                  if(ch == '.')
                     state = state_in_field;
                  else if(ch == '\\')
                     state = state_in_table_quoted;
                  else if(ch != '\"')
                     table_name.append(ch);
                  else
                     state = state_complete;
                  break;
                  
               case state_in_table_quoted:
                  if(ch == '.')
                     table_name.append(ch);
                  else
                  {
                     table_name.append('\\');
                     table_name.append(ch);
                  }
                  state = state_in_table;
                  break;
                  
               case state_in_field:
                  if(ch == '\\')
                     state = state_in_field_quoted;
                  else if(ch != '\"')
                     column_name.append(ch);
                  else
                     state = state_complete;
                  break;
                     
               case state_in_field_quoted:
                  if(ch == '.')
                     column_name.append(ch);
                  else
                  {
                     column_name.append('\\');
                     column_name.append(ch);
                  }
                  state = state_in_field;
                  break;
               }
            }
         } // parse
      };

      
      ////////////////////////////////////////////////////////////
      // class CsiDbSource definitions
      ////////////////////////////////////////////////////////////
      int CsiDbSource::connect_count = 0;
      StrUni const CsiDbSource::db_type_name(L"db-type");
      StrUni const CsiDbSource::db_use_windows_authentication_name(L"db-use-windows-authentication");
      StrUni const CsiDbSource::db_save_user_id_name(L"db-save-user-id");
      StrUni const CsiDbSource::db_user_id_name(L"db-user-id");
      StrUni const CsiDbSource::db_password_name(L"db-password");
      StrUni const CsiDbSource::db_data_source_name(L"db-data-source");
      StrUni const CsiDbSource::db_initial_catalog_name(L"db-initial-catalog");
      StrUni const CsiDbSource::poll_interval_name(L"poll-interval");
      StrUni const CsiDbSource::should_poll_meta_name(L"should-poll-meta");
      Csi::SharedPtr<CsiDbHelpers::MyThread> CsiDbSource::db_thread;
      
      
      CsiDbSource::CsiDbSource(StrUni const &name):
         SourceBase(name),
         connection_pending(false),
         was_connected(false),
         retry_id(0),
         poll_schedule(0),
         poll_schedule_enabled(true)
      { }

      
      CsiDbSource::~CsiDbSource()
      {
         if(scheduler != 0)
         {
            if(poll_schedule != 0)
               scheduler->cancel(poll_schedule);
            scheduler.clear();
         }
         if(symbol != 0)
            symbol->set_source(0);
         symbol.clear();
         disconnect();
      } // destructor

      
      void CsiDbSource::connect()
      {
         // if the connection already exists, we will ignore this call
         if(db_connection == 0 && !connection_pending)
         {
            was_connected = true;
            try
            {
               // we need to ensure that the database thread is started
               if(connect_count <= 0)
                  db_thread.bind(new CsiDbHelpers::MyThread);
               ++connect_count;

               // we now need to post a command to the thread to request a connection
               db_thread->add_command(new CsiDbHelpers::ConnectCommand(this, properties));
            }
            catch(std::exception &)
            {
               connect_count = 0;
               db_thread.clear();
               retry_id = timer->arm(this, std_retry_interval);
            }
         }
      } // connect

      
      void CsiDbSource::stop()
      {
         SourceBase::stop();
         if(retry_id != 0)
         {
            timer->disarm(retry_id);
            retry_id = 0;
         }
      } // stop


      void CsiDbSource::disconnect()
      {
         if(poll_schedule != 0)
         {
            if(scheduler != 0)
               scheduler->cancel(poll_schedule);
            poll_schedule = 0;
         }
         if(db_connection != 0)
         {
            CsiDbHelpers::MyThread::command_handle command(
               new CsiDbHelpers::DisconnectCommand(
                  this, db_connection));
            db_connection.clear();
            db_thread->add_command(command);
            if(--connect_count <= 0)
               db_thread.clear();
         }
      } // disconnect

      
      bool CsiDbSource::is_connected() const
      {
         bool rtn = false;
         if(db_connection != 0)
            rtn = true;
         return rtn;
      } // is_connected


      void CsiDbSource::get_properties(Csi::Xml::Element &prop_xml)
      { properties.get_properties(prop_xml); }

      
      void CsiDbSource::set_properties(Csi::Xml::Element &prop_xml)
      { properties.set_properties(prop_xml); }


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate can_add_request
         ////////////////////////////////////////////////////////////
         struct can_add_request
         {
            CsiDbSource::request_handle &request;
            can_add_request(CsiDbSource::request_handle &request_):
               request(request_)
            { }

            bool operator ()(CsiDbSource::cursor_handle &cursor)
            { return cursor->add_request(request); }
         };
      };
      
      
      void CsiDbSource::add_request(request_handle &request, bool more_to_follow)
      {
         if(SinkBase::is_valid_instance(request->get_sink()) && db_connection != 0)
         {
            // it's most efficient to break down the request symbols here.  This way, it can be done
            // only once for the request and we can choose (or create) the appropriate cursor more
            // efficiently. 
            symbols_type symbols;
            StrUni table_name, column_name;
            
            breakdown_uri(symbols, request->get_uri());
            if(symbols.empty())
               throw std::invalid_argument("invalid uri syntax");
            switch(symbols.back().second)
            {
            case SymbolBase::type_table:
               table_name = symbols.back().first;
               break;
               
            case SymbolBase::type_scalar:
               column_name = symbols.back().first;
               symbols.pop_back();
               table_name = symbols.back().first;
               break;
               
            default:
               throw std::invalid_argument("invalid uri syntax");
               break; 
            }
            request->set_wart(new CsiDbHelpers::Wart(table_name.to_utf8(), column_name.to_utf8()));
            
            // we now need to search for a cursor that can accept the symbol
            cursors_type::iterator ci = std::find_if(
               cursors.begin(), cursors.end(), can_add_request(request));
            if(ci == cursors.end())
            {
               cursor_handle cursor(new CsiDbHelpers::Cursor(this, request));
               cursors.push_back(cursor);
               if(!more_to_follow)
                  cursor->start();
            }
         }
      } // add_request

      
      void CsiDbSource::remove_request(request_handle &request)
      {
         for(cursors_type::iterator ci = cursors.begin(); ci != cursors.end(); ++ci)
         {
            cursors_type::value_type cursor(*ci);
            if(cursor->remove_request(request))
            {
               cursors.erase(ci);
               break;
            }
         }
      } // remove_request


      void CsiDbSource::remove_all_requests()
      {
         cursors.clear();
      } // remove_all_requests


      void CsiDbSource::activate_requests()
      {
         for(cursors_type::iterator ci = cursors.begin(); ci != cursors.end(); ++ci)
         {
            cursors_type::value_type &cursor = *ci;
            cursor->start();
         }

         if(is_started() && poll_schedule == 0)
            poll_schedule = scheduler->start(this, 0, properties.get_poll_interval(), true);
      } // activate_requests

      
      CsiDbSource::symbol_handle CsiDbSource::get_source_symbol()
      {
         if(symbol == 0)
            symbol.bind(new CsiDbSymbol(this));
         return symbol.get_handle();
      } // get_source_symbol


      void CsiDbSource::breakdown_uri(symbols_type &symbols, StrUni const &uri_)
      {
         Uri uri(uri_);
         symbols.clear();
         if(uri.get_source_name().length() > 0)
         {
            symbols.push_back(symbol_type(uri.get_source_name(), SymbolBase::type_db_source));
            if(uri.get_table_name().length() > 0)
            {
               symbols.push_back(symbol_type(uri.get_table_name(), SymbolBase::type_table));
               if(uri.get_column_name().length() > 0)
                  symbols.push_back(symbol_type(uri.get_column_name(), SymbolBase::type_scalar));
            }
         }
      } // breakdown_uri


      void CsiDbSource::set_manager(Manager *manager)
      {
         SourceBase::set_manager(manager);
         if(manager != 0)
         {
            timer = manager->get_timer();
            scheduler.bind(new Scheduler(timer));
         }
      } // set_manager


      void CsiDbSource::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace CsiDbHelpers;
         if(ev->getType() == CommandCompleteEvent::event_id)
         {
            CommandCompleteEvent *event = static_cast<CommandCompleteEvent *>(ev.get_rep());
            if(event->command->command_type == ConnectCommand::command_id)
               on_connect_complete(static_cast<ConnectCommand *>(event->command.get_rep()));
            
         }
         else if(ev->getType() == event_connect_failure::event_id)
         {
            cursors.clear();
            db_connection.clear(); 
            retry_id = timer->arm(this, std_retry_interval);
            manager->report_source_disconnect(this, ManagerClient::disconnect_connection_failed);
         }
      } // receive


      void CsiDbSource::onOneShotFired(uint4 id)
      {
         if(id == retry_id)
         {
            retry_id = 0;
            if(was_connected && !connection_pending)
               connect();
         }
      } // onOneShotFired


      void CsiDbSource::onScheduledEvent(uint4 id)
      {
         if(id == poll_schedule && poll_schedule_enabled)
         {
            for(cursors_type::iterator ci = cursors.begin(); ci != cursors.end(); ++ci)
            {
               cursors_type::value_type &cursor = *ci;
               cursor->poll();
            }
         }
      } // onScheduledEvent


      void CsiDbSource::list_sources(source_names_type &source_names, int source_type)
      {
         // we need to start the thread if it isn't already going
         if(connect_count <= 0)
            db_thread.bind(new CsiDbHelpers::MyThread);
         ++connect_count;

         // we can now call the DLL method to get the sources
         using namespace CsiDbHelpers;
         Csi::PolySharedPtr<CommandBase, ListDataSourcesCommand> command(
            new ListDataSourcesCommand(source_type)); 
         source_names.clear();
         db_thread->add_command(command.get_handle());
         command->condition.wait();
         source_names = command->sources;

         // we will now decrement the connect count and stop the thread if needed
         --connect_count;
         if(connect_count <= 0)
         {
            connect_count = 0;
            db_thread.clear();
         }
      } // list_sources


      void CsiDbSource::list_databases(
         databases_type &databases, Csi::Xml::Element &props_xml)
      {
         // we need to start the thread if it isn't already going
         if(connect_count <= 0)
            db_thread.bind(new CsiDbHelpers::MyThread);
         ++connect_count;

         // we can now call the DLL method to get the sources
         using namespace CsiDbHelpers;
         properties_type properties;
         properties.set_properties(props_xml);
         Csi::PolySharedPtr<CommandBase, ListDatabasesCommand> command(
            new ListDatabasesCommand(properties));
         databases.clear();
         db_thread->add_command(command.get_handle());
         command->condition.wait();
         databases = command->databases;

         // we will now decrement the connect count and stop the thread if needed
         --connect_count;
         if(connect_count <= 0)
         {
            connect_count = 0;
            db_thread.clear();
         }

         // we can now test the results of the command. 
         if(command->last_error.length())
            throw Csi::MsgExcept(command->last_error.c_str());
      } // list_databases


      void CsiDbSource::test_connection(Csi::Xml::Element &props_xml)
      {
         // we need to start the thread if it isn't already going
         if(connect_count <= 0)
            db_thread.bind(new CsiDbHelpers::MyThread);
         ++connect_count;

         // we will use the list databases command to test whether the source is viable.
         using namespace CsiDbHelpers;
         properties_type properties;
         properties.set_properties(props_xml);
         Csi::PolySharedPtr<CommandBase, ListDatabasesCommand> command(
            new ListDatabasesCommand(properties));
         db_thread->add_command(command.get_handle());
         command->condition.wait();

         // we need to ensure that the connection we made will get cleared
         --connect_count;
         if(connect_count <= 0)
         {
            connect_count = 0;
            db_thread.clear();
         }

         // finally, we can test the results from the command
         if(command->last_error.length())
            throw Csi::MsgExcept(command->last_error.c_str());
      } // test_connection
      

      void CsiDbSource::get_table_range(
         Csi::EventReceiver *client, StrUni const &uri)
      {
         if(db_thread != 0)
         {
            Uri parser(uri);
            db_thread->add_command(
               new CsiDbHelpers::GetTableRangeCommand(
                  client, uri.to_utf8(), parser.get_table_name().to_utf8(), db_connection));
         }
         else
         {
            GetTableRangeCompleteEvent::cpost(
               client, uri, GetTableRangeCompleteEvent::outcome_not_connected);
         }
      } // get_table_range
      

      void CsiDbSource::on_connect_complete(CsiDbHelpers::ConnectCommand *command)
      {
         db_connection = command->get_connection();
         connection_pending = false;
         if(db_connection != 0)
         {
            manager->report_source_connect(this);
            if(symbol != 0)
               symbol->on_source_connect(db_thread.get_rep(), db_connection);
            if(is_started())
            {
               start();
               poll_schedule = scheduler->start(this, 0, properties.get_poll_interval(), true);
            }
         }
         else if (manager != nullptr)
         {
            manager->report_source_log(this, StrAsc("connect failed\",\"") + command->last_error);
            manager->report_source_disconnect(this, ManagerClient::disconnect_connection_failed);
            retry_id = timer->arm(this, std_retry_interval);
         }
      } // on_connect_complete


      namespace CsiDbHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_connect_failure definitions
         ////////////////////////////////////////////////////////////
         uint4 const event_connect_failure::event_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::event_connect_failure"));
      };
   };
};

