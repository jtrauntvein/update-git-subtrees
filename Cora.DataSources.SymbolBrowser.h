/* Cora.DataSources.SymbolBrowser.h

   Copyright (C) 2008, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 August 2008
   Last Change: Thursday 13 February 2020
   Last Commit: $Date: 2020-02-14 09:53:04 -0600 (Fri, 14 Feb 2020) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_DataSources_SymbolBrowser_h
#define Cora_DataSources_SymbolBrowser_h

#include "Cora.DataSources.SymbolBase.h"
#include "Cora.DataSources.Manager.h"


namespace Cora
{
   namespace DataSources
   {
      /**
       * Defines an object that receives event notifications from the symbol browser regarding the
       * discovery and removal of symbols in the manager network.
       */
      class SymbolBrowser;
      class SymbolBrowserClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when a new symbol has been discovered by the browser.
          *
          * @param sender Specifies the browser that discovered the symbol.
          *
          * @param symbol Specifies the symbol that has been discovered.
          */
         typedef Csi::SharedPtr<SymbolBase> symbol_handle;
         virtual void on_symbol_added(
            SymbolBrowser *sender, symbol_handle &symbol)
         { }

         /**
          * Called when a symbol that was previously reported has been determined to be deleted.
          *
          * @param sender Specifies the browser that sent this notification.
          *
          * @param symbol Specifies the affected symbol.
          *
          * @param reason Specifies a code that explains the reason for the removal.
          */
         enum remove_reason_type
         {
            remove_unknown = 0,
            remove_server_connection_lost = 1,
            remove_station_deleted = 2,
            remove_table_deleted = 3,
            remove_station_shut_down = 4,
            remove_source_removed = 5,
            remove_table_changed = 6,
            remove_column_deleted = 7
         };
         virtual void on_symbol_removed(
            SymbolBrowser *sender, symbol_handle &symbol, remove_reason_type reason)
         { }

         /**
          * Called when the connected state of a source level symbol has been changed.
          *
          * @param sender Specifies the browser that sent this notification.
          *
          * @param source_symbol Speciifes the source symbol.
          */
         virtual void on_source_connect_change(
            SymbolBrowser *sender, symbol_handle &source_symbol)
         { }

         /**
          * Called when the enabled state of a symbol (being enabled for scheduled collection) has
          * been changed.
          *
          * @param sender Specifies the browser reporting this event.
          *
          * @param symbol Specifies the effected symbol.
          */
         virtual void on_symbol_enabled_change(
            SymbolBrowser *sender, symbol_handle &symbol)
         { }

         /**
          * Called to report that the expansion of the specified symbol has been completed.
          *
          * @param sender Specifies the browser sending this notification.
          *
          * @param symbol Specifies the symbol that has finished expansion.
          */
         virtual void on_symbol_expansion_complete(
            SymbolBrowser *sender, symbol_handle &symbol)
         { }
      };


      /**
       * Defines an object that can be used to browse the symbols (data sources, stations, tables,
       * fields, & etc.) that are available in the network managed by an associated data source
       * manager.
       */
      class SymbolBrowser:
         public ManagerClient,
         public Csi::EventReceiver
      {
      public:
         /**
          * Constructor
          *
          * @param manager_ Specifies the data source manager.
          *
          * @param first_client Optionally specifies a first client reference.
          */
         typedef Csi::SharedPtr<Manager> manager_handle;
         typedef SymbolBrowserClient client_type;
         SymbolBrowser(manager_handle manager_, client_type *first_client = 0);

         /**
          * Destructor.
          */
         virtual ~SymbolBrowser();

         /**
          * @param client Specifies a client that can be added to monitor the symbols associated
          * with this browser.
          */
         void add_client(client_type *client);

         /**
          * @param client Specifies a client that should be removed from the set monitoring this
          * browser.
          */
         void remove_client(client_type *client);

         /**
          * @return Returns an iterator to the first source level symbol.
          */
         typedef SymbolBase::children_type symbols_type;
         typedef symbols_type::value_type value_type;
         typedef symbols_type::iterator iterator;
         typedef symbols_type::const_iterator const_iterator;
         iterator begin()
         { return symbols.begin(); }
         const_iterator begin() const
         { return symbols.begin(); }

         /**
          * @return Returns an iterator beyond the last source level symbol.
          */
         iterator end()
         { return symbols.end(); }
         const_iterator end() const
         { return symbols.end(); }

         /**
          * @return Returns true if there are no symbols known.
          */
         bool empty() const
         { return symbols.empty(); }

         /**
          * @return Returns the number of source level symbols.
          */
         typedef symbols_type::size_type size_type;
         size_type size() const
         { return symbols.size(); }
         
         /**
          * Overloads the base class version to handle the notification of a new source.
          */
         virtual void on_source_added(
            Manager *manager, source_handle &source);

         /**
          * Overloads the base class version to handle the notification of a source having been
          * removed.
          */
         virtual void on_source_removed(
            Manager *manager, source_handle &source);

         /**
          * Sends a notification of a symbol having been added.
          *
          * @param symbol Specifies the symbol that has been added.
          */
         void send_symbol_added(value_type &symbol);

         /**
          * Sends a notification of a symbol having been removed.
          *
          * @param symbol Specifies the affected symbol.
          *
          * @param reason Specifies the reason.
          */
         void send_symbol_removed(value_type &symbol, client_type::remove_reason_type reason);

         /**
          * Sends a notification that the specified source level symbol connection state has been
          * changed.
          *
          * @param symbol Specifies the symbol that has reported this.
          */
         void send_symbol_connected_change(SymbolBase *symbol);

         /**
          * Sends a notification that the specified symbol enabled state has been changed.
          *
          * @param symbol Specifies the symbol reporting this change.
          */
         void send_symbol_enabled_change(SymbolBase *symbol);

         /**
          * Sends the notification that the specified symbokl expansion is complete.
          */
         void send_expansion_complete(SymbolBase *symbol);

         /**
          * @return Returns the symbol handle associated with the specified pointer.
          */
         value_type find_symbol(SymbolBase *symbol);

         /**
          * @return Returns the symbol handle associated with the specified uri.
          */
         value_type find_symbol(StrUni const &uri);

         /**
          * Overloads the base class version to handle asynchronous events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         /**
          * Specifies the collection of clients associated with this browser.
          */
         typedef std::list<client_type *> clients_type;
         clients_type clients;

         /**
          * Specifies the collection of source level symbols.
          */
         symbols_type symbols;

         /**
          * Specifies the data source manager.
          */
         manager_handle manager;
      };
   };
};


#endif
