/* Csi.PakBus.Message.h

   Copyright (C) 2001, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 24 February 2001
   Last Change: Wednesday 04 April 2018
   Last Commit: $Date: 2018-04-04 16:43:59 -0600 (Wed, 04 Apr 2018) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_PakBus_Message_h
#define Csi_PakBus_Message_h


#include "Packet.h"
#include "Csi.PakBus.Defs.h"
#include "CsiTypeDefs.h"
#include <iostream>


namespace Csi
{
   namespace PakBus
   {
      //@group class forward declarations
      class PortBase;
      //@endgroup


      /**
       * Defines a base class for all messages that can be sent or received using the PakBus
       * protocol.
       */
      class Message: public ::Packet
      {
      public:
         /**
          * Specifeis the maximum number of unquoted bytes allowed for the body of a PakBus message
          * excluding the space required for the header.
          */
         static uint4 const max_body_len;

         /**
          * Default constructor
          *
          * @param header_len=0 Specifies the expected size of the header.
          */
         Message(uint4 header_len = 0);

         /**
          * Copy constructor
          *
          * @param other Specifies the message to copy.
          *
          * @param header_len=0 Specifies the expected length of the header.
          *
          * @param deep_copy=false Set to true if this message is to make its own buffer for the
          * message.
          */
         Message(Message &other, uint4 header_len = 0, bool deep_copy = false);

         /**
          * Construct from raw bytes.
          *
          * @param buff Specifies the source of the message.
          *
          * @param buff_len Specifies the number of bytes in the source.
          *
          * @param header_len=0 Specifies the expected header length.
          *
          * @param make_copy=true Set to false if the message should use the provided buffer instead
          * of copying it.
          */
         Message(
            void const *buff,
            uint4 buff_len,
            uint4 header_len = 0,
            bool make_copy = true);

         // @group: header field access 

         /**
          * @return Returns the high protocol code for this message.
          */
         typedef ProtocolTypes::protocol_type high_protocol_type;
         high_protocol_type get_high_protocol() const
         { return high_protocol; }

         /**
          * @param high_protocol_ Specifies the high level protocol code for the message.
          */
         void set_high_protocol(high_protocol_type high_protocol_)
         { high_protocol = high_protocol_; }

         /**
          * @return Returns the message destination address.
          */
         uint2 get_destination() const
         { return destination; }

         /**
          * @param destination_ Specifies the message destination address.
          */
         void set_destination(uint2 destination_)
         { destination = destination_; } 

         /**
          * @return Returns the hop count.
          */
         byte get_hop_count() const
         { return hop_count; }

         /**
          * @param hop_count_ Specifies the hop count.
          */
         void set_hop_count(byte hop_count_)
         { hop_count = hop_count_; }

         /**
          * @return Returns the message source address.
          */
         uint2 get_source() const
         { return source; }

         /**
          * @param source_ Specifies the message source address.
          */
         void set_source(uint2 source_)
         { source = source_; }

         /**
          * @return Returns the expect more flag for this message.
          */
         typedef ExpectMoreCodes::expect_more_code_type expect_more_type;
         expect_more_type get_expect_more() const
         { return expect_more; }

         /**
          * @param expect_more_ Specifies the expect more flag for this message.
          */
         void set_expect_more(expect_more_type expect_more_)
         { expect_more = expect_more_; }

         /**
          * @return Returns this message priority code.
          */
         typedef Priorities::priority_type priority_type;
         priority_type get_priority() const
         { return priority; }

         /**
          * @param priority_ Specifies this message priority.
          */
         void set_priority(priority_type priority_)
         { priority = priority_; }

         /**
          * @return Returns the message physical source address.
          */
         uint2 get_physical_source() const
         { return physical_source; }

         /**
          * @param physical_source_ Specifies the message physical source.
          */
         void set_physical_source(uint2 physical_source_)
         { physical_source = physical_source_; }

         /**
          * @return Returns the physical destination address.
          */
         uint2 get_physical_destination() const
         { return physical_destination; }

         /**
          * @param physical_destination_ Specifies the physical destination.
          */
         void set_physical_destination(uint2 physical_destination_)
         { physical_destination = physical_destination_; }

         /**
          * @return Returns the port assigned to this message (the port from which this message was
          * received.
          */
         PortBase *get_port() const
         { return port; }

         /**
          * @param port_ Specifies the port assigned to this message.
          */
         void set_port(PortBase *port_)
         { port = port_; }
         
         // @endgroup:

         /**
          * @return Returns the time that this message has been in the queue in milliseconds.
          */
         uint4 get_age_msec() const;

         /**
          * Resets ther age timer for this message.
          */
         void reset_age();

         /**
          * @return Returns true if this message should specify its own routing.
          */
         bool get_use_own_route() const
         { return use_own_route; }

         /**
          * @param use_own_route_ Set to true if this message should specify its own routing.
          */
         void set_use_own_route(bool use_own_route_)
         { use_own_route = use_own_route_; }

         /**
          * @return Returns the interval in milli-seconds at which a response is expected for this
          * message.
          */
         uint4 get_expected_response_interval() const
         { return expected_response_interval; }

         /**
          * @param interval Specifies the interval in milliseconds for which a response is expected
          * for this message.
          */
         void set_expected_response_interval(uint4 interval)
         { expected_response_interval = interval; }

         /**
          * @param will_close_ Set to true if this message is expected to close the link.
          */
         void set_will_close(bool will_close_)
         { will_close = will_close_; }

         /**
          * @return Returns true if this message is expected to close the link.
          */
         bool get_will_close() const
         { return will_close; }

         /**
          * Writes a description of the message to the specified stream in a format that can be
          * written to router log files.
          *
          * @return Returns true if the message is a please wait message.
          */
         virtual bool describe_message(std::ostream &out);

         /**
          * @return Returns a pointer to the beginning of the message body.
          */
         void const *get_body()
         { return getMsg() + get_headerLen(); }

         /**
          * @return Returns the number of byets in the message body.
          */
         uint4 get_body_len() const
         { return length() - get_headerLen(); }

         /**
          * @return Returns true if this message has been encrypted.
          */
         bool get_encrypted() const
         { return encrypted; }

         /**
          * @param encrypted_ Set to true if this message has been replaced with an encrypted
          * version.
          */
         void set_encrypted(bool encrypted_)
         { encrypted = encrypted_; }

         /**
          * @return Can be overloaded to return true if this message is a type that can be
          * encrypted.
          */
         virtual bool should_encrypt()
         { return false; }
         
      protected:
         //@group: meta-data to/from the network layer

         /**
          * Specifies whether there are more messages to be expected on the network path.
          */
         expect_more_type expect_more;

         /**
          * Specifies the physical source address.
          */
         uint2 physical_source;

         /**
          * Specifies the physical destination address.
          */
         uint2 physical_destination;

         /**
          * Specifies the port assigned to deliver the message or from which the message was
          * received.
          */
         PortBase *port;

         /**
          * Specifies the priority of the message.
          */
         priority_type priority;

         /**
          * Specifies the high level protocol code.
          */
         high_protocol_type high_protocol;

         /**
          * Specifis the message source address.
          */
         uint2 source;

         /**
          * Specifies the message destination address.
          */
         uint2 destination;

         /**
          * Specifies the total number of times that this message has been forwarded.
          */
         byte hop_count;

         /**
          * Specifies the base for the age counter for this message.
          */
         uint4 age_base;

         /**
          * Set to true if this message should use its own port and physical destination rather than
          * letting the router determine the route.
          */
         bool use_own_route;

         /**
          * Specifies the interval in milliseconds for which a response will be expected for this
          * message.  This value will be interpreted by half-duplex links and will prevent the
          * router from sending more messages until this time has elapsed.
          */
         uint4 expected_response_interval;

         /**
          * Set to true if this message is meant to terminate a session.
          */
         bool will_close;

         /**
          * Set to true if this message was created from an encrypted payload or if it should be
          * encrypted before being sent.
          */
         bool encrypted;
         
         //@endgroup
      };
   };
};


#endif
