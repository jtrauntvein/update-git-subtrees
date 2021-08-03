/* Cora.LgrNet.OperationsEnumerator.h

   Copyright (C) 2010, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 29 September 2010
   Last Change: Friday 05 August 2011
   Last Commit: $Date: 2011-08-05 14:48:48 -0600 (Fri, 05 Aug 2011) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_LgrNet_OperationsEnumerator_h
#define Cora_LgrNet_OperationsEnumerator_h

#include "Cora.ClientBase.h"


namespace Cora
{
   namespace LgrNet
   {
      // @group: class forward declarations
      class OperationsEnumerator;
      // @endgroup:


      namespace OperationsEnumeratorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class Report
         ////////////////////////////////////////////////////////////
         class Report
         {
         public:
            ////////////////////////////////////////////////////////////
            // id
            ////////////////////////////////////////////////////////////
            int8 const id;

            ////////////////////////////////////////////////////////////
            // description
            ////////////////////////////////////////////////////////////
            StrAsc description;

            ////////////////////////////////////////////////////////////
            // start_time
            ////////////////////////////////////////////////////////////
            Csi::LgrDate start_time;

            ////////////////////////////////////////////////////////////
            // device_name
            ////////////////////////////////////////////////////////////
            StrUni device_name;

            ////////////////////////////////////////////////////////////
            // priority
            ////////////////////////////////////////////////////////////
            uint4 priority;
            bool priority_changed;
            
            ////////////////////////////////////////////////////////////
            // last_transmit_time
            ////////////////////////////////////////////////////////////
            Csi::LgrDate last_transmit_time;
            bool last_transmit_time_changed;
            
            ////////////////////////////////////////////////////////////
            // last_receive_time
            ////////////////////////////////////////////////////////////
            Csi::LgrDate last_receive_time;
            bool last_receive_time_changed;
            
            ////////////////////////////////////////////////////////////
            // timeout_interval
            ////////////////////////////////////////////////////////////
            uint4 timeout_interval;
            bool timeout_interval_changed;
            
            ////////////////////////////////////////////////////////////
            // state
            ////////////////////////////////////////////////////////////
            StrAsc state;
            bool state_changed;

            ////////////////////////////////////////////////////////////
            // account_name
            ////////////////////////////////////////////////////////////
            StrUni account_name;
            bool account_name_changed;

            ////////////////////////////////////////////////////////////
            // app_name
            ////////////////////////////////////////////////////////////
            StrUni app_name;
            bool app_name_changed;

         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            Report(int8 id_):
               id(id_),
               priority_changed(false),
               last_transmit_time_changed(false),
               last_receive_time_changed(false),
               timeout_interval_changed(false),
               state_changed(false),
               account_name_changed(false),
               app_name_changed(false)
            { }

            ////////////////////////////////////////////////////////////
            // read
            ////////////////////////////////////////////////////////////
            void read(Csi::Messaging::Message &in);
         };
      };
      
      
      ////////////////////////////////////////////////////////////
      // class OperationsEnumeratorClient
      ////////////////////////////////////////////////////////////
      class OperationsEnumeratorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_session_failed,
            failure_invalid_logon,
            failure_unsupported,
            failure_server_security_blocked
         };
         virtual void on_failure(
            OperationsEnumerator *enumerator, failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_operation_added
         ////////////////////////////////////////////////////////////
         typedef OperationsEnumeratorHelpers::Report report_type;
         typedef Csi::SharedPtr<report_type> report_handle;
         virtual void on_operation_added(
            OperationsEnumerator *enumerator, report_handle &op)
         { }

         ////////////////////////////////////////////////////////////
         // on_operation_deleted
         ////////////////////////////////////////////////////////////
         virtual void on_operation_deleted(
            OperationsEnumerator *enumerator, report_handle &op)
         { }

         ////////////////////////////////////////////////////////////
         // on_operation_changed
         ////////////////////////////////////////////////////////////
         virtual void on_operation_changed(
            OperationsEnumerator *enumerator, report_handle &op)
         { }

         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            OperationsEnumerator *enumerator)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class OperationsEnumerator
      //
      // This class defines a component that can be used to monitor the
      // progress of server device operations (generally things that require
      // communication with a datalogger).  It will maintain a set of report
      // objects that define the set of operations currently reported by the
      // LoggerNet server.
      //
      // The application can use this component by creating an instance of this
      // class, optionally setting attributes such as logon name, and invoking
      // one of the two versions of start().  If the LoggerNet server succeeds
      // in establishing the underlying transaction, the component will invoke
      // the client's on_operation_added() once for each known operation and
      // then call the client's on_started() method.  If, at any time, the
      // transaction cannot be started or fails, the client will be informed
      // through its on_failure() method. 
      ////////////////////////////////////////////////////////////
      class OperationsEnumerator:
         public Cora::ClientBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         OperationsEnumeratorClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_ready,
            state_waiting_for_session,
            state_waiting_for_start,
            state_started
         } state;

         ////////////////////////////////////////////////////////////
         // reports
         ////////////////////////////////////////////////////////////
      public:
         typedef OperationsEnumeratorClient client_type;
         typedef client_type::report_type report_type;
         typedef client_type::report_handle report_handle;
         typedef std::map<int8, report_handle> reports_type;
         typedef reports_type::value_type value_type;
      private:
         reports_type reports;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         OperationsEnumerator():
            client(0),
            state(state_ready)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~OperationsEnumerator()
         { reports.clear(); }

         ////////////////////////////////////////////////////////////
         // start (from other component)
         ////////////////////////////////////////////////////////////
         void start(client_type *client_, ClientBase *other_component)
         {
            if(state != state_ready)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_waiting_for_session;
            ClientBase::start(other_component);
         }

         ////////////////////////////////////////////////////////////
         // start (from new router)
         ////////////////////////////////////////////////////////////
         void start(client_type *client_, router_handle router)
         {
            if(state != state_ready)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_waiting_for_session;
            ClientBase::start(router);
         }

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         { ClientBase::finish(); }

         ////////////////////////////////////////////////////////////
         // describe_failure
         ////////////////////////////////////////////////////////////
         static void describe_failure(std::ostream &out, client_type::failure_type failure);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
         // @group: definitions to act as a container

         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef reports_type::const_iterator const_iterator;
         const_iterator begin() const
         { return reports.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         const_iterator end() const
         { return reports.end(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return reports.empty(); }

         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         typedef reports_type::size_type size_type;
         size_type size() const
         { return reports.size(); }

         ////////////////////////////////////////////////////////////
         // find
         ////////////////////////////////////////////////////////////
         const_iterator find(int8 id) const
         { return reports.find(id); }
            
         // @endgroup:
         
      protected:
         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure()
         { on_corabase_failure(corabase_failure_session); }

         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         typedef Csi::Messaging::Router router_type;
         typedef Csi::Messaging::Message message_type;
         virtual void onNetMessage(
            router_type *router, message_type *message);

      private:
         ////////////////////////////////////////////////////////////
         // on_start_ack
         ////////////////////////////////////////////////////////////
         void on_start_ack(message_type *message);

         ////////////////////////////////////////////////////////////
         // on_added_not
         ////////////////////////////////////////////////////////////
         void on_added_not(message_type *message);

         ////////////////////////////////////////////////////////////
         // on_changed_not
         ////////////////////////////////////////////////////////////
         void on_changed_not(message_type *message);

         ////////////////////////////////////////////////////////////
         // on_deleted_not
         ////////////////////////////////////////////////////////////
         void on_deleted_not(message_type *message);
      };
   };
};


#endif
