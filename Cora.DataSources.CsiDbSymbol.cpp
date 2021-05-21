/* Cora.DataSources.CsiDbSymbol.cpp

   Copyright (C) 2009, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 07 February 2009
   Last Change: Friday 14 February 2020
   Last Commit: $Date: 2020-02-14 09:53:04 -0600 (Fri, 14 Feb 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.CsiDbSymbol.h"
#include "Cora.DataSources.CsiDbSource.h"
#include "Cora.DataSources.SymbolBrowser.h"


namespace Cora
{
   namespace DataSources
   {
      ////////////////////////////////////////////////////////////
      // class CsiDbSymbol definitions
      ////////////////////////////////////////////////////////////
      CsiDbSymbol::CsiDbSymbol(CsiDbSource *source):
         SymbolBase(source, source->get_name()),
         db_source(source),
         expansion_started(false)
      { }

      
      CsiDbSymbol::~CsiDbSymbol()
      {
         db_connection.clear();
      } // destructor

      
      bool CsiDbSymbol::is_connected() const
      { return db_source->is_connected(); }
      
         
      void CsiDbSymbol::start_expansion()
      {
         expansion_started = true;
         if(is_connected())
            refresh();
      } // start_expansion


      void CsiDbSymbol::refresh()
      {
         if(db_connection != 0)
            db_thread->add_command(new list_tables_type(this, db_connection));
      } // refresh


      void CsiDbSymbol::on_source_connect(
         thread_type *db_thread_, connection_handle &db_connection_)
      {
         db_connection = db_connection_;
         db_thread = db_thread_;
         if(expansion_started)
            refresh();
      } // on_source_connect


      void CsiDbSymbol::on_source_disconnect()
      {
         if(db_connection != 0)
         {
            db_connection.clear();
            browser->send_symbol_connected_change(this);
         }
         while(!children.empty())
         {
            browser->send_symbol_removed(children.front(), SymbolBrowserClient::remove_server_connection_lost);
            children.pop_front();
         }
      } // on_source_disconnect


      void CsiDbSymbol::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace CsiDbHelpers;
         if(ev->getType() == CommandCompleteEvent::event_id)
         {
            CommandCompleteEvent *event = static_cast<CommandCompleteEvent *>(ev.get_rep());
            if(event->command->command_type == list_tables_type::command_id)
               on_list_tables_complete(static_cast<list_tables_type *>(event->command.get_rep()));
         }
      } // receive


      void CsiDbSymbol::on_list_tables_complete(list_tables_type *command)
      {
         // we will reconcile in two stages.  If there are any children that don't have
         // corresponding names in the table names list, those symbols need to be deleted.
         // Similarly, if there are any tables listed for which we don't have symbols, we will need
         // to add those symbols.
         iterator si = begin();
         bool const was_empty = children.empty();
         StrAsc child_name;
         while(si != end())
         {
            value_type child = *si;
            list_tables_type::const_iterator ti;
            child_name = child->get_name().to_utf8();
            child_name.replace("\\.", ".");
            ti = std::find(command->begin(), command->end(), child_name);
            if(ti == command->end())
            {
               iterator dsi = si++;
               children.erase(dsi);
               browser->send_symbol_removed(child, SymbolBrowserClient::remove_table_deleted);
            }
            else
               ++si;
         }
         for(list_tables_type::const_iterator ci = command->begin(); ci != command->end(); ++ci)
         {
            StrUni name(*ci);
            name.replace(L".", L"\\.");
            if(was_empty || find(name) == end())
            {
               value_type symbol(
                  new CsiDbTableSymbol(name, this, db_thread, db_connection));
               children.insert(
                  std::upper_bound(
                     begin(), end(), symbol, symbol_name_less()),
                  symbol);
               browser->send_symbol_added(symbol);
            }
         }
         browser->send_expansion_complete(this);
      } // on_list_tables_complete


      ////////////////////////////////////////////////////////////
      // class CsiDbTableSymbol definitions
      ////////////////////////////////////////////////////////////
      void CsiDbTableSymbol::refresh()
      {
         if(db_connection != 0)
         {
            StrAsc unquoted(name.to_utf8());
            unquoted.replace("\\.", ".");
            db_thread->add_command(new list_columns_type(this, db_connection, unquoted));
         }
      } // refresh


      void CsiDbTableSymbol::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace CsiDbHelpers;
         if(ev->getType() == CommandCompleteEvent::event_id)
         {
            CommandCompleteEvent *event = static_cast<CommandCompleteEvent *>(ev.get_rep());
            if(event->command->command_type == list_columns_type::command_id)
               on_list_columns_complete(static_cast<list_columns_type *>(event->command.get_rep()));
         }
      } // receive


      void CsiDbTableSymbol::on_list_columns_complete(list_columns_type *command)
      {
         // as with the list of table names, we will need to reconcile the list of column names
         // returned from the database DLL with the list of child names.  We will first see if there
         // are any children that are not in the new list
         iterator si = begin();
         bool const was_empty = children.empty();
         while(si != end())
         {
            value_type child = *si;
            StrAsc child_name(child->name.to_utf8());
            list_columns_type::const_iterator ci = std::find(command->begin(), command->end(), child_name);
            if(ci == command->end())
            {
               iterator dsi = si++;
               children.erase(dsi);
               browser->send_symbol_removed(child, SymbolBrowserClient::remove_column_deleted);
            }
            else
               ++si;
         }
         for(list_columns_type::const_iterator ci = command->begin(); ci != command->end(); ++ci)
         {
            StrAsc const &name(*ci);
            if(was_empty || find(name) == end())
            {
               value_type symbol(new CsiDbColumnSymbol(name, this));
               children.push_back(symbol);
               browser->send_symbol_added(symbol);
            } 
         }
         browser->send_expansion_complete(this);
      } // on_list_columns_complete
   };
};

