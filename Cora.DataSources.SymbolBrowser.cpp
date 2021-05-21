/* Cora.DataSources.SymbolBrowser.cpp

   Copyright (C) 2008, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 August 2008
   Last Change: Friday 14 February 2020
   Last Commit: $Date: 2020-02-14 09:53:04 -0600 (Fri, 14 Feb 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.SymbolBrowser.h"


namespace Cora
{
   namespace DataSources
   {
      SymbolBrowser::SymbolBrowser(
         manager_handle manager_, client_type *first_client):
         manager(manager_)
      {
         if(first_client)
            add_client(first_client);
         manager->add_client(this);
         for(Manager::iterator si = manager->begin(); si != manager->end(); ++si)
            on_source_added(manager.get_rep(), *si);
      } // constructor


      SymbolBrowser::~SymbolBrowser()
      {
         manager->remove_client(this);
         manager.clear();
         symbols.clear();
         clients.clear();
      } // destructor


      void SymbolBrowser::add_client(client_type *client)
      {
         clients_type::iterator ci = std::find(clients.begin(), clients.end(), client);
         if(ci == clients.end())
            clients.push_back(client);
      } // add_client


      void SymbolBrowser::remove_client(client_type *client)
      {
         clients_type::iterator ci = std::find(clients.begin(), clients.end(), client);
         if(ci != clients.end())
            clients.erase(ci);
      } // remove_client


      void SymbolBrowser::on_source_added(
         Manager *manager, source_handle &source)
      {
         value_type symbol(source->get_source_symbol());
         if(symbol.get_rep())
         {
            symbol->set_browser(this);
            symbols.push_back(symbol);
            send_symbol_added(symbol);
         }
      } // on_source_added


      namespace
      {
         struct symbol_has_source
         {
            SourceBase *source;
            symbol_has_source(SourceBase *source_):
               source(source_)
            { }

            bool operator ()(SymbolBrowser::value_type &symbol)
            { return symbol->get_source() == source; }
         };
      };
      

      void SymbolBrowser::on_source_removed(
         Manager *manager, source_handle &source)
      {
         iterator si = std::find_if(symbols.begin(), symbols.end(), symbol_has_source(source.get_rep()));
         if(si != symbols.end())
         {
            value_type symbol(*si);
            symbols.erase(si);
            send_symbol_removed(symbol, client_type::remove_source_removed);
         }
      } // on_source_removed


      namespace
      {
         class event_symbol_added: public Csi::Event
         {
         public:
            static uint4 const event_id;
            SymbolBrowser::value_type symbol;

            static void cpost(SymbolBrowser *browser, SymbolBrowser::value_type &symbol)
            {
               event_symbol_added *ev = new event_symbol_added(browser, symbol);
               ev->post();
            }

         private:
            event_symbol_added(SymbolBrowser *browser, SymbolBrowser::value_type &symbol_):
               Event(event_id, browser),
               symbol(symbol_)
            { } 
         };


         uint4 const event_symbol_added::event_id = Csi::Event::registerType(
            "Cora::DataSources::SymbolBrowser::event_symbol_added");
      };

      
      void SymbolBrowser::send_symbol_added(value_type &symbol)
      { event_symbol_added::cpost(this, symbol); }


      namespace
      {
         class event_symbol_removed: public Csi::Event
         {
         public:
            static uint4 const event_id;
            SymbolBrowser::value_type symbol;
            SymbolBrowserClient::remove_reason_type reason;

            static void cpost(
               SymbolBrowser *browser,
               SymbolBrowser::value_type &symbol,
               SymbolBrowserClient::remove_reason_type reason)
            {
               event_symbol_removed *ev = new event_symbol_removed(browser, symbol, reason);
               ev->post();
            }

         private:
            event_symbol_removed(
               SymbolBrowser *browser,
               SymbolBrowser::value_type &symbol_,
               SymbolBrowserClient::remove_reason_type reason_):
               Event(event_id, browser),
               symbol(symbol_),
               reason(reason_)
            { } 
         };


         uint4 const event_symbol_removed::event_id = Csi::Event::registerType(
            "Cora::DataSources::SymbolBrowser::event_symbol_removed");
      };
      

      void SymbolBrowser::send_symbol_removed(
         value_type &symbol, client_type::remove_reason_type reason)
      { event_symbol_removed::cpost(this, symbol, reason); }


      namespace
      {
         class event_symbol_connected_change: public Csi::Event
         {
         public:
            static uint4 const event_id;
            SymbolBrowser::value_type symbol;

            static void cpost(SymbolBrowser *browser, SymbolBrowser::value_type symbol)
            {
               event_symbol_connected_change *ev = new event_symbol_connected_change(browser, symbol);
               ev->post();
            }

         private:
            event_symbol_connected_change(SymbolBrowser *browser, SymbolBrowser::value_type &symbol_):
               Event(event_id, browser),
               symbol(symbol_)
            { } 
         };


         uint4 const event_symbol_connected_change::event_id = Csi::Event::registerType(
            "Cora::DataSources::SymbolBrowser::event_symbol_connected_change");
      };

      
      void SymbolBrowser::send_symbol_connected_change(SymbolBase *symbol_)
      { event_symbol_connected_change::cpost(this, find_symbol(symbol_)); }


      namespace
      {
         class event_symbol_enabled_change: public Csi::Event
         {
         public:
            static uint4 const event_id;
            SymbolBrowser::value_type symbol;

            static void cpost(SymbolBrowser *browser, SymbolBrowser::value_type symbol)
            {
               event_symbol_enabled_change *ev = new event_symbol_enabled_change(browser, symbol);
               ev->post();
            }

         private:
            event_symbol_enabled_change(SymbolBrowser *browser, SymbolBrowser::value_type &symbol_):
               Event(event_id, browser),
               symbol(symbol_)
            { } 
         };


         uint4 const event_symbol_enabled_change::event_id = Csi::Event::registerType(
            "Cora::DataSources::SymbolBrowser::event_symbol_enabled_change");
      };
      
      
      void SymbolBrowser::send_symbol_enabled_change(SymbolBase *symbol_)
      { event_symbol_enabled_change::cpost(this, find_symbol(symbol_)); }


      namespace
      {
         class event_symbol_expansion_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            SymbolBrowser::value_type symbol;

            static void cpost(SymbolBrowser *browser, SymbolBrowser::value_type symbol)
            { (new event_symbol_expansion_complete(browser, symbol))->post(); }

         private:
            event_symbol_expansion_complete(SymbolBrowser *browser, SymbolBrowser::value_type &symbol_):
               Event(event_id, browser),
               symbol(symbol_)
            { }
         };

         uint4 const event_symbol_expansion_complete::event_id(
            Csi::Event::registerType("Cora::DataSources::SymbolBrowser::symbol-expansion_complete"));
      };


      void SymbolBrowser::send_expansion_complete(SymbolBase *symbol)
      { event_symbol_expansion_complete::cpost(this, find_symbol(symbol)); }

      
      SymbolBrowser::value_type SymbolBrowser::find_symbol(SymbolBase *symbol)
      {
         value_type rtn;
         symbols_type search_stack(symbols);
         while(rtn == 0 && !search_stack.empty())
         {
            value_type candidate = search_stack.front();
            search_stack.pop_front();
            if(candidate == symbol)
               rtn = candidate;
            else if(symbol->has_parent(candidate.get_rep()))
               search_stack.insert(search_stack.end(), candidate->begin(), candidate->end());
         }
         return rtn;
      } // find_symbol


      namespace
      {
         struct symbol_has_name
         {
            StrUni const &name;
            symbol_has_name(StrUni const &name_):
               name(name_)
            { }

            bool operator ()(SymbolBrowser::value_type &symbol) const
            { return symbol->get_name() == name; }
         };
      };
      

      SymbolBrowser::value_type SymbolBrowser::find_symbol(StrUni const &uri)
      {
         // the first thing that we shall do is to break down the URI into its component symbols
         value_type rtn;
         SourceBase::symbols_type uri_symbols;

         manager->breakdown_uri(uri_symbols, uri);
         if(!uri_symbols.empty())
         {
            // make sure that the symbol is escaped
            StrUni name(uri_symbols.front().first);
            name.replace(L".", L"\\.");
            
            // we can now search for the source that is identified in the URI.  Once we have located
            // that, we can iterate through the set of symbols in the name.
            iterator si(std::find_if(begin(), end(), symbol_has_name(name)));
            if(si != end())
            {
               rtn = *si;
               uri_symbols.pop_front();
               while(!uri_symbols.empty() && rtn != 0)
               {
                  name = uri_symbols.front().first;
                  name.replace(L".", L"\\.");
                  si = std::find_if(rtn->begin(), rtn->end(), symbol_has_name(name));
                  if(si != rtn->end())
                  {
                     rtn = *si;
                     uri_symbols.pop_front();
                  }
                  else
                     rtn.clear();
               }
            }
         }
         return rtn;
      } // find_symbol


      void SymbolBrowser::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         uint4 id = ev->getType();
         if(id == event_symbol_added::event_id)
         {
            event_symbol_added *event = static_cast<event_symbol_added *>(ev.get_rep());
            clients_type::iterator ci = clients.begin();
            while(ci != clients.end())
            {
               client_type *client = *ci;
               if(client_type::is_valid_instance(client))
               {
                  client->on_symbol_added(this, event->symbol);
                  ++ci;
               }
               else
               {
                  clients_type::iterator dci = ci++;
                  clients.erase(dci);
               }
            }
         }
         else if(id == event_symbol_removed::event_id)
         {
            event_symbol_removed *event = static_cast<event_symbol_removed *>(ev.get_rep());
            clients_type::iterator ci = clients.begin();
            while(ci != clients.end())
            {
               client_type *client = *ci;
               if(client_type::is_valid_instance(client))
               {
                  client->on_symbol_removed(this, event->symbol, event->reason);
                  ++ci;
               }
               else
               {
                  clients_type::iterator dci = ci++;
                  clients.erase(dci);
               }
            }
         }
         else if(id == event_symbol_connected_change::event_id)
         {
            event_symbol_connected_change *event = static_cast<event_symbol_connected_change *>(ev.get_rep());
            clients_type::iterator ci = clients.begin();
            while(ci != clients.end() && event->symbol != 0)
            {
               client_type *client = *ci;
               if(client_type::is_valid_instance(client))
               {
                  client->on_source_connect_change(this, event->symbol);
                  ++ci;
               }
               else
               {
                  clients_type::iterator dci = ci++;
                  clients.erase(dci); 
               }
            } 
         }
         else if(id == event_symbol_enabled_change::event_id)
         {
            event_symbol_enabled_change *event = static_cast<event_symbol_enabled_change *>(ev.get_rep());
            clients_type::iterator ci = clients.begin();
            while(ci != clients.end() && event->symbol != 0)
            {
               client_type *client = *ci;
               if(client_type::is_valid_instance(client))
               {
                  ++ci;
                  client->on_symbol_enabled_change(this, event->symbol);
               }
               else
               {
                  clients_type::iterator dci = ci++;
                  clients.erase(dci);
               }
            } 
         }
         else if(id == event_symbol_expansion_complete::event_id)
         {
            auto *event((event_symbol_expansion_complete *)ev.get_rep());
            clients_type temp(clients);
            for(auto ci = temp.begin(); ci != temp.end(); ++ci)
            {
               client_type *client(*ci);
               if(client_type::is_valid_instance(client))
                  client->on_symbol_expansion_complete(this, event->symbol);
               else
                  remove_client(client);
            }
         }
      } // receive
   };
};

