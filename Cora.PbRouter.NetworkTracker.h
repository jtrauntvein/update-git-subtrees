/* Cora.PbRouter.NetworkTracker.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 27 February 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_PbRouter_NetworkTracker_h
#define Cora_PbRouter_NetworkTracker_h

#include "Cora.LgrNet.DeviceMapper.h"
#include "Cora.PbRouter.NodesLister.h"
#include "Cora.PbRouter.LinksEnumerator.h"
#include "Cora.Device.SettingsEnumerator.h"
#include "OneShot.h"


namespace Cora
{
   namespace PbRouter
   {
      //@group class forward declarations
      class NetworkTracker;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class NetworkTrackerClient
      //
      // Defines a base class for objects that can act as clients to the
      // NetworkTracker component.
      ////////////////////////////////////////////////////////////
      class NetworkTrackerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            NetworkTracker *tracker)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_connection_failed = 1,
            failure_invalid_logon = 2,
            failure_server_security_blocked = 3,
            failure_invalid_router_id = 4,
            failure_server_session_failed = 5,
            failure_unsupported = 6,
         };
         virtual void on_failure(
            NetworkTracker *tracker,
            failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_node_added
         //
         // Called when a node has been added to the list known by this
         // tracker. 
         ////////////////////////////////////////////////////////////
         class node_info
         {
         public:
            ////////////////////////////////////////////////////////////
            // pakbus_address
            //
            // The address of this node on the network
            ////////////////////////////////////////////////////////////
            uint2 pakbus_address;

            ////////////////////////////////////////////////////////////
            // is_router
            //
            // Set to true if this node is a router.
            ////////////////////////////////////////////////////////////
            bool is_router;

            ////////////////////////////////////////////////////////////
            // is_server
            //
            // Set to true if this node represents the server
            ////////////////////////////////////////////////////////////
            bool is_server;
            
            ////////////////////////////////////////////////////////////
            // device_info
            //
            // Fills in information on this device from the network map.  Will
            // be a null pointer if node does not have a representation in the
            // network map. 
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<LgrNet::DeviceMapperHelpers::Device> device_info;

            ////////////////////////////////////////////////////////////
            // default constructor
            ////////////////////////////////////////////////////////////
            node_info():
               pakbus_address(0),
               is_router(false),
               is_server(false)
            { }

            ////////////////////////////////////////////////////////////
            // copy constructor
            ////////////////////////////////////////////////////////////
            node_info(node_info const &other):
               pakbus_address(other.pakbus_address),
               is_router(other.is_router),
               is_server(other.is_server),
               device_info(other.device_info)
            { }

            ////////////////////////////////////////////////////////////
            // copy operator
            ////////////////////////////////////////////////////////////
            node_info &operator =(node_info const &other)
            {
               pakbus_address = other.pakbus_address;
               is_router = other.is_router;
               is_server = other.is_server;
               device_info = other.device_info;
               return *this;
            }

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            node_info(
               uint2 pakbus_address_,
               bool is_router_,
               bool is_server_):
               pakbus_address(pakbus_address_),
               is_router(is_router_),
               is_server(is_server_)
            { }
               
         };
         virtual void on_node_added(
            NetworkTracker *tracker,
            node_info &info)
         { }

         ////////////////////////////////////////////////////////////
         // on_node_deleted
         //
         // Called when a node has been deleted from the PakBus network. 
         ////////////////////////////////////////////////////////////
         virtual void on_node_deleted(
            NetworkTracker *tracker,
            node_info &info)
         { }

         ////////////////////////////////////////////////////////////
         // on_node_changed
         //
         // Called when part of the information about the specified
         // node has changed (such as its name, whether it's a router,
         // etc.)
         ////////////////////////////////////////////////////////////
         virtual void on_node_changed(
            NetworkTracker *tracker,
            node_info &info)
         { }

         ////////////////////////////////////////////////////////////
         // on_link_added
         //
         // Called when a link has been added between nodes.  The link response
         // time and a flag indicating whether the link is static are provided
         // as parameters along with references to the two nodes affected.
         //////////////////////////////////////////////////////////// 
         virtual void on_link_added(
            NetworkTracker *tracker,
            node_info &info1,
            node_info &info2,
            uint4 link_response_time,
            bool is_static)
         { }

         ////////////////////////////////////////////////////////////
         // on_link_deleted
         //
         // Called when a link deletion has occurred.  
         ////////////////////////////////////////////////////////////
         virtual void on_link_deleted(
            NetworkTracker *tracker,
            node_info &node1,
            node_info &node2)
         { }

         ////////////////////////////////////////////////////////////
         // on_link_changed
         //
         // Called when a new response time for an existing link has been
         // reported by the router. 
         ////////////////////////////////////////////////////////////
         virtual void on_link_changed(
            NetworkTracker *tracker,
            node_info &node1,
            node_info &node2,
            uint4 link_response_time,
            bool is_static)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class NetworkTracker
      //
      // Defines a component that tracks the nodes and links on a PakBus
      // network for the application that uses it.  This component will combine
      // information derived from three different server transactions to give a
      // set of notifications as well as methods for enumerating routes and
      // links.
      //
      // In order to use this component, an application must provide a client
      // object derived from class NetworkTrackerClient (typedefed as
      // client_type within this class).  It should then create an instance of
      // this class, call the appropriate methods to set attributes (such as
      // set_pakbus_router_id()), and then invoke one of the two versions of
      // start().  Once start() has been invoked, this component will start a
      // network map enumeration transaction as well as a PakBus Links
      // enumeration transaction with the server.  It will also start a list
      // nodes transaction when a potential change occurs (such as the addition
      // or removal of a PakBus device in the network map).  The client's
      // various notification methods will be invoked at various times as this
      // initial process takes place.  Once all of the initial information has
      // been loaded, the client's on_started() method will be invoked.
      // Thereafter, the client will be kept abreast of any changes that take
      // place in the information already reported.
      //
      // At any time while the component is in a started state, the application
      // can enumerate both nodes and links using nodes_begin()/nodes_end() and
      // links_begin()/links_end().
      //
      // The application can return this component to a standby state by
      // calling finish().  Calling finish() will also clear any network
      // information that the component may have cached.  It can also stop the
      // server transactions and clean up any additional resource used by this
      // component by deleting the component.  If, at any time, the component
      // cannot maintain the server transactions, it will invoke the client's
      // on_failure() method and return to a standby state as if finish() had
      // been invoked.
      ////////////////////////////////////////////////////////////
      class NetworkTracker:
         public PbRouterBase,
         public LinksEnumeratorClient,
         public NodesListerClient,
         public LgrNet::DeviceMapperClient,
         public Device::SettingsEnumeratorClient,
         public Csi::EventReceiver,
         public OneShotClient
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         NetworkTracker();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~NetworkTracker();

         ////////////////////////////////////////////////////////////
         // set_one_shot
         //
         // Sets the one shot timer object used by this component to keep track
         // of the refresh timer.  If not called before start() is called, the
         // timer will be allocated for this component.
         ////////////////////////////////////////////////////////////
         void set_one_shot(Csi::SharedPtr<OneShot> one_shot_);

         ////////////////////////////////////////////////////////////
         // get_one_shot
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<OneShot> &get_one_shot()
         { return one_shot; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef NetworkTrackerClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         //@group nodes enumeration methods
         ////////////////////////////////////////////////////////////
         // nodes_begin
         //////////////////////////////////////////////////////////// 
         typedef client_type::node_info node_type;
         typedef std::map<uint2, node_type> nodes_type;
         typedef nodes_type::const_iterator nodes_iterator;
         nodes_iterator nodes_begin() const
         { return nodes.begin(); }

         ////////////////////////////////////////////////////////////
         // nodes_end
         ////////////////////////////////////////////////////////////
         nodes_iterator nodes_end() const
         { return nodes.end(); }

         ////////////////////////////////////////////////////////////
         // nodes_empty
         ////////////////////////////////////////////////////////////
         bool nodes_empty() const
         { return nodes.empty(); }

         ////////////////////////////////////////////////////////////
         // nodes_size
         ////////////////////////////////////////////////////////////
         nodes_type::size_type nodes_size() const
         { return nodes.size(); }

         ////////////////////////////////////////////////////////////
         // find_node
         //
         // Looks up the node info for the specified address and copies the
         // information into the provided buffer.  Will return false if the
         // specified address does not exist.
         ////////////////////////////////////////////////////////////
         bool find_node(node_type &node, uint2 address); 
         //@endgroup

         //@group links enumeration methods
         ////////////////////////////////////////////////////////////
         // links_begin
         ////////////////////////////////////////////////////////////
         struct link_type
         {
            uint2 address1, address2;
            uint4 response_time_msec;
            bool is_static;
         };
         typedef std::list<link_type> links_type;
         typedef links_type::const_iterator links_iterator;
         links_iterator links_begin() const
         { return links.begin(); }

         ////////////////////////////////////////////////////////////
         // links_end
         ////////////////////////////////////////////////////////////
         links_iterator links_end() const
         { return links.end(); }

         ////////////////////////////////////////////////////////////
         // links_size
         ////////////////////////////////////////////////////////////
         links_type::size_type links_size() const
         { return links.size(); }

         ////////////////////////////////////////////////////////////
         // links_empty
         ////////////////////////////////////////////////////////////
         bool links_empty() const
         { return links.empty(); }
         //@endgroup

      protected:
         ////////////////////////////////////////////////////////////
         // nodes
         //
         // Keeps track of all nodes that are known on the network.
         ////////////////////////////////////////////////////////////
         nodes_type nodes;

         ////////////////////////////////////////////////////////////
         // links
         //
         // Keeps track of all links between nodes in the network.
         ////////////////////////////////////////////////////////////
         links_type links;

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_ready();

         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_failure(pbrouterbase_failure_type failure);

         //@group class LinksEnumeratorClient derived methods
         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(LinksEnumerator *tran);

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            LinksEnumerator *tran,
            LinksEnumeratorClient::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_link_added
         ////////////////////////////////////////////////////////////
         virtual void on_link_added(
            LinksEnumerator *tran,
            uint2 node1_address,
            uint4 node1_net_map_id,
            bool node1_is_router,
            uint2 node2_address,
            uint4 node2_net_map_id,
            bool node2_is_router,
            uint4 worst_case_resp_time);

         ////////////////////////////////////////////////////////////
         // on_link_deleted
         ////////////////////////////////////////////////////////////
         virtual void on_link_deleted(
            LinksEnumerator *tran,
            uint2 node1_address,
            uint4 node1_net_map_id,
            uint2 node2_address,
            uint4 node2_net_map_id);

         ////////////////////////////////////////////////////////////
         // on_link_changed
         ////////////////////////////////////////////////////////////
         virtual void on_link_changed(
            LinksEnumerator *tran,
            uint2 node1_address,
            uint4 node1_net_map_id,
            bool node1_is_router,
            uint2 node2_address,
            uint4 node2_net_map_id,
            bool node2_is_router,
            uint4 worst_case_resp_time);
         //@endgroup

         //@group class NodesListerClient derived methods
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         virtual void on_complete(
            NodesLister *lister,
            outcome_type outcome,
            NodesListerClient::nodes_list_type &nodes_list);
         //@endgroup

         //@group class DeviceMapperClient derived methods
         ////////////////////////////////////////////////////////////
         // on_change_complete
         ////////////////////////////////////////////////////////////
         virtual void on_change_complete(
            LgrNet::DeviceMapper *mapper);

         ////////////////////////////////////////////////////////////
         // on_device_added
         ////////////////////////////////////////////////////////////
         virtual void on_device_added(
            LgrNet::DeviceMapper *mapper,
            device_handle &device);

         ////////////////////////////////////////////////////////////
         // on_device_deleted
         ////////////////////////////////////////////////////////////
         virtual void on_device_deleted(
            LgrNet::DeviceMapper *mapper,
            device_handle &device);

         ////////////////////////////////////////////////////////////
         // on_device_renamed
         ////////////////////////////////////////////////////////////
         virtual void on_device_renamed(
            LgrNet::DeviceMapper *mapper,
            device_handle &device,
            StrUni const &old_name);

         ////////////////////////////////////////////////////////////
         // on_device_parent_change
         ////////////////////////////////////////////////////////////
         virtual void on_device_parent_change(
            LgrNet::DeviceMapper *mapper,
            device_handle &device,
            uint4 old_parent_id);

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            LgrNet::DeviceMapper *mapper,
            DeviceMapperClient::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            Device::SettingsEnumerator *enumerator,
            SettingsEnumeratorClient::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_setting_changed
         ////////////////////////////////////////////////////////////
         virtual void on_setting_changed(
            Device::SettingsEnumerator *enumerator,
            Csi::SharedPtr<Setting> &setting);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // onOneShotFired
         ////////////////////////////////////////////////////////////
         virtual void onOneShotFired(uint4 event_id);
         
      private:
         ////////////////////////////////////////////////////////////
         // add_node
         ////////////////////////////////////////////////////////////
         nodes_type::iterator add_node(
            uint2 pakbus_address,
            bool is_router,
            uint4 object_id = 0,
            wchar_t const *device_name = 0);

         ////////////////////////////////////////////////////////////
         // remove_node
         ////////////////////////////////////////////////////////////
         void remove_node(nodes_type::iterator ni);

         ////////////////////////////////////////////////////////////
         // update_node
         ////////////////////////////////////////////////////////////
         nodes_type::iterator update_node(
            uint2 pakbus_address,
            bool is_router,
            uint4 object_id);

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
            state_before_active,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // links_lister
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<LinksEnumerator> links_lister;

         ////////////////////////////////////////////////////////////
         // nodes_lister
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<NodesLister> nodes_lister;

         ////////////////////////////////////////////////////////////
         // device_mapper
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<LgrNet::DeviceMapper> device_mapper;

         ////////////////////////////////////////////////////////////
         // settings
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Device::SettingsEnumerator> setting_handle;
         typedef std::map<uint4, setting_handle> settings_type;
         settings_type settings;

         ////////////////////////////////////////////////////////////
         // refresh_timer
         ////////////////////////////////////////////////////////////
         uint4 refresh_timer;

         ////////////////////////////////////////////////////////////
         // one_shot
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<OneShot> one_shot;
      };
   }; 
};


#endif
