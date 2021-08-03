/* Cora.Device.PoolMonitor.h

   Copyright (C) 2011, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Tuesday 01 February 2011
   Last Change: Tuesday 01 February 2011
   Last Commit: $Date: 2011-02-03 13:45:54 -0600 (Thu, 03 Feb 2011) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_PoolMonitor_h
#define Cora_Device_PoolMonitor_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      // @group: class forward declarations

      class PoolMonitor;
      namespace PoolMonitorHelpers
      {
         class Resource;
      };
      
      // @endgroup:


      namespace PoolMonitorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class Resource
         ////////////////////////////////////////////////////////////
         class Resource
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            Resource(StrAsc const &name_):
               name(name_),
               error_rate(0.0),
               skipped_count(0),
               available(false)
            { }

            ////////////////////////////////////////////////////////////
            // get_name
            ////////////////////////////////////////////////////////////
            StrAsc const &get_name() const
            { return name; }

            ////////////////////////////////////////////////////////////
            // get_error_rate
            ////////////////////////////////////////////////////////////
            float get_error_rate() const
            { return error_rate; }

            ////////////////////////////////////////////////////////////
            // get_skipped_count
            ////////////////////////////////////////////////////////////
            uint4 get_skipped_count() const
            { return skipped_count; }

            ////////////////////////////////////////////////////////////
            // get_available
            ////////////////////////////////////////////////////////////
            bool get_available() const
            { return available; }
               
            ////////////////////////////////////////////////////////////
            // read
            ////////////////////////////////////////////////////////////
            void read(Csi::Messaging::Message *in);

         private:
            ////////////////////////////////////////////////////////////
            // name
            ////////////////////////////////////////////////////////////
            StrAsc name;

            ////////////////////////////////////////////////////////////
            // error_rate
            ////////////////////////////////////////////////////////////
            float error_rate;

            ////////////////////////////////////////////////////////////
            // skipped_count
            ////////////////////////////////////////////////////////////
            uint4 skipped_count;

            ////////////////////////////////////////////////////////////
            // available
            ////////////////////////////////////////////////////////////
            bool available;
         };


         ////////////////////////////////////////////////////////////
         // class Decision
         ////////////////////////////////////////////////////////////
         class Decision
         {
         public:
            enum event_type
            {
               event_unknown = 0,
               event_resource_chosen = 1,
               event_dialing_succeeded = 2,
               event_dialing_failed = 3
            };
            
         private:
            ////////////////////////////////////////////////////////////
            // event
            ////////////////////////////////////////////////////////////
            event_type event;

            ////////////////////////////////////////////////////////////
            // resource_name
            ////////////////////////////////////////////////////////////
            StrAsc resource_name;

            ////////////////////////////////////////////////////////////
            // time
            ////////////////////////////////////////////////////////////
            Csi::LgrDate time;

            ////////////////////////////////////////////////////////////
            // error_rate
            ////////////////////////////////////////////////////////////
            double error_rate;

            ////////////////////////////////////////////////////////////
            // skipped_count
            ////////////////////////////////////////////////////////////
            uint4 skipped_count;

            ////////////////////////////////////////////////////////////
            // available
            ////////////////////////////////////////////////////////////
            bool available;

            ////////////////////////////////////////////////////////////
            // child_name
            ////////////////////////////////////////////////////////////
            StrUni child_name;

         public:
            ////////////////////////////////////////////////////////////
            // default constructor
            ////////////////////////////////////////////////////////////
            Decision():
               event(event_unknown),
               error_rate(0),
               skipped_count(0),
               available(false)
            { }

            ////////////////////////////////////////////////////////////
            // copy constructor
            ////////////////////////////////////////////////////////////
            Decision(Decision const &other):
               event(other.event),
               time(other.time),
               resource_name(other.resource_name),
               error_rate(other.error_rate),
               skipped_count(other.skipped_count),
               available(other.available),
               child_name(other.child_name)
            { } 

            ////////////////////////////////////////////////////////////
            // copy operator
            ////////////////////////////////////////////////////////////
            Decision &operator =(Decision const &other)
            {
               event = other.event;
               resource_name = other.resource_name;
               time = other.time;
               error_rate = other.error_rate;
               skipped_count = other.skipped_count;
               available = other.available;
               child_name = other.child_name;
               return *this;
            }

            ////////////////////////////////////////////////////////////
            // get_event
            ////////////////////////////////////////////////////////////
            event_type get_event() const
            { return event; }

            ////////////////////////////////////////////////////////////
            // get_time
            ////////////////////////////////////////////////////////////
            Csi::LgrDate const &get_time() const
            { return time; }

            ////////////////////////////////////////////////////////////
            // get_resource_name
            ////////////////////////////////////////////////////////////
            StrAsc const &get_resource_name() const
            { return resource_name; }

            ////////////////////////////////////////////////////////////
            // get_error_rate
            ////////////////////////////////////////////////////////////
            double get_error_rate() const
            { return error_rate; }

            ////////////////////////////////////////////////////////////
            // get_skipped_count
            ////////////////////////////////////////////////////////////
            uint4 get_skipped_count() const
            { return skipped_count; }

            ////////////////////////////////////////////////////////////
            // get_available
            ////////////////////////////////////////////////////////////
            bool get_available() const
            { return available; }

            ////////////////////////////////////////////////////////////
            // get_child_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_child_name() const
            { return child_name; }
            
            ////////////////////////////////////////////////////////////
            // read
            ////////////////////////////////////////////////////////////
            void read(Csi::Messaging::Message *in);
         };
      };
      
      
      ////////////////////////////////////////////////////////////
      // class PoolMonitorClient
      ////////////////////////////////////////////////////////////
      class PoolMonitorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_logon,
            failure_session,
            failure_invalid_device_name,
            failure_unsupported,
            failure_security,
            failure_device_shut_down
         };
         virtual void on_failure(PoolMonitor *monitor, failure_type failure) = 0;
         
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when all initial messages have been handled and the monitor
         // in now in a steady state.
         ////////////////////////////////////////////////////////////
         virtual void on_started(PoolMonitor *monitor)
         { }

         ////////////////////////////////////////////////////////////
         // on_resource_added
         //
         // Called when a resource has been added to the pool.
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<PoolMonitorHelpers::Resource> resource_handle;
         virtual void on_resource_added(
            PoolMonitor *monitor, resource_handle &resource) 
         { }

         ////////////////////////////////////////////////////////////
         // on_resource_removed
         ////////////////////////////////////////////////////////////
         virtual void on_resource_removed(
            PoolMonitor *monitor, resource_handle &resource)
         { }

         ////////////////////////////////////////////////////////////
         // on_resource_changed
         ////////////////////////////////////////////////////////////
         virtual void on_resource_changed(
            PoolMonitor *monitor, resource_handle &resource)
         { }

         ////////////////////////////////////////////////////////////
         // on_decision
         ////////////////////////////////////////////////////////////
         typedef PoolMonitorHelpers::Decision decision_type;
         virtual void on_decision(
            PoolMonitor *monitor, decision_type const &decision)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class PoolMonitor
      //
      // Defines a component that can be used to monitor that status of a
      // LoggerNet pool type device.  This object will maintain a collection of
      // objects that will represent the status of the resources used for that
      // pool.
      //
      // In order to use this component, the application must provide an object
      // derived from class PoolMonitorClient.  It should then create an
      // instance of this class, call appropriate methods such as
      // set_device_name(), and then invoke one of the two different versions
      // of start().  The component will invoke various methods of the client
      // when key events (resource added, resource removed, resource info
      // changed) occur and also will call the client's on_started() method
      // when the monitor is up to date after being started.
      ////////////////////////////////////////////////////////////
      class PoolMonitor:
         public DeviceBase,
         public Csi::EventReceiver
      {
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         PoolMonitorClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_inactive,
            state_delegate,
            state_starting,
            state_started
         } state;

         ////////////////////////////////////////////////////////////
         // transaction_no
         ////////////////////////////////////////////////////////////
         uint4 transaction_no;

         ////////////////////////////////////////////////////////////
         // decision_past_interval
         ////////////////////////////////////////////////////////////
         uint4 decision_past_interval;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         PoolMonitor():
            client(0),
            state(state_inactive),
            decision_past_interval(0)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~PoolMonitor()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // get_decision_past_interval
         ////////////////////////////////////////////////////////////
         uint4 get_decision_past_interval() const
         { return decision_past_interval; }

         ////////////////////////////////////////////////////////////
         // set_decision_past_interval
         ////////////////////////////////////////////////////////////
         void set_decision_past_interval(uint4 interval)
         {
            if(state != state_inactive)
               throw exc_invalid_state();
            decision_past_interval = interval;
         }
         
         ////////////////////////////////////////////////////////////
         // start (new router)
         ////////////////////////////////////////////////////////////
         typedef PoolMonitorClient client_type;
         void start(client_type *client_, router_handle &router)
         {
            if(state != state_inactive)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }

         ////////////////////////////////////////////////////////////
         // start (existing component)
         ////////////////////////////////////////////////////////////
         void start(client_type *client_, ClientBase *other)
         {
            if(state != state_inactive)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(other);
         }

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         {
            client = 0;
            state = state_inactive;
            resources.clear();
            DeviceBase::finish();
         }

         // @group: definitions to act as a collection of resources

         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef PoolMonitorHelpers::Resource resource_type;
         typedef Csi::SharedPtr<resource_type> resource_handle;
         typedef std::map<StrAsc, resource_handle> resources_type;
         typedef resources_type::const_iterator const_iterator;
         const_iterator begin() const
         { return resources.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         const_iterator end() const
         { return resources.end(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return resources.empty(); }

         ////////////////////////////////////////////////////////////
         // find
         ////////////////////////////////////////////////////////////
         const_iterator find(StrAsc const &name) const
         { return resources.find(name); }
         
         // @endgroup:

         ////////////////////////////////////////////////////////////
         // describe_failure
         ////////////////////////////////////////////////////////////
         static void describe_failure(
            std::ostream &out, client_type::failure_type failure);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      protected:
         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(devicebase_failure_type failure);

      private:
         ////////////////////////////////////////////////////////////
         // resources
         ////////////////////////////////////////////////////////////
         resources_type resources;
      };
   };
};


#endif

