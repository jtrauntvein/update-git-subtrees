/* Csi.PakBus.Router.h

   Copyright (C) 2001, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 01 March 2001
   Last Change: Monday 25 June 2018
   Last Commit: $Date: 2018-06-25 14:06:55 -0600 (Mon, 25 Jun 2018) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Csi_PakBus_Router_h
#define Csi_PakBus_Router_h

#include "Csi.PakBus.Defs.h"
#include "Csi.PakBus.PakCtrlMessage.h"
#include "Csi.PakBus.Bmp5Message.h"
#include "Csi.PakBus.RouterHelpers.h"
#include "Csi.PakBus.CipherBase.h"
#include "Csi.PolySharedPtr.h"
#include "Csi.OrderedList.h"
#include "OneShot.h"
#include <list>
#include <map>


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class Router
      //
      // Defines a base class that implements the PakBus networking layer routing protocols and
      // implements the PakBus application interface. An application would need to derive from this
      // class in order process its own messages. Specifically, a derived class would need to
      // overload on_message() in order to process messages coming from the PakBus network.
      //
      // This class also provides a facility for managing PakBus transactions for the application
      // that uses this router. An application can create new transaction objects that can be used
      // to send messages to a specified destination.  These same transaction objects will receive
      // notices when incoming messages specifying that destination and transaction number arrive as
      // well as failure notifications.
      ////////////////////////////////////////////////////////////
      class Router: public OneShotClient
      {
      public:
         ////////////////////////////////////////////////////////////
         // max_hop_count
         //
         // Records the maximum hop count allowed
         ////////////////////////////////////////////////////////////
         static byte const max_hop_count;

         ////////////////////////////////////////////////////////////
         // broadcast_address
         //
         // Records the specific node address that is identified in PakBus convention as the
         // broadcast address. Messages that have this address in their destination field will be
         // accepted by any listening neighbour.
         ////////////////////////////////////////////////////////////
         static uint2 const broadcast_address;


         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<OneShot> timer_handle;
         Router(timer_handle timer_);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Router();

         ////////////////////////////////////////////////////////////
         // shut_down
         ////////////////////////////////////////////////////////////
         virtual void shut_down();
         

         ////////////////////////////////////////////////////////////
         // register_port
         //
         // This method should be called by the application to register a PakBus port object with
         // this router. The act of registration is an indication of readiness to receive packets
         // and to deliver incoming packets. 
         ////////////////////////////////////////////////////////////
         void register_port(PortBase *port);

         ////////////////////////////////////////////////////////////
         // unregister_port
         //
         // This method should be called by the application at a point when the point object is no
         // longer considered a valid part of the PakBus network.
         ////////////////////////////////////////////////////////////
         void unregister_port(PortBase *port);

         ////////////////////////////////////////////////////////////
         // this_node_address access methods
         //
         // These methods allow the this_node_address member to be read and changed. It should be
         // noted that calling set_this_node_address() can be disruptive to communications in the
         // network for some time to follow.
         ////////////////////////////////////////////////////////////
         uint2 get_this_node_address() const;
         virtual void set_this_node_address(uint2 this_node_address);

         ////////////////////////////////////////////////////////////
         // on_port_message
         //
         // This method should be invoked by port objects when they have a PakBus message that
         // should be processed by this node. This method will be responsible for deciding if the
         // message should be forwarded to its destination or if it should be processed by this
         // node. It will also be responsible for maintaining the routing table.
         //
         // If the message is decided to be for this node to process, the on_message() method will
         // be invoked. The derived class should overload on_message() in order to introduce its
         // specific processing on that message.
         ////////////////////////////////////////////////////////////
         typedef Message message_type;
         typedef Csi::SharedPtr<message_type> message_handle;
         virtual void on_port_message(
            PortBase *port,
            message_handle &message);

         ////////////////////////////////////////////////////////////
         // send_message_from_app
         //
         // Prepares the message to be routed from an application attached to this router by setting
         // the appropriate source and passing the message on to route_message()
         ////////////////////////////////////////////////////////////
         bool send_message_from_app(message_handle &message);

         ////////////////////////////////////////////////////////////
         // cancel_message_from_app
         //
         // Removes the specified message from the router's queue and sends notification to the port
         // that the delivery request has been cancelled. Returns true if the operation succeeds or
         // false if the message could not be found in the queue.
         ////////////////////////////////////////////////////////////
         bool cancel_message_from_app(message_handle &message);

         ////////////////////////////////////////////////////////////
         // send_delivery_fault_message
         //
         // Sends a PakBus packet delivery fault message to the source node associated with the
         // specified message.
         ////////////////////////////////////////////////////////////
         typedef PakCtrl::DeliveryFailure::failure_type failure_type;
         void send_delivery_fault_message(
            message_handle &message,
            failure_type error_code);

         ////////////////////////////////////////////////////////////
         // get_next_port_message
         //
         // Scans the list of unrouted messages for any message that should be sent to the specified
         // port. Most of the routing algorithm will be contained within this method. If a message
         // is found, its reference will be copied to the message parameter and the return value
         // will be true. If no message is found, the the return value will be false and the message
         // parameter will be cleared.
         ////////////////////////////////////////////////////////////
         bool get_next_port_message(
            PortBase *port,
            uint2 physical_destination,
            message_handle &message);

         ////////////////////////////////////////////////////////////
         // is_route_reachable
         //
         // Returns true if there is a route recorded for the specified node address.
         ////////////////////////////////////////////////////////////
         bool is_route_reachable(uint2 node_address) const;

         ////////////////////////////////////////////////////////////
         // on_port_delivery_failure
         //
         // Called when a port is unable to ring up its peer to deliver messages. This could result
         // in sending a failure message to the source node (if different from this node) or it will
         // result in a failure being reported to application for any packets that would be routed
         // to the port.
         ////////////////////////////////////////////////////////////
         virtual void on_port_delivery_failure(
            PortBase *port,
            uint2 physical_destination = 0);

         ////////////////////////////////////////////////////////////
         // add_static_route
         //
         // Creates (or modifies an existing) static route. The set of routes created using this
         // method will be consulted when a message cannot be routed through the information in the
         // normal routing table. The route can be removed by invoking remove_static_route() with
         // the same destination identifier.
         ////////////////////////////////////////////////////////////
         void add_static_route(
            uint2 destination,
            PortBase *port,
            uint2 physical_destination,
            byte hop_count);

         ////////////////////////////////////////////////////////////
         // remove_static_route
         //
         // Removes only the static route associated with the destination address.
         ////////////////////////////////////////////////////////////
         void remove_static_route(uint2 destination);

         ////////////////////////////////////////////////////////////
         // get_route_response_time
         //
         // Returns the expected response time for the specified route in milli-seconds.
         ////////////////////////////////////////////////////////////
         uint4 get_route_response_time(uint2 destination);

         //@group transaction management methods
         ////////////////////////////////////////////////////////////
         // open_transaction
         //
         // Adds the specified transaction handle to the list managed by this router.  The
         // transaction object will be assigned a new identifier which will also be the return value
         // from this function. 
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<PakBusTran> transaction_handle;
         byte open_transaction(transaction_handle transaction);

         ////////////////////////////////////////////////////////////
         // reassign_transaction_id
         //
         // Assigns the transaction specified by the transaction id a new transaction identifier and
         // returns that new identifier.  
         ////////////////////////////////////////////////////////////
         byte reassign_transaction_id(
            uint2 destination,
            byte old_transaction_id);

         ////////////////////////////////////////////////////////////
         // close_transaction
         //
         // Removes the specified transaction from the list managed by this router.
         ////////////////////////////////////////////////////////////
         void close_transaction(
            uint2 destination,
            byte transaction_id);

         ////////////////////////////////////////////////////////////
         // close_all_transactions_for_node
         //
         // Closes any open transaction for the specified destination address
         // including the transaction that may currently have focus.  
         ////////////////////////////////////////////////////////////
         void close_all_transactions_for_node(uint2 destination_address);

         ////////////////////////////////////////////////////////////
         // on_transaction_closed
         ////////////////////////////////////////////////////////////
         virtual void on_transaction_closed(
            transaction_handle &transaction)
         { }

         ////////////////////////////////////////////////////////////
         // request_transaction_focus
         //
         // Enters the specified transaction into a prioritised queue of objects that are competing
         // for focus.  When the focus is obtained, the transaction object's on_focus_start() method
         // will be invoked.   This focus will be lost when the release_transaction_focus() method
         // is called or when the transaction is removed via close_transaction().
         //
         // Preferential treatment is given to transactions with nodes where there is already a
         // session established.  
         ////////////////////////////////////////////////////////////
         void request_transaction_focus(
            uint2 destination,
            byte transaction_id);

         ////////////////////////////////////////////////////////////
         // release_transaction_focus
         //
         // Releases the focus obtained or requested through request_transaction_focus().  If the
         // specified transaction does not currently have the focus, it will be removed from the
         // queue of waiting objects.  
         ////////////////////////////////////////////////////////////
         void release_transaction_focus(
            uint2 destination,
            byte transaction_id);

         ////////////////////////////////////////////////////////////
         // on_application_transaction_finished
         //
         // Called when the application is finished with the specified transaction.  This method
         // will call the transaction's on_application_finished() virtual method.
         ////////////////////////////////////////////////////////////
         void on_application_transaction_finished(
            uint2 destination,
            byte transaction_id);

         ////////////////////////////////////////////////////////////
         // find_transaction
         //
         // Attempts to look up the transaction referenced by the specified
         // address and transaction number.  If the specified transaction can
         // be found and is not in a closing state, its reference will be
         // copied into the provided dest handle and the return value will be
         // true.  Otherwise, the dest handle will be set to a null pointer and
         // the return value will be false.
         ////////////////////////////////////////////////////////////
         bool find_transaction(
            transaction_handle &dest,
            uint2 address,
            byte transaction_id);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // on_beacon
         //
         // Called by a port when a beacon message has been received.  A beacon is any packet that
         // the port picks up that has a both physical and logical sources whether that packet is
         // directed at this node or not.  The is_broadcast parameter indicates whether the received
         // packet was assigned a broadcast destination address and is provided so a random delay
         // can be introduced before responding to such a beacon with the hello command. 
         ////////////////////////////////////////////////////////////
         virtual void on_beacon(
            PortBase *port,
            uint2 physical_source,
            bool is_broadcast);

         ////////////////////////////////////////////////////////////
         // on_neighbour_lost
         //
         // Called communication with a neighbour has been lost.  This will remove the specified
         // neighbour from the list managed by this router and cause the routing tables to be
         // updated. 
         ////////////////////////////////////////////////////////////
         virtual void on_neighbour_lost(
            PortBase *port,
            uint2 physical_address);

         ////////////////////////////////////////////////////////////
         // on_neighbour_info
         //
         // Called when neighbour information has been received either because a transaction
         // received a hello acknowledgement or because a node sent us a hello command.
         ////////////////////////////////////////////////////////////
         virtual void on_neighbour_info(
            PortBase *port,
            uint2 physical_address,
            bool is_router,
            HopMetric hop_metric,
            uint2 beacon_interval);

         ////////////////////////////////////////////////////////////
         // on_router_neighbour_list
         //
         // Called when neighbour list information has been received from a remote router. 
         ////////////////////////////////////////////////////////////
         enum neighbour_list_update_type
         {
            neighbour_list_update_complete = 0,
            neighbour_list_update_add = 1,
            neighbour_list_update_remove = 2,
         };
         typedef PakCtrlMessage pakctrl_message_type;
         typedef Csi::PolySharedPtr<message_type, pakctrl_message_type> pakctrl_message_handle;
         virtual void on_router_neighbour_list(
            uint2 router_id,
            pakctrl_message_handle &message,
            neighbour_list_update_type change_code);

         ////////////////////////////////////////////////////////////
         // count_messages_for_port
         //
         // Returns the number of messages that are waiting for the specified
         // port and physical destination address.  If the specified
         // destination address is zero, all of the messages for the port will
         // be counted.
         ////////////////////////////////////////////////////////////
         uint4 count_messages_for_port(
            PortBase *port,
            uint2 physical_destination);

         /**
          * @return Returns the start iterator for the collection of unrouted messages.
          */
         class message_handle_gtr
         {
         public:
            bool operator ()(message_handle const &m1, message_handle const &m2)
            { return m1->get_priority() > m2->get_priority(); }
         };
         typedef OrderedList<message_handle, message_handle_gtr> unrouted_messages_type;
         typedef unrouted_messages_type::iterator unrouted_iterator;
         unrouted_iterator unrouted_begin()
         { return unrouted_messages.begin(); }

         /**
          * @return Returns the end iterator for the collection of unrouted messages.
          */
         unrouted_iterator unrouted_end()
         { return unrouted_messages.end(); }

         ////////////////////////////////////////////////////////////
         // port_is_needed
         //
         // Evaluates whether the specified port is needed either because of
         // waiting messages to send or because transactions are still pending
         // for the port.  Returns true if the port is still needed in an
         // on-line state. 
         ////////////////////////////////////////////////////////////
         bool port_is_needed(PortBase *port);

         ////////////////////////////////////////////////////////////
         // log_debug
         //
         // Should be overloaded if the application wants to make use of the
         // debug strings produced in association with the router.
         ////////////////////////////////////////////////////////////
         virtual void log_debug(
            char const *object_name,
            char const *log_message)
         { }

         ////////////////////////////////////////////////////////////
         // log_comms
         //
         // Should be overloaded if the application wants to make use of
         // communication log messages.
         ////////////////////////////////////////////////////////////
         enum comms_message_type
         {
            comms_status,
            comms_warning,
            comms_fault,
            comms_status_neutral
         };
         virtual void log_comms(
            comms_message_type severity,
            char const *message)
         { }

         ////////////////////////////////////////////////////////////
         // send_reset_command
         //
         // Sends a reset command to the specified address.
         ////////////////////////////////////////////////////////////
         void send_reset_command(
            uint2 destination,
            bool destructive);

         ////////////////////////////////////////////////////////////
         // get_current_port_priority
         //
         // Evaluates what priority a port should have based upon the set of
         // transactions associated with routes that use that port.  A value of
         // Priorities::low will be returned if there are no pending
         // transactions for the port. 
         ////////////////////////////////////////////////////////////
         typedef Priorities::priority_type priority_type;
         priority_type get_current_port_priority(PortBase *port);

         ////////////////////////////////////////////////////////////
         // on_port_allowed_neighbours_changed
         //
         // Called when the set of allowed neighbours for a port has been
         // changed.  This method will eliminate any routers (static or
         // learned), neighbours, routers, or routes that are dependent upon
         // links that have become banned.
         ////////////////////////////////////////////////////////////
         virtual void on_port_allowed_neighbours_changed(PortBase *port);

         ////////////////////////////////////////////////////////////
         // route_port_is_active
         //
         // Returns true if the port associated with the speciifed route is in
         // an active state.  
         ////////////////////////////////////////////////////////////
         bool route_port_is_active(uint2 dest_address);

         ////////////////////////////////////////////////////////////
         // route_port_should_cap_timeout
         //
         // Returns true if the port associated with the specified route should
         // cap the transaction timeout value at 35 seconds.  This is returned
         // based upon the return value of the port's should_cap_timeout()
         // method. 
         ////////////////////////////////////////////////////////////
         bool route_port_should_cap_timeout(uint2 address);

         ////////////////////////////////////////////////////////////
         // route_port_reset_session_timer
         ////////////////////////////////////////////////////////////
         void route_port_reset_session_timer(uint2 address);

         ////////////////////////////////////////////////////////////
         // set_is_leaf_node
         //
         // Has the dual effect of setting future behaviour for the router and
         // also resetting the router state.
         ////////////////////////////////////////////////////////////
         void set_is_leaf_node(bool is_leaf_node);

         ////////////////////////////////////////////////////////////
         // get_is_leaf_node
         ////////////////////////////////////////////////////////////
         bool get_is_leaf_node() const
         { return is_leaf_node; }

         ////////////////////////////////////////////////////////////
         // on_port_ready
         //
         // Called by the port object when it has entered an on-line state
         // after having been inactive.
         ////////////////////////////////////////////////////////////
         virtual void on_port_ready(PortBase *port);

         ////////////////////////////////////////////////////////////
         // reassign_port_messages
         //
         // Changes the port assignment for any messages that specify the port
         // (generally this is done for hello messages).
         ////////////////////////////////////////////////////////////
         void reassign_port_messages(PortBase *new_port, PortBase *old_port);

         typedef std::map<RouterHelpers::transaction_id, transaction_handle> transactions_type;
         
         ////////////////////////////////////////////////////////////
         // find_route
         //
         // Searches for the route entry associated with the specified node address. Returns a
         // pointer to that entry if the route can be found or returns a null pointer if no such
         // entry exists.
         ////////////////////////////////////////////////////////////
         RouterHelpers::route_type *find_route(uint2 node_id);

         ////////////////////////////////////////////////////////////
         // get_timer
         ////////////////////////////////////////////////////////////
         SharedPtr<OneShot> &get_timer()
         { return timer; }

         ////////////////////////////////////////////////////////////
         // set_cipher
         //
         // called by the application to set the cipher that will be used for
         // the specified PakBus address.  If the provided cipher is null, any
         // existing cipher will be removed.  If the cipher address is 4095 (or
         // greater), the specified cipher will be used as the default cipher. 
         ////////////////////////////////////////////////////////////
         typedef SharedPtr<CipherBase> cipher_handle;
         virtual void set_cipher(uint2 address, cipher_handle cipher);

         ////////////////////////////////////////////////////////////
         // clear_cipher
         ////////////////////////////////////////////////////////////
         virtual void clear_cipher(uint2 address)
         { set_cipher(address, cipher_handle()); }

         ////////////////////////////////////////////////////////////
         // should_encrypt_message
         //
         // Called by the router to determine whether a message for the
         // specified protocol code and message type should be encrypted. 
         ////////////////////////////////////////////////////////////
         virtual bool should_encrypt_message(message_handle &message);

         ////////////////////////////////////////////////////////////
         // get_max_body_len
         //
         // Calculates the maximum body length for BMP5 and PakCtrl  messages
         // to the specified destination.  This can depend upon the cipher
         // being used. 
         ////////////////////////////////////////////////////////////
         virtual uint4 get_max_body_len(uint2 destination) const;

         // @group: methods dealing with reporting router transactions

         ////////////////////////////////////////////////////////////
         // add_report
         //
         // can be overloaded to generate a report ID that can be associated with a PakBus
         // transaction.  
         ////////////////////////////////////////////////////////////
         virtual int8 add_report(
            StrAsc const &name,
            priority_type priority,
            StrAsc const &state = "")
         { return -1; }

         ////////////////////////////////////////////////////////////
         // set_report_state
         ////////////////////////////////////////////////////////////
         virtual void set_report_state(
            int8 report_id,
            StrAsc const &state)
         { }

         ////////////////////////////////////////////////////////////
         // set_report_transmit_time
         ////////////////////////////////////////////////////////////
         virtual void set_report_transmit_time(
            int8 report_id,
            LgrDate const &transmit_time)
         { }

         ////////////////////////////////////////////////////////////
         // set_report_receive_time
         ////////////////////////////////////////////////////////////
         virtual void set_report_receive_time(
            int8 report_id,
            LgrDate const &receive_time)
         { }

         ////////////////////////////////////////////////////////////
         // set_report_timeout
         ////////////////////////////////////////////////////////////
         virtual void set_report_timeout(
            int8 report_id,
            uint4 interval)
         { }

         ////////////////////////////////////////////////////////////
         // remove_report
         ////////////////////////////////////////////////////////////
         virtual void remove_report(int8 report_id)
         { }
            
         // @endgroup
         
      protected:
         ////////////////////////////////////////////////////////////
         // on_message
         //
         // Called by on_port_message() in order to process the incoming message. An application's
         // Router-derived class should overload this method in order to pass the message up to its
         // higher levels. It should delegate to this version all messages that it does not
         // support.
         //
         // The return value should be true if the message was processed. It should be false if the
         // message was not processed. A value of false will result in a delivery fault message
         // being sent.
         ////////////////////////////////////////////////////////////
         virtual bool on_message(message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_pakctrl_message
         //
         // Called when a PakBus PAKCTRL message has been received. This method should return a true
         // value if the message was processed. If the message was not processed, the return value
         // should be false.
         ////////////////////////////////////////////////////////////
         virtual bool on_pakctrl_message(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_bmp5_message
         //
         // Called when a PakBus BMP5 message has been received.  This method will return a value of
         // true if the message was handled and false if the message was not handled.
         ////////////////////////////////////////////////////////////
         typedef Csi::PolySharedPtr<Message, Bmp5Message> bmp5_message_handle;
         virtual bool on_bmp5_message(bmp5_message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_encrypted_message
         ////////////////////////////////////////////////////////////
         virtual bool on_encrypted_message(message_handle &message);
         
         ////////////////////////////////////////////////////////////
         // on_delivery_failure
         //
         // Called when a PakBus delivery failure message has been received. The failure parameter
         // will indicate the nature of the failure. The fragment parameter will be a reference to
         // the fragment of the original message that could not be delivered.
         ////////////////////////////////////////////////////////////
         virtual void on_delivery_failure(
            failure_type failure,
            message_handle &fragment);

         ////////////////////////////////////////////////////////////
         // on_hello_cmd
         ////////////////////////////////////////////////////////////
         virtual void on_hello_cmd(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_send_neighbour_list_cmd
         ////////////////////////////////////////////////////////////
         virtual void on_send_neighbour_list_cmd(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_get_neighbour_list_cmd
         ////////////////////////////////////////////////////////////
         virtual void on_get_neighbour_list_cmd(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_echo_cmd
         ////////////////////////////////////////////////////////////
         virtual void on_echo_cmd(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_reset_routes_cmd
         ////////////////////////////////////////////////////////////
         virtual void on_reset_routes_cmd(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_goodbye_cmd
         ////////////////////////////////////////////////////////////
         virtual void on_goodbye_cmd(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_hello_request_cmd
         ////////////////////////////////////////////////////////////
         virtual void on_hello_request_cmd(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_remote_echo_cmd
         ////////////////////////////////////////////////////////////
         virtual void on_remote_echo_cmd(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_list_port_names_cmd
         ////////////////////////////////////////////////////////////
         virtual void on_list_port_names_cmd(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_remote_hello_request_cmd
         ////////////////////////////////////////////////////////////
         virtual void on_remote_hello_request_cmd(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // route_message
         //
         // Routes the message to the port that is associated with the most favourable route. The
         // return value will be true if the message is routable. Otherwise, it will be false.
         ////////////////////////////////////////////////////////////
         bool route_message(message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_sending_message
         //
         // Called just before the specified message gets sent. This method is provided as a hook
         // that the higher levels can use to set watch dog timers, update message fields, etc.
         ////////////////////////////////////////////////////////////
         virtual void on_sending_message(message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_link_change
         //
         // Called when something has happened to change a link.  If overloaded, the derived class
         // should delegate to this method so that processing can be done to keep the routers list
         // in sequence.  A derived class can overload this method if it needs to track the changes
         // made to the links table over time. If it does so, it should delegate to this method so
         // that required book-keeping can be done.
         ////////////////////////////////////////////////////////////
      public:
         enum link_change_type
         {
            link_change_none,
            link_change_added,
            link_change_deleted,
            link_change_changed,
         };
      protected:
         virtual void on_link_change(
            uint2 node1,
            uint2 node2,
            HopMetric hop_metric,
            link_change_type change);

         ////////////////////////////////////////////////////////////
         // onOneShotFired
         ////////////////////////////////////////////////////////////
         virtual void onOneShotFired(uint4 id);

         ////////////////////////////////////////////////////////////
         // on_unhandled_message
         ////////////////////////////////////////////////////////////
         virtual void on_unhandled_message(message_handle &message);

         ////////////////////////////////////////////////////////////
         // do_debug_report
         //
         // Produces a series of messages in the debug log that describe the
         // current state of this router.
         ////////////////////////////////////////////////////////////
         virtual void do_debug_report();
         
      protected:
         ////////////////////////////////////////////////////////////
         // routes
         //
         // The collection of routes that are discovered by the exchange of routing information and
         // other types of packets. When a message is routed, this container will be searched first
         // for a favourable route. If a route is not found, the static_routes container will be
         // searched as well.
         ////////////////////////////////////////////////////////////
         typedef std::map<uint2, RouterHelpers::route_type> routes_type;
         routes_type routes;

         ////////////////////////////////////////////////////////////
         // static_routes
         //
         // The collection of routes that are provided by the application. Routs placed in this
         // container by an application (calling add_static_route()) can only be removed by the
         // application calling remove_static_route().
         ////////////////////////////////////////////////////////////
         routes_type static_routes;

      private:
         ////////////////////////////////////////////////////////////
         // forward_message
         //
         // Called when a message has been received that should be forwarded
         ////////////////////////////////////////////////////////////
         void forward_message(message_handle &message);

         ////////////////////////////////////////////////////////////
         // on_delivery_failure
         ////////////////////////////////////////////////////////////
         void on_delivery_failure(pakctrl_message_handle &message);

         ////////////////////////////////////////////////////////////
         // new_transaction_id
         //
         // Looks for a new, unique transaction identifier
         ////////////////////////////////////////////////////////////
         byte new_transaction_id(uint2 destination);

         ////////////////////////////////////////////////////////////
         // do_next_router_transaction
         //
         // Evaluates what router transaction should be started next. 
         ////////////////////////////////////////////////////////////
         void do_next_router_transaction();

         ////////////////////////////////////////////////////////////
         // set_next_transaction_focus
         //
         // Checks to see if the next transaction can take focus.  This method will have no effect
         // if there is currently a transaction that does have focus.
         //
         // If there is a transaction in the queue associated with a device with which there is a
         // session established (determined by using the route to the device and querying the
         // associated port), that transaction will be given precedence over other transactions even
         // if that transaction is newer or has lower priority than transactions that do not
         // established sessions.  This is done in order to prevent unwarranted disconnections and
         // deadlocks. 
         ////////////////////////////////////////////////////////////
         void set_next_transaction_focus();
         
         ////////////////////////////////////////////////////////////
         // update_links
         //
         // Called when link information is known to have changed. The first two parameters define
         // the link end-points and the third parameter indicates the type of change that has taken
         // place. 
         ////////////////////////////////////////////////////////////
         void update_links(
            uint2 node1,
            uint2 node2,
            HopMetric hop_metric,
            link_change_type change,
            bool regenerate_routes);

         ////////////////////////////////////////////////////////////
         // update_routers
         //
         // Called when a new router is suspected or a router is to be deleted
         ////////////////////////////////////////////////////////////
         void update_routers(
            uint2 router_id,
            link_change_type link_change);

         ////////////////////////////////////////////////////////////
         // generate_routes_from_links
         //
         // Responsible for generating the routes set from the links and routers information.
         ////////////////////////////////////////////////////////////
         void generate_routes_from_links();

         ////////////////////////////////////////////////////////////
         // evaluate_route_for_node
         //
         // Evaluates a route to the specified node from this router address and creates a new
         // route record if a path can be found.  If the path cannot be found then the return value
         // will be false and no record will be created.
         ////////////////////////////////////////////////////////////
         bool evaluate_route_for_node(uint2 node_id);

         ////////////////////////////////////////////////////////////
         // on_neighbour_change
         //
         // Called when a change is pending that will affect the neighbour list.  This method will
         // do the appropriate book-keeping to make note of the change and update the routers list
         // so that the change(s) will be sent.
         ////////////////////////////////////////////////////////////
         void on_neighbour_change(
            uint2 neighbour_id,
            link_change_type change);

      protected:
         ////////////////////////////////////////////////////////////
         // ports
         //
         // The collection of ports that are currently registered with this router.
         ////////////////////////////////////////////////////////////
         typedef std::list<PortBase *> ports_type;
         ports_type ports;

         ////////////////////////////////////////////////////////////
         // this_node_address
         //
         // The address of this node
         ////////////////////////////////////////////////////////////
         uint2 this_node_address;

         ////////////////////////////////////////////////////////////
         // unrouted_messages
         //
         // A prioritised queue of messages that still need to be routed. This queue is checked each
         // time a port object requests the next message that should be delivered through that port
         // via get_port_send_message(). This list is ordered by the priority of the messages as
         // well as the order in which they were placed in the queue.
         ////////////////////////////////////////////////////////////
         unrouted_messages_type unrouted_messages;

         ////////////////////////////////////////////////////////////
         // last_transaction_id
         //
         // Keeps track of the last transaction identifier that was used.  This value is initialised
         // to a random value in the constructor and is incremented each time that a new transaction
         // is added or an existing transaction is reassigned. 
         ////////////////////////////////////////////////////////////
         byte last_transaction_id;

         ////////////////////////////////////////////////////////////
         // transactions
         //
         // The list of transactions that are currently active on this router.  Note that due to the
         // limitation in the key size, there are only a limited number (255) of transactions that
         // can be supported concurrently.
         ////////////////////////////////////////////////////////////
         transactions_type transactions;

         ////////////////////////////////////////////////////////////
         // neighbours
         //
         // The list of neighbour records currently known to this router. 
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<RouterHelpers::neighbour_type> neighbour_handle;
         typedef std::map<uint2, neighbour_handle> neighbours_type;
         neighbours_type neighbours;

         ////////////////////////////////////////////////////////////
         // current_transaction
         //
         // Keeps track of the current pending router transaction, if any.   This transaction can
         // be a hello transaction or a get or send neighbour lists transaction.  This member is
         // used to serialise these transactions so that they don't all happen at once. 
         ////////////////////////////////////////////////////////////
         transaction_handle current_transaction;

         ////////////////////////////////////////////////////////////
         // timer
         //
         // Reference to the one shot timer that used to drive transactions.
         ////////////////////////////////////////////////////////////
         timer_handle timer;

         ////////////////////////////////////////////////////////////
         // all_routers
         //
         // The list of all known routers in the network including this node.
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<RouterHelpers::router_type> router_handle;
         typedef std::map<uint2, router_handle> all_routers_type;
         all_routers_type all_routers;
         
         ////////////////////////////////////////////////////////////
         // links
         //
         // The list of all links that are currently known in the network.
         ////////////////////////////////////////////////////////////
         typedef std::list<RouterHelpers::link_entry_type> links_type;
         links_type links;

         ////////////////////////////////////////////////////////////
         // neighbour_list_version
         //
         // Keeps track of the current version of the neighbour list for this router. This version
         // number will be incremented each time that a change occurs to the neighbour list.  It is
         // sent to other routers in the network so that they can be assured that their lists are
         // current.
         ////////////////////////////////////////////////////////////
         byte neighbour_list_version;

         ////////////////////////////////////////////////////////////
         // last_neighbour_change
         //
         // Keeps track of the last change that occurred on the neighbour list.
         ////////////////////////////////////////////////////////////
         struct last_neighbour_change_type
         {
            link_change_type change;
            neighbour_handle neighbour;
         } last_neighbour_change;

         ////////////////////////////////////////////////////////////
         // waiting_transactions
         //
         // The queue for transactions that are waiting for focus.  Note that this queue is order by
         // the transaction priority such that higher priority transactions gavitate toward the
         // front. 
         ////////////////////////////////////////////////////////////
         class waiting_transaction_gtr
         {
         public:
            bool operator ()(transaction_handle const &t1, transaction_handle const &t2);
         };
         typedef
         OrderedList<transaction_handle, waiting_transaction_gtr> waiting_transactions_type;
         waiting_transactions_type waiting_transactions;

         ////////////////////////////////////////////////////////////
         // current_transaction_focus
         //
         // Handle to the transaction (if there is any) that currently has the focus.  
         ////////////////////////////////////////////////////////////
         transaction_handle current_transaction_focus;

         ////////////////////////////////////////////////////////////
         // maintenance_id
         //
         // Identifies the timer that is used to drive an occasional maintenance schedule for this
         // router.
         ////////////////////////////////////////////////////////////
         uint4 maintenance_id;

         ////////////////////////////////////////////////////////////
         // is_shutting_down
         //
         // This flag is set when the destructor begins to execute.  It prevents things like
         // transaction focus from being set.
         ////////////////////////////////////////////////////////////
         bool is_shutting_down;

         ////////////////////////////////////////////////////////////
         // is_leaf_node
         //
         // If set to true, this router will behave as a leaf node.  This
         // member is set via the set_is_leaf_node() and accessed via the
         // get_is_leaf_node() methods.  
         ////////////////////////////////////////////////////////////
         bool is_leaf_node;

         ////////////////////////////////////////////////////////////
         // prevent_next_focus
         //
         // Set to prevent the next focus from being set after a transaction is
         // closed.  This is needed because, when a link fails, any
         // transactions associated with that link can be shut down in rapid
         // succession.  If the next focus is not prevented, there may be extra
         // communication following that is undesirable.
         ////////////////////////////////////////////////////////////
         bool prevent_next_focus;

         ////////////////////////////////////////////////////////////
         // time_since_last_report
         //
         // Keeps track of the time elapsed since a status report was written
         // to the debug log.  
         ////////////////////////////////////////////////////////////
         uint4 time_since_last_report;

         ////////////////////////////////////////////////////////////
         // ciphers
         ////////////////////////////////////////////////////////////
         typedef std::map<uint2, cipher_handle> ciphers_type;
         ciphers_type ciphers;

         ////////////////////////////////////////////////////////////
         // maintenance_interval
         ////////////////////////////////////////////////////////////
         static uint4 const maintenance_interval;

         friend class RouterHelpers::SendNeighboursTran;
         friend class RouterHelpers::GetNeighboursTran;
      };
   };
};


#endif

