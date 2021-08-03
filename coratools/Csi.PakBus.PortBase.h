/* Csi.PakBus.PortBase.h

   Copyright (C) 2001, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 24 February 2001
   Last Change: Monday 02 March 2020
   Last Commit: $Date: 2020-03-03 10:29:08 -0600 (Tue, 03 Mar 2020) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_PakBus_PortBase_h
#define Csi_PakBus_PortBase_h

#include "Csi.InstanceValidator.h"
#include "CsiTypeDefs.h"
#include "Csi.PakBus.HopMetric.h"
#include "Csi.SharedPtr.h"
#include "Csi.PakBus.Defs.h"
#include "StrAsc.h"



namespace Csi
{
   namespace PakBus
   {
      class Router;
      class Message;


      /**
       * Defines a base class for all PakBus port objects.  A PakBus port is an abstraction that is
       * responsible for the delivery of PakBus messages over some medium such as a serial port or a
       * socket.
       */
      class PortBase: public InstanceValidator
      {
      public:
         /**
          * Destructor
          */
         virtual ~PortBase()
         { }
         
         /**
          * Called by the router when it has detected a condition where it might have a message that
          * this port should delivery.  When the port is in a state where it is ready to sendthat
          * message, it should call the router's get_next_port_message() passing itself as the
          * port.  From the time when the router calls this method to the time when the port object
          * queries the router for the message, the router may have received updated route
          * information and decided either that the message is undeliverable or should be sent to
          * another port.  In addition, more than one message may be queued.
          *
          * @param physical_destination Specifies the neighbour address for the message to be sent.
          *
          * @param priority Specifies the priority for the message.
          */
         typedef Priorities::priority_type priority_type;
         virtual void on_message_ready(uint2 physical_destination, priority_type priority) = 0;

         /**
          * Called by the router when the specified message should be broadcast.
          *
          * @param message Specifies the message to broadcast.
          */
         typedef SharedPtr<Message> message_handle;
         virtual void broadcast_message(message_handle &message) = 0;

         /**
          * Called by the router when a message that has not yet been retrieved by the port has been
          * removed from the queue at the request of the application layer.  This must be overloaded
          * to help determine whether the port link resource should be kept open.
          *
          * @param physical_destination Specifies the neighbour address of the aborted message.
          */
         virtual void on_message_aborted(uint2 physical_destination) = 0;

         /**
          * @return Must be overloaded to return the configured beacon interval in seconds.
          */
         virtual uint2 get_beacon_interval() = 0;

         /**
          * @return Returns the configured verify interval in seconds.  If not overloaded, returns
          * the configured beacon interval.
          */
         virtual uint2 get_verify_interval()
         { return get_beacon_interval(); }

         /**
          * @return Returns the worst case response interval in millisecons expected for the link
          * associated with this port.  This estimate should include the amount of time required to
          * dial the link as well.
          */
         virtual uint4 get_worst_case_response()  = 0; 

         /**
          * @return Returns the hop metric for the link associated with this port based upon the
          * return value for get_worst_case_response().  This method can be overloaded if a
          * different hop metric should be supplied.
          */
         virtual HopMetric get_hop_metric()
         {
            HopMetric rtn;
            rtn.set_response_time_msec(get_worst_case_response());
            return rtn;
         }

         /**
          * @return Returns true if a session exists between the two specified addresses.
          *
          * @param source_address Specifies the source address for the session.
          *
          * @param destination_address Specifies he destination address for the session.
          */
         virtual bool has_session(uint2 source_address, uint2 destination_address) = 0;

         /**
          * @return Evaluates whether the link that supports this port must be dialed like a phone
          * modem.  These types of links require special treatment because they cannot be left open
          * all of the time.
          */
         virtual bool link_is_dialed() = 0;

         /**
          * @return Evaluates whether the link resource used by this port is active (ready to send
          * or receive messages.
          */
         virtual bool link_is_active() = 0;

         /**
          * @return Returns true if the link resource used by the port must be closed immediately.
          */
         virtual bool must_close_link() = 0;

         /**
          * Called by the router when it has determined that the specified neighbour address is no
          * longer valid.  This can happen if the router processes a goodbye command or a similar
          * event.
          *
          * @param physical_address Specifies the address of the neighbour.
          */
         virtual void on_neighbour_lost(uint2 physical_address) = 0;

         /**
          * @return Evaluates whether the specified physical address should be accepted as a
          * neighbour in this port.
          *
          * @param neighbour_address Specifies the address of the potential neighbour.
          */
         virtual bool can_accept_neighbour(uint2 neighbour_address)
         { return neighbour_address > 0 && neighbour_address < 4095; }

         /**
          * @return Evaludates based upon the physical source and destination addresses whether a
          * message should be processed through this port.  This logic is generally shared and so is
          * implemented in this base class.  A derived class can overload this or it can modify the
          * behaviour in an overloaded version of can_accept_neighbour().
          *
          * @param physical_source Specifies the source neighbour address.
          *
          * @param physical_destination Specifies the destination neighbour address.
          *
          * @param router Specifies the router calling this method.
          */
         virtual bool should_process_message(
            uint2 physical_source, uint2 physical_destination, Router *router);

         /**
          * @return Evaluates to true if the overall timeout for transactions that use this port
          * should be capped at 35 seconds.
          */
         virtual bool should_cap_timeout() const
         { return true; }

         /**
          * Called to reset the timer for the session between the two addresses.
          *
          * @param source Specifies the source address.
          *
          * @param dest Specifies the destination address.
          */
         virtual void reset_session_timer(uint2 source, uint2 dest)
         { }

         /**
          * @return Returns the name of this port.
          */
         virtual StrAsc get_port_name() const = 0;

         /**
          * @param router_ Specifies the PakBus router that will be used by this port.
          */
         typedef Csi::SharedPtr<Router> router_handle;
         virtual void set_pakbus_router(router_handle router_) = 0;

         /**
          * @return Must be overloaded to return the assigned PakBus router.
          */
         virtual router_handle &get_pakbus_router() = 0;
      };
   };
};


#endif
