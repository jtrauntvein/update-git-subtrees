/* Cora.DataSources.LgrNetSymbols.h

   Copyright (C) 2008, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 August 2008
   Last Change: Wednesday 27 May 2020
   Last Commit: $Date: 2020-05-27 16:12:00 -0600 (Wed, 27 May 2020) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_DataSources_LgrNetSymbols_h
#define Cora_DataSources_LgrNetSymbols_h

#include "Cora.DataSources.SymbolBase.h"
#include "Cora.LgrNet.BrokerBrowser2.h"
#include "OneShot.h"


namespace Cora
{
   namespace DataSources
   {
      /**
       * Defines an object that acts as the symbol for the LoggerNet data source type.
       */
      class LgrNetSource;
      class LgrNetSourceSymbol:
         public SymbolBase,
         public Cora::LgrNet::BrokerBrowser2Client,
         public OneShotClient
      {
      public:
         /**
          * Constructor
          *
          * @param source_ Specifies the source assocated with this symbol.
          *
          * @param name Specifiesthe name of the source.
          */
         LgrNetSourceSymbol(
            LgrNetSource *source_, StrUni const &name);

         /**
          * Desgtructor
          */
         virtual ~LgrNetSourceSymbol();

         /**
          * @return Overloads the base class version to return the type code for this symbol.
          */
         virtual symbol_type_code get_symbol_type() const override
         { return type_lgrnet_source; }
         
         /**
          * @return Overloads the base class version to return the component that maintains the
          * source connection to loggernet.
         */
         virtual Cora::ClientBase *get_loggernet_component() override;

         /**
          * @return Overloads the base class version to indicate that the symbpol can be expanded.
          */
         virtual bool can_expand() const override
         { return true; }

         /**
          * @return Overloads the base class version to start the expansion  process.
          */
         virtual void start_expansion() override;

         /**
          * @return Returns true if the source connection is valid.
          */
         virtual bool is_connected() const override;

         /**
          * @return Returns true if the source connection is valid.
          */
         virtual bool is_enabled() const override
         { return is_connected(); }
         
         /**
          * Overloads the base class version to handle the start event for the server connection
          * started event.
          */
         typedef Cora::LgrNet::BrokerBrowser2 brokers_type;
         virtual void on_server_connect_started(brokers_type *brokers) override;

         /**
          * Overloads the base class version to handle the failed connection event.
          */
         virtual void on_server_connect_failed(
            brokers_type *brokers,
            brokers_type::client_type::failure_type failure) override;

         /**
          * @return Overloads the base class version to indicate that specific brokers information
          * should be sent.
          */
         virtual bool get_notify_specifics() override
         { return true; }
         
         /**
          * Overloads the event as a no-op.
          */
         virtual void on_brokers_changed() override
         { }

         /**
          * Overloads the event as a no-op.
          */
         virtual void on_tables_changed(StrUni const &broker_name) override
         { }

         /**
          * Overloads the event a no-op.
          */
         virtual void on_table_defs_updated(
            StrUni const &broker_name, StrUni const &table_name) override
         { }

         /**
          * Overloads the base class version to handle the event when a new data broker has been
          * added.
          */
         virtual void on_broker_added(StrUni const &broker_name) override;

         /**
          * Overloads the base class version to handle the event when a broker has been deleted.
          */
         virtual void on_broker_deleted(StrUni const &broker_name) override;

         /**
          * Handles timed events for this component.
          */
         virtual void onOneShotFired(uint4 id) override;
         
      private:
         /**
          * Specifies the data source for this symbol.
          */
         LgrNetSource *source;

         /**
          * Specifies the component that tracks data brokers.
          */
         Csi::SharedPtr<brokers_type> brokers;

         /**
          * Set to true if we have started monitoring data brokers.
          */
         bool brokers_are_started;

         /**
          * Identifies the timer used to start connection retries.
          */
         uint4 retry_id;

         /**
          * Specifies the number of times that the connection has been attempted.
          */
         int retries_count;
      };
   };
};


#endif
