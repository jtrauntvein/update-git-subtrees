/* Cora.Device.CollectAreaPoller.h

   Copyright (C) 2002, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 24 June 2002
   Last Change: Monday 18 May 2015
   Last Commit: $Date: 2015-05-18 13:10:48 -0600 (Mon, 18 May 2015) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_CollectAreaPoller_h
#define Cora_Device_CollectAreaPoller_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class CollectAreaPoller; 
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class CollectAreaPollerClient
      ////////////////////////////////////////////////////////////
      class CollectAreaPollerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_status_not
         ////////////////////////////////////////////////////////////
         virtual void on_status_not(
            CollectAreaPoller *poller,
            uint4 values_expected,
            uint4 values_stored)
         { }
         
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_server_session_failed = 3,
            outcome_invalid_device_name = 4,
            outcome_unsupported = 5,
            outcome_server_security_blocked = 6,
            outcome_logger_security_blocked = 7,
            outcome_comm_failure = 8,
            outcome_communication_disabled = 9,
            outcome_table_defs_invalid = 10,
            outcome_invalid_collect_area_name = 11,
            outcome_file_io_failure = 12,
            outcome_logger_busy = 13,
            outcome_aborted = 14
         };
         virtual void on_complete(
            CollectAreaPoller *poller,
            outcome_type outcome)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class CollectAreaPoller
      ////////////////////////////////////////////////////////////
      class CollectAreaPoller:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // collect_area_name
         //
         // Identifies the collect area that should be polled.
         //////////////////////////////////////////////////////////// 
         StrUni collect_area_name;

         ////////////////////////////////////////////////////////////
         // values_expected
         ////////////////////////////////////////////////////////////
         uint4 values_expected;

         ////////////////////////////////////////////////////////////
         // values_stored
         ////////////////////////////////////////////////////////////
         uint4 values_stored;

         ////////////////////////////////////////////////////////////
         // priority
         ////////////////////////////////////////////////////////////
         uint4 priority;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         CollectAreaPoller();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~CollectAreaPoller();

         ////////////////////////////////////////////////////////////
         // get_collect_area_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_collect_area_name() const { return collect_area_name; }

         ////////////////////////////////////////////////////////////
         // set_collect_area_name
         ////////////////////////////////////////////////////////////
         void set_collect_area_name(StrUni const &collect_area_name_);

         ////////////////////////////////////////////////////////////
         // get_priority
         ////////////////////////////////////////////////////////////
         enum priority_type
         {
            priority_lower = 1,
            priority_same = 2,
            priority_higher = 3
         };
         priority_type get_priority() const
         { return static_cast<priority_type>(priority); }

         ////////////////////////////////////////////////////////////
         // set_priority
         ////////////////////////////////////////////////////////////
         void set_priority(priority_type priority_);

         ////////////////////////////////////////////////////////////
         // get_values_expected
         ////////////////////////////////////////////////////////////
         uint4 get_values_expected() const
         { return values_expected; }

         ////////////////////////////////////////////////////////////
         // get_values_stored
         ////////////////////////////////////////////////////////////
         uint4 get_values_stored() const
         { return values_stored; }

         ////////////////////////////////////////////////////////////
         // start (new connection)
         ////////////////////////////////////////////////////////////
         typedef CollectAreaPollerClient client_type;
         void start(
            client_type *client_,
            router_handle &router);

         ////////////////////////////////////////////////////////////
         // start (share existing connection)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            ClientBase *other_client);

         /**
          * Overloads the base class to cancel a transaction in progress.
          */
         virtual bool cancel();

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // format_outcome
         ////////////////////////////////////////////////////////////
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_session_failure();

      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Keeps track of the poll transaction number.
          */
         uint4 poll_tran;
      };
   };
};


#endif
