/* Cora.LgrNet.PooledResourcesMonitor.h

   Copyright (C) 2011, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Friday 04 February 2011
   Last Change: Friday 04 February 2011
   Last Commit: $Date: 2011-02-25 14:20:55 -0600 (Fri, 25 Feb 2011) $
   Last Changed by: $Author: tmecham $

*/

#ifndef Cora_LgrNet_PooledResourcesMonitor_h
#define Cora_LgrNet_PooledResourcesMonitor_h

#include "Cora.ClientBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace LgrNet
   {
      // @group: class forward declarations

      class PooledResourcesMonitor;
      
      // @endgroup:


      namespace PooledResourcesMonitorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class Resource
         ////////////////////////////////////////////////////////////
         class Resource
         {
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
            // available
            ////////////////////////////////////////////////////////////
            bool available;

            ////////////////////////////////////////////////////////////
            // use_percent
            ////////////////////////////////////////////////////////////
            float use_percent;

            ///////////////////////////////////////////////////////////
            // target_device
            ///////////////////////////////////////////////////////////
            StrUni target_device;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            Resource(StrAsc const &name_):
               error_rate(0),
               available(false),
               use_percent(0),
               name(name_)
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
            // get_available
            ////////////////////////////////////////////////////////////
            bool get_available() const
            { return available; }

            ////////////////////////////////////////////////////////////
            // get_use_percent
            ////////////////////////////////////////////////////////////
            float get_use_percent() const
            { return use_percent; }

            ///////////////////////////////////////////////////////////
            // get_target_device
            ///////////////////////////////////////////////////////////
            StrUni const &get_target_device() const
            { return target_device; }

            ////////////////////////////////////////////////////////////
            // read
            ////////////////////////////////////////////////////////////
            void read(Csi::Messaging::Message &in);
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class PooledResourcesMonitorClient
      ////////////////////////////////////////////////////////////
      class PooledResourcesMonitorClient: public Csi::InstanceValidator
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
            failure_unsupported,
            failure_security
         };
         virtual void on_failure(
            PooledResourcesMonitor *monitor, failure_type reason) = 0;

         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            PooledResourcesMonitor *monitor)
         { }

         ////////////////////////////////////////////////////////////
         // on_resource_added
         ////////////////////////////////////////////////////////////
         typedef PooledResourcesMonitorHelpers::Resource resource_type;
         typedef Csi::SharedPtr<resource_type> resource_handle;
         virtual void on_resource_added(
            PooledResourcesMonitor *monitor, resource_handle &resource)
         { }

         ////////////////////////////////////////////////////////////
         // on_resource_changed
         ////////////////////////////////////////////////////////////
         virtual void on_resource_changed(
            PooledResourcesMonitor *monitor, resource_handle &resource)
         { }

         ////////////////////////////////////////////////////////////
         // on_resource_removed
         ////////////////////////////////////////////////////////////
         virtual void on_resource_removed(
            PooledResourcesMonitor *monitor, resource_handle &resource)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class PooledResourcesMonitor
      //
      // Defines a component that can be used to monitor the status of pool
      // resources for a LoggerNet server.  In order to use this component, an
      // application must provide a client object derived from class
      // PooledResourcesMonitorClient.  It must then create an instance of
      // class PooledResourcesMonitor and, after setting appropropriate
      // properties, must invoke one of the two versions of the start()
      // method.
      //
      // As the component receives notications of pool resources, it will
      // invoke the client's on_resource_added() method.  When all existing
      // resources have been described, the client's on_started() method will
      // be invoked.  Thereafter, the component will invoke the client's
      // on_resource_changed() method when new statistics have been received
      // for a resource and  will invoke the client's on_resource_removed()
      // method when notice is received from the server that the pool resource
      // is no longer being used.   The client's on_failure() method will be
      // called at any time if the transaction cannot be established or if it
      // fails.
      ////////////////////////////////////////////////////////////
      class PooledResourcesMonitor:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         PooledResourcesMonitorClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_inactive,
            state_delegate,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // transaction_no
         ////////////////////////////////////////////////////////////
         uint4 transaction_no;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         PooledResourcesMonitor():
            client(0),
            state(state_inactive),
            transaction_no(0)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~PooledResourcesMonitor()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // start (new router)
         ////////////////////////////////////////////////////////////
         typedef PooledResourcesMonitorClient client_type;
         void start(
            client_type *client_, router_handle router)
         {
            if(state != state_inactive)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            state = state_delegate;
            client = client_;
            ClientBase::start(router);
         }

         ////////////////////////////////////////////////////////////
         // start (from existing component)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_, ClientBase *other)
         {
            if(state != state_inactive)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            state = state_delegate;
            client = client_;
            ClientBase::start(other);
         }

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish()
         {
            client = 0;
            state = state_inactive;
            resources.clear();
            transaction_no = 0;
         }

         // @group: resources container definitions

         ////////////////////////////////////////////////////////////
         // describe_failure
         ////////////////////////////////////////////////////////////
         static void describe_failure(
            std::ostream &out,  client_type::failure_type failure);
         
         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef PooledResourcesMonitorHelpers::Resource resource_type;
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
         // size
         ////////////////////////////////////////////////////////////
         typedef resources_type::size_type size_type;
         size_type size() const
         { return resources.size(); }

         ////////////////////////////////////////////////////////////
         // find
         ////////////////////////////////////////////////////////////
         const_iterator find(StrAsc const &resource_name) const
         { return resources.find(resource_name); }
         
         // @endgroup:

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
      protected:
         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);
         
         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);
         
      private:
         ////////////////////////////////////////////////////////////
         // resources
         ////////////////////////////////////////////////////////////
         resources_type resources;
      };
   };
};


#endif
