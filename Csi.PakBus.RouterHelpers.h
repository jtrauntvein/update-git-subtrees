/* Csi.PakBus.RouterHelpers.h

   This header contains definitions of classes that are used by class Router.
   They are declared in this header to avoid confusion in the header that
   declares class Router.

   Copyright (C) 2002, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 22 March 2002
   Last Change: Friday 10 February 2012
   Last Commit: $Date: 2012-02-10 11:33:16 -0600 (Fri, 10 Feb 2012) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_RouterHelpers_h
#define Csi_PakBus_RouterHelpers_h

#include "Csi.Utils.h"
#include "Csi.PakBus.HopMetric.h"
#include "Csi.PakBus.PortBase.h"
#include "Csi.MaxMin.h"


namespace Csi
{
   namespace PakBus
   {
      //@group class forward declarations
      class PakBusTran;
      //@endgroup

      
      namespace RouterHelpers
      {
         //@group class forward declarations
         class HelloTran;
         class SendNeighboursTran;
         class GetNeighboursTran;
         //@endgroup

         
         ////////////////////////////////////////////////////////////
         // class transaction_id
         //
         // Used to identify a PakBus transaction.
         ////////////////////////////////////////////////////////////
         class transaction_id
         {
         private:
            ////////////////////////////////////////////////////////////
            // coded_id
            //
            // Contains the coded identifier for the transaction with the
            // destination address in the least significant bytes and the
            // transaction number in the second most significant byte.
            ////////////////////////////////////////////////////////////
            uint4 coded_id;
            
         public:
            ////////////////////////////////////////////////////////////
            // default constructor
            ////////////////////////////////////////////////////////////
            transaction_id():
               coded_id(0)
            { }
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            transaction_id(uint2 address, byte transaction_no)
            {
               coded_id = static_cast<uint4>(address) +
                  (static_cast<uint4>(transaction_no) << 24);
            }

            ////////////////////////////////////////////////////////////
            // copy constructor
            ////////////////////////////////////////////////////////////
            transaction_id(transaction_id const &other):
               coded_id(other.coded_id)
            { }

            ////////////////////////////////////////////////////////////
            // copy operator
            ////////////////////////////////////////////////////////////
            transaction_id &operator =(transaction_id const &other)
            { coded_id = other.coded_id; return *this; }

            ////////////////////////////////////////////////////////////
            // comparison
            ////////////////////////////////////////////////////////////
            bool operator <(transaction_id const &other) const
            { return coded_id < other.coded_id; }
         };


         ////////////////////////////////////////////////////////////
         // class neighbour_type
         //
         // Keeps track of the state information associated with a neighbour
         // node.
         ////////////////////////////////////////////////////////////
         class neighbour_type
         {
         public:
            ////////////////////////////////////////////////////////////
            // is_router
            //
            // Set to true if this neighbour is a router
            ////////////////////////////////////////////////////////////
            bool is_router;

            ////////////////////////////////////////////////////////////
            // hop_metric
            //
            // Identifies the time required to reach that neighbour
            ////////////////////////////////////////////////////////////
            HopMetric hop_metric;

            ////////////////////////////////////////////////////////////
            // port
            //
            // Refers to the port through which this neighbour is reached.
            ////////////////////////////////////////////////////////////
            PortBase *port;

            ////////////////////////////////////////////////////////////
            // physical_destination
            //
            // The physical address of this neighbour.
            ////////////////////////////////////////////////////////////
            uint2 physical_address;

            ////////////////////////////////////////////////////////////
            // time_since_last_beacon
            //
            // Value of Csi::counter(0) at the time that the last beacon was
            // received from this neighbour.
            ////////////////////////////////////////////////////////////
            uint4 time_since_last_beacon;

            ////////////////////////////////////////////////////////////
            // verification_interval
            //
            // Stores the verification interval (in seconds) reported by the
            // neighbour.  This value is used by broadcast_is_beacon() and
            // calculate_verification_interval().
            ////////////////////////////////////////////////////////////
            uint2 verification_interval;

            ////////////////////////////////////////////////////////////
            // hello_tries
            //
            // This value keeps track of whether the hello transaction needs to
            // be tried with this neighbour and also of the number of retries
            // of that transaction that have taken place.  It should be reset
            // to zero when a successful hello takes place with the neighbour
            // (including when that neighbour sends a hello command).  It will
            // be set to one if the beacon interval for that neighbour expires
            // and will be incremented with each subsequent attempt to execute
            // the hello transaction with that neighbour.
            ////////////////////////////////////////////////////////////
            byte hello_tries;

            ////////////////////////////////////////////////////////////
            // send_hello_delay_base
            //
            // Records the base time that a send hello should be delayed for
            // this neighbour.  This is used in conjunction with needs_hello
            // and send_hello_delay to determine when the hello command can be
            // sent to this neighbour.
            ////////////////////////////////////////////////////////////
            uint4 send_hello_delay_base;

            ////////////////////////////////////////////////////////////
            // send_hello_delay
            //
            // Records the amount of time that we should delay before sending a
            // hello command to this neighbour.  This value is determined to a
            // random value when a new neighbour is detected from a broadcast
            // beacon.
            ////////////////////////////////////////////////////////////
            uint4 send_hello_delay;

            ////////////////////////////////////////////////////////////
            // needs_hello_info
            //
            // A flag that is set when this neighbour was created because of a
            // beacon that indicates that we haven't received the first hello
            // information from this neighbour.  This flag will be reset by the
            // router when either a hello command or hello response is received
            // from the neighbour.
            ////////////////////////////////////////////////////////////
            bool needs_hello_info;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            neighbour_type(
               PortBase *port_,
               HopMetric hop_metric_,
               uint2 physical_address_):
               port(port_),
               hop_metric(hop_metric_),
               physical_address(physical_address_), 
               is_router(false),
               verification_interval(0),
               hello_tries(1),
               time_since_last_beacon(Csi::counter(0)),
               send_hello_delay(0),
               send_hello_delay_base(Csi::counter(0)),
               needs_hello_info(true)
            { }

            ////////////////////////////////////////////////////////////
            // pack_list_entry
            //
            // Packs the neighbour information into a two byte entry as sent in
            // the send or get neighbour list transactions.
            ////////////////////////////////////////////////////////////
            uint2 pack_list_entry() const
            {
               uint2 rtn = is_router ? 0x08 : 0;
               rtn |= (static_cast<byte>(hop_metric) & 0x07);
               rtn <<= 12;
               rtn |= (physical_address & 0x0fff);
               return rtn; 
            }

            ////////////////////////////////////////////////////////////
            // broadcast_is_beacon
            //
            // Evaluates whether a broadcast message should be treated as a
            // beacon for this neighbour.  This is done by comparing the
            // reported verification interval with the port's beacon interval.
            ////////////////////////////////////////////////////////////
            bool broadcast_is_beacon()
            {
               // calculate the intervals of the port and the neighbour in milli-seconds
               uint4 neighbour_msec = verification_interval;
               uint4 port_msec = port->get_beacon_interval();

               if(neighbour_msec == 0)
                  neighbour_msec = 0xffff;
               if(port_msec == 0)
                  port_msec = 0xffff;
               neighbour_msec *= 1000;
               port_msec *= 1000;

               // now compare the two
               bool rtn = true;
               if(neighbour_msec*2 + neighbour_msec/2 < port_msec)
                  rtn = false;
               return rtn;
            } 

            ////////////////////////////////////////////////////////////
            // calculate_verification_interval
            //
            // Calculates the verification interval (in milli-seconds) for this
            // link based upon the port's beacon interval and the neighbour's
            // reported verification interval.  If neighter side specifies a
            // valid verification inteval, a default of five minutes will be
            // used.
            ////////////////////////////////////////////////////////////
            uint4 calculate_verification_interval()
            {
               // calculate the intervals of the port and the neighbour in milli-seconds
               uint4 neighbour_msec = verification_interval;
               uint4 port_msec = port->get_verify_interval();
               uint4 rtn;
               
               if(neighbour_msec == 0)
                  neighbour_msec = 0xffff;
               if(port_msec == 0)
                  port_msec = 0xffff;
               neighbour_msec *= 1000;
               port_msec *= 1000;

               // now we will compare them
               rtn = csimin(
                  neighbour_msec*2 + neighbour_msec/2,
                  port_msec*3);
               if(rtn == 0 && port->should_cap_timeout())
                  rtn = 300000;
               return rtn;
            } 
         };


         ////////////////////////////////////////////////////////////
         // class route_type
         //
         // Describes an entry in the router's route table.
         ////////////////////////////////////////////////////////////
         class route_type
         {
         public:
            ////////////////////////////////////////////////////////////
            // destination
            ////////////////////////////////////////////////////////////
            uint2 destination;

            ////////////////////////////////////////////////////////////
            // neighbour_id
            //
            // The physical identifier for the neighbour that should be used for this route.
            ////////////////////////////////////////////////////////////
            uint2 neighbour_id;

            ////////////////////////////////////////////////////////////
            // port
            ////////////////////////////////////////////////////////////
            PortBase *port;

            ////////////////////////////////////////////////////////////
            // resp_time_msec
            ////////////////////////////////////////////////////////////
            uint4 resp_time_msec;

         public:
            ////////////////////////////////////////////////////////////
            // default constructor
            ////////////////////////////////////////////////////////////
            route_type():
               destination(0),
               resp_time_msec(0),
               neighbour_id(0),
               port(0)
            { }

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            route_type(
               uint2 destination_,
               uint2 neighbour_id_,
               PortBase *port_,
               uint4 resp_time_msec_):
               destination(destination_),
               neighbour_id(neighbour_id_),
               port(port_),
               resp_time_msec(resp_time_msec_)
            { }

            ////////////////////////////////////////////////////////////
            // copy constructor
            ////////////////////////////////////////////////////////////
            route_type(route_type const &other):
               destination(other.destination),
               neighbour_id(other.neighbour_id),
               port(other.port),
               resp_time_msec(other.resp_time_msec)
            { }

            ////////////////////////////////////////////////////////////
            // copy operator
            ////////////////////////////////////////////////////////////
            route_type &operator =(route_type const &other)
            {
               destination = other.destination;
               neighbour_id = other.neighbour_id;
               port = other.port;
               resp_time_msec = other.resp_time_msec;
               return *this;
            }
         };


         ////////////////////////////////////////////////////////////
         // router_type
         //
         // Defines the information that a router object will keep about other routers in the
         // network. 
         ////////////////////////////////////////////////////////////
         class router_type
         {
         public:
            ////////////////////////////////////////////////////////////
            // router_id
            // 
            // The node id of the router.
            ////////////////////////////////////////////////////////////
            uint2 router_id;

            ////////////////////////////////////////////////////////////
            // neighbour_list_version
            //
            // The version of the neighbour list that was last sent to this router.
            ////////////////////////////////////////////////////////////
            byte neighbour_list_version;

            ////////////////////////////////////////////////////////////
            // send_change
            //
            // Set to true if the most recent change to the neighbour list should be sent to this
            // router. 
            ////////////////////////////////////////////////////////////
            bool send_change;

            ////////////////////////////////////////////////////////////
            // send_all
            //
            // Set to true if the entire neighbour list should be sent to this router.  This value
            // will get set if one or more changes occur after the send_change flag was already
            // set. 
            ////////////////////////////////////////////////////////////
            bool send_all;

            ////////////////////////////////////////////////////////////
            // get_all
            //
            // Set to true if all of the neighbours associated with this router need to be
            // retrieved.  This value will be reset when the neighbour list from the router is
            // received.
            ////////////////////////////////////////////////////////////
            bool get_all;

            ////////////////////////////////////////////////////////////
            // temp_is_visited
            //
            // Used temporarily by the route generation algorithm to evaluate whether this router
            // has already been considered. 
            ////////////////////////////////////////////////////////////
            bool temp_is_visited;

            ////////////////////////////////////////////////////////////
            // temp_neighbour_id
            //
            // Temporarily records the neighbour identifier for use during the route generation
            // algorithm. 
            ////////////////////////////////////////////////////////////
            uint2 temp_neighbour_id;

            ////////////////////////////////////////////////////////////
            // temp_resp_msec
            //
            // Temporarily records the timed response in milli-seconds to this node during the route
            // generation algorithm.
            ////////////////////////////////////////////////////////////
            uint4 temp_resp_msec;

            ////////////////////////////////////////////////////////////
            // send_delay_base
            //
            // Specifies the base time that should be used to delay the sending or getting of
            // neighbour lists from this router.  This value will be zero if there should be no
            // delay. 
            ////////////////////////////////////////////////////////////
            uint4 send_delay_base;

            ////////////////////////////////////////////////////////////
            // validated
            //
            // Set to false when the port used to reach this router is in an
            // inactive state.  The router will set this flag to true when
            // hello information has been received or if this address is
            // mentioned in a neighbour list that has been received.  This flag
            // will prevent the server from initiating send or get neighbours
            // with the router. 
            ////////////////////////////////////////////////////////////
            bool validated;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            router_type(
               uint2 router_id_,
               byte neighbour_list_version_):
               router_id(router_id_),
               neighbour_list_version(neighbour_list_version_),
               send_change(false),
               send_all(true),
               get_all(true),
               send_delay_base(0),
               validated(true)
            { }
         };


         ////////////////////////////////////////////////////////////
         // link_entry_type
         //
         // Stores the information about one link in the network.
         ////////////////////////////////////////////////////////////
         class link_entry_type
         {
         private:
            ////////////////////////////////////////////////////////////
            // id1
            ////////////////////////////////////////////////////////////
            uint2 id1;

            ////////////////////////////////////////////////////////////
            // id2
            ////////////////////////////////////////////////////////////
            uint2 id2;

            ////////////////////////////////////////////////////////////
            // hop_metric
            ////////////////////////////////////////////////////////////
            HopMetric hop_metric;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            link_entry_type(uint2 id1_ = 0, uint2 id2_ = 0, HopMetric hop_metric_ = 0):
               id1(id1_),
               id2(id2_),
               hop_metric(hop_metric_)
            { }

            ////////////////////////////////////////////////////////////
            // copy constructor
            ////////////////////////////////////////////////////////////
            link_entry_type(link_entry_type const &other):
               id1(other.id1),
               id2(other.id2),
               hop_metric(other.hop_metric)
            { }

            ////////////////////////////////////////////////////////////
            // copy operator
            ////////////////////////////////////////////////////////////
            link_entry_type &operator =(link_entry_type const &other)
            {
               id1 = other.id1;
               id2 = other.id2;
               hop_metric = other.hop_metric;
               return *this;
            }

            ////////////////////////////////////////////////////////////
            // get_id1
            ////////////////////////////////////////////////////////////
            uint2 get_id1() const { return id1; }

            ////////////////////////////////////////////////////////////
            // get_id2
            ////////////////////////////////////////////////////////////
            uint2 get_id2() const { return id2; }

            ////////////////////////////////////////////////////////////
            // get_hop_metric
            ////////////////////////////////////////////////////////////
            HopMetric get_hop_metric() const { return hop_metric; }

            ////////////////////////////////////////////////////////////
            // set_hop_metric
            ////////////////////////////////////////////////////////////
            void set_hop_metric(HopMetric hop_metric_) { hop_metric = hop_metric_; }

            ////////////////////////////////////////////////////////////
            // has_id
            //
            // Evaluates whether one of the identifiers matches the specified one.  Returns the
            // other identifier if there is a match or zero if there is no match.
            ////////////////////////////////////////////////////////////
            uint2 has_id(uint2 id) const
            {
               uint2 rtn = 0;
               if(id1 == id)
                  rtn = id2;
               else if(id2 == id)
                  rtn = id1;
               return rtn;
            }
         };
      }; 
   };
};


#endif
