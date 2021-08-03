/* Cora.LgrNet.CollectionStatusMonitor.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 09 August 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_LgrNet_CollectionStatusMonitor_h
#define Cora_LgrNet_CollectionStatusMonitor_h


#include "Cora.ClientBase.h"
#include "Csi.InstanceValidator.h"
#include "Cora.LgrNet.BrokerLister.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class CollectionStatusMonitor;
      namespace CollectionStatusMonitorHelpers
      { class Station; }
      //@endgroup
      

      ////////////////////////////////////////////////////////////
      // class CollectionStatusMonitorClient
      ////////////////////////////////////////////////////////////
      class CollectionStatusMonitorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //////////////////////////////////////////////////////////// 
         virtual void on_started(CollectionStatusMonitor *monitor)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         //////////////////////////////////////////////////////////// 
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_logon = 1,
            failure_server_security = 2,
            failure_unsupported = 3,
            failure_session = 4,
         };
         virtual void on_failure(CollectionStatusMonitor *monitor,
                                 failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_station_added
         //////////////////////////////////////////////////////////// 
         // Called whenever a station added event is detected
         virtual void on_station_added(
            CollectionStatusMonitor *monitor,
            StrUni const &station_name,
            bool is_scheduled)
         { }

         ////////////////////////////////////////////////////////////
         // on_station_deleted
         //////////////////////////////////////////////////////////// 
         // Called whenever a station deleted event is detected
         virtual void on_station_deleted(
            CollectionStatusMonitor *monitor,
            StrUni const &station_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_station_scheduled
         //////////////////////////////////////////////////////////// 
         // Called whenever the station schedule enabled status changes
         virtual void on_station_scheduled(
            CollectionStatusMonitor *monitor,
            StrUni const &station_name,
            bool is_scheduled)
         { }

         ////////////////////////////////////////////////////////////
         // on_table_added
         //////////////////////////////////////////////////////////// 
         // Called whenever a new table added event has been detected
         virtual void on_table_added(
            CollectionStatusMonitor *monitor,
            StrUni const &station_name,
            StrUni const &table_name,
            bool is_scheduled)
         { }

         ////////////////////////////////////////////////////////////
         // on_table_scheduled
         //////////////////////////////////////////////////////////// 
         // Called whenever a table scheduled status change event occurs
         virtual void on_table_scheduled(
            CollectionStatusMonitor *monitor,
            StrUni const &station_name,
            StrUni const &table_name,
            bool is_scheduled)
         { }

         ////////////////////////////////////////////////////////////
         // on_table_deleted
         //////////////////////////////////////////////////////////// 
         // Called when a table deleted event has been detected
         virtual void on_table_deleted(
            CollectionStatusMonitor *monitor,
            StrUni const &station_name,
            StrUni const &table_name)
         { }
      };



      ////////////////////////////////////////////////////////////
      // class CollectionStatusMonitor
      //
      // Defines an object that monitors the server network and provides notifications regarding
      // whether stations are enabled for scheduled collection and what tables on those stations are
      // enabled.
      ////////////////////////////////////////////////////////////
      class CollectionStatusMonitor:
         public ClientBase,
         public BrokerListerClient,
         public Csi::EvReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         CollectionStatusMonitor();

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~CollectionStatusMonitor();

         ////////////////////////////////////////////////////////////
         // start
         //////////////////////////////////////////////////////////// 
         void start(
            CollectionStatusMonitorClient *client_,
            router_handle &router);
         void start(
            CollectionStatusMonitorClient *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         //////////////////////////////////////////////////////////// 
         void finish();

      protected:
         //@group class ClientBase overloaded methods
         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         //////////////////////////////////////////////////////////// 
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_corabase_session_failure();
         //@endgroup

         //@group class BrokerListerClient overloaded methods
         ////////////////////////////////////////////////////////////
         // on_started
         //////////////////////////////////////////////////////////// 
         virtual void on_started(
            BrokerLister *lister);

         ////////////////////////////////////////////////////////////
         // on_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_failure(
            BrokerLister *lister,
            failure_type failure);
         
         ////////////////////////////////////////////////////////////
         // on_broker_added
         //////////////////////////////////////////////////////////// 
         virtual void on_broker_added(
            BrokerLister *lister,
            StrUni const &broker_name,
            uint4 broker_id,
            broker_type_code type);

         ////////////////////////////////////////////////////////////
         // on_broker_deleted
         //////////////////////////////////////////////////////////// 
         virtual void on_broker_deleted(
            BrokerLister *lister,
            StrUni const &broker_name,
            uint4 broker_id,
            broker_type_code type);

         ////////////////////////////////////////////////////////////
         // on_broker_renamed
         ////////////////////////////////////////////////////////////
         virtual void on_broker_renamed(
            BrokerLister *lister,
            StrUni const &broker_name,
            StrUni const &old_name,
            uint4 broker_id,
            broker_type_code type);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         //////////////////////////////////////////////////////////// 
         void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // on_station_active
         //
         // Called after a station object has been initialised
         //////////////////////////////////////////////////////////// 
         void on_station_active(StrUni const &station_name);

      private:
         ////////////////////////////////////////////////////////////
         // client
         //////////////////////////////////////////////////////////// 
         typedef CollectionStatusMonitorClient client_type;
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         //////////////////////////////////////////////////////////// 
         enum state_type
         {
            state_standby,
            state_before_active,
            state_active,
         } state;

         ////////////////////////////////////////////////////////////
         // wait_for_start_count
         //
         // Keeps track of the number of scheduled tables monitors that we are waiting for an
         // on_start() event while in a pre-active state. This value is change only while in a
         // before_active state. It is incremented when stations are discovered and decremented when
         // an on_started event arrives.
         //////////////////////////////////////////////////////////// 
         uint4 wait_for_start_count;

         ////////////////////////////////////////////////////////////
         // stations
         //
         // List of all the stations currently known to this object
         //////////////////////////////////////////////////////////// 
         typedef Csi::SharedPtr<CollectionStatusMonitorHelpers::Station> station_handle;
         typedef std::map<StrUni, station_handle> stations_type;
         stations_type stations;

         ////////////////////////////////////////////////////////////
         // lister
         //////////////////////////////////////////////////////////// 
         Csi::SharedPtr<BrokerLister> lister;

         // we declare the station helper class a friend here so that it can operate independently
         friend class CollectionStatusMonitorHelpers::Station;
      };
   };
};

#endif
