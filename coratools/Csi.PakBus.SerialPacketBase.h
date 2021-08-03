/* Csi.PakBus.SerialPacketBase.h

   Copyright (C) 2001, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 12 July 2001
   Last Change: Monday 02 March 2020
   Last Commit: $Date: 2020-03-03 10:29:08 -0600 (Tue, 03 Mar 2020) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_PakBus_SerialPacketBase_h
#define Csi_PakBus_SerialPacketBase_h


#include "Csi.PakBus.PortBase.h"
#include "Csi.PakBus.SerialPacket.h"
#include "Csi.PakBus.Router.h"
#include "Csi.Events.h"
#include "OneShot.h"
#include "StrBin.h" 
#include <map>
#include <list>


namespace Csi
{
   namespace PakBus
   {
      //@group class forward declarations
      class Message;
      namespace SerialPacketBaseHelpers
      {
         class link_type;
      };
      //@endgroup


      /**
       * Defines an abstract base class for a PakBus port that implements the PakBus low level
       * serial protocol.  This class does not contain the code that interfaces directly with the
       * communications subsystem but defines protected virtual methods that must be overloaded to
       * perform low level communication services.  This class does, however, implement most of the
       * logic and data needed to implement the serial packet protocol.
       */
      class SerialPacketBase:
         public PortBase,
         public OneShotClient,
         public EventReceiver
      {
      public:
         /**
          * Construct with router.
          *
          * @param router_ Specifies the router that will own this port.
          *
          * @param timer_ Specifies the one shot timer used for measuring time intervals.
          */
         typedef Csi::SharedPtr<OneShot> timer_handle;
         SerialPacketBase(
            router_handle &router_,
            timer_handle &timer_);

         /**
          * Construct without router.  If the port is constriucted this way, the router must be
          * provided before the port can be used.
          *
          * @param timer_ Specifies the one shot timer.
          */
         SerialPacketBase(timer_handle timer_);

         /**
          * Destructor
          */
         virtual ~SerialPacketBase();

         // @group: methods overloaded from class PortBase

         /**
          * Overloads the base class version to handle the notification that a message is ready.
          */
         virtual void on_message_ready(
            uint2 physical_destination,
            priority_type priority);

         /**
          * Overloads the base class version to handle the notification that a message has been
          * removed.
          */
         virtual void on_message_aborted(uint2 physical_destination);

         /**
          * Overloads the base class version to send the specified message as a broadcast.
          */
         virtual void broadcast_message(message_handle &message);

         /**
          * @return Overloads the base class version to return true if a session exists for the
          * specified combination of addresses.
          */
         virtual bool has_session(
            uint2 source_address,
            uint2 destination_address);

         /**
          * @return Overloads the base class version to return true to indicate whether the link is
          * active.
          */
         virtual bool link_is_active();

         /**
          * Overloads the base class version to handle the event where a neighbour has been lost.
          */
         virtual void on_neighbour_lost(uint2 physical_destination);

         /**
          * Overloads the base class version to reset the timer for the session associated with the
          * specified pair of addresses.
          */
         virtual void reset_session_timer(uint2 source, uint2 dest);

         // @endgroup:

         /**
          * Overloads the base class version to handle delayed timer events.
          */
         virtual void onOneShotFired(uint4 timer_id);

         /**
          * Overloads the base class version to handle asynchronous events.
          */
         virtual void receive(SharedPtr<Event> &ev);

         /**
          * @return Returns the PakBus router.
          */
         router_handle &get_pakbus_router()
         { return router; }

         /**
          * @param router_ Specifies the PakBus router.
          */
         virtual void set_pakbus_router(router_handle router_)
         { router = router_; }

         /**
          * Sends the specified low level serial packet.  This method will fill in the details of
          * the packet including the destination and source physical addresses as well as packet
          * link state.
          *
          * @param packet Specifies the packet to send.
          *
          * @param destination_physical_address Specifies the physical address for the destination.
          *
          * @param link_state Specifies the link state.
          */
         void send_serial_packet(
            SerialPacket &packet,
            uint2 destination_physical_address,
            SerialPacket::link_state_type link_state);

         /**
          * Called by the application to specify whether communications are enabled or disabled.
          *
          * @param comm_enabled_ Set to true if communications for this port are enabled.
          */
         void on_comm_enabled_change(bool comm_enabled_);

         /**
          * Must be overloaded to handle the event where there is data that needs to be sent on this
          * port but the port is in an off-line state.  The application can use this to start the
          * process of allocating communication resources and must call on_link_ready() when the
          * resources are ready.
          *
          * @param priority Specifies the priority of the request.
          */
         virtual void on_needs_to_dial(priority_type priority) = 0;

         /**
          * @return Can be overloaded to return true if the port resource is ready to send packets.
          */
         virtual bool get_is_dialed()
         { return true; }

      protected:
         /**
          * This method must be overloaded to transmit the data in the specified buffer.
          *
          * @param buffer Specifies the buffer to transmit.
          *
          8 @param buffer_len Specifies the numebr of bytes to transmit.
          *
          * @param repeat_count Specifies the number of times that the buffer should be sent.  A
          * value of one means that the buffer will be sent only once.
          *
          * @param msec_between Specifies the minimum delay between repeated transmissions.  A value
          * of zero means that there should be no delay.
          */
         virtual void write_data(
            void const *buffer,
            uint4 buffer_len,
            uint4 repeat_count = 1,
            uint4 msec_between = 0) = 0;

         /**
          * Defines a hook that can be overloaded to catch the event when a packet has been written
          * in its entirety.  This is intended to make it possible for a derived class to detect
          * packet boundaries of transmitted data without having to parse the stream.
          */
         virtual void on_packet_written()
         { }

         /**
          * Defines a hook that can be overloaded to handle the event where the read buffer has been
          * reset following a timer event, receipt of bytes that could frame a packet, & etc.
          */
         virtual void on_read_buffer_reset()
         { }

         /**
          * Called from time to tome by the port base with the contents of an input buffer.  This
          * method can be overloaded to look for strings like "NO CARRIER" that can be inserted into
          * the stream by devices such as phone modems.
          *
          * @return Returns true if the buffer content indicates a problem.
          */
         virtual bool has_no_carrier(StrBin const &read_buffer)
         { return false; }

         /**
          * Can be overloaded to process debugger log messages that are produced by this class.
          *
          * @param object_name Specifies the objject name for the message.
          *
          * @param log_message Specifies the content of the message.
          */
         virtual void log_debug(char const *object_name, char const *log_message)
         { }

         /**
          * Can be overloaded to log communication status type messages that are produced by this
          * class.
          *
          * @param message Specifies the content of the message.
          *
          * @param is_neutral Set to true if the message should not affect any error rate counter.
          */
         virtual void log_comms_status(char const *message, bool is_neutral = false)
         { }

         /**
          * Can be overloaded to log warning type messages that are produced by this class.
          *
          * @param message Specifies the message content.
          *
          * @param is_neutral Set to true if the message should not affect any error rate counter.
          */
         virtual void log_comms_warning(char const *message, bool is_neutral = false)
         { }

         /**
          * Can be overloaded to log failure type messages that are produced by this class.
          *
          * @param message Specifies the content of the message
          *
          * @param is_neutral Set to true if this message should not affect any error rate counter.
          */
         virtual void log_comms_fault(char const *message, bool is_neutral = false)
         { }

         /**
          * Determines if there is an outgoing message that should be sent on this port and
          * specified physical address.  This version will check wit the router but an overloaded
          * version may also control things like route broadcasts.
          *
          * @return Returns true if there is a message
          *
          * @param message Specifies a reference that will be set to the message that is to be sent.
          *
          * @param physical_destination Specifies the physical destination.
          */
         typedef SharedPtr<Message> message_handle;
         virtual bool get_next_out_message(message_handle &message, uint2 physical_destination);

         /**
          * Must be overloaded to handle the event where the line state has transistion to
          * off-line.  A derived version can do things such as releasing communication resources.
          */
         virtual void on_hanging_up() = 0;

         /**
          * @return Returns true of there is a reason to keep the link alive and there is no
          * compelling reason to get off-line.  This can be overloaded by the derived class but that
          * version should invoke this version if it has no reason to return true.
          */
         virtual bool should_keep_link();

         /**
          * @return Returns true if the link must be closed down now.  This method can be overloaded
          * in order to add reasons to compel the link off-line.
          */
          virtual bool must_close_link()
         { return !comm_enabled; }

         /**
          * Called by the application when communication resources are ready to and the port can
          * start the ringing process.
          */
         virtual void on_link_ready();

         /**
          * Called by the application to report that the link has failed.
          */
         virtual void on_link_failed(); 

         /**
          * Called by the application in order to process the specified received data buffer.
          *
          * @param buffer Specifies the data that has been received.
          *
          * @param buffer_len Specifies the number of bytes in the buffer.
          */
         void on_data_read(void const *buffer, uint4 buffer_len);

         /**
          * Called by the application when the beacon interval for this port has been changed.
          */
         void on_beacon_interval_change();

         /**
          * @return Returns true if any link is in a finished state.
          */
         bool waiting_for_off();

         /**
          * @return Can be overloaded to return the maximum amount if time that should be placed
          * between rink packets in units of milliseconds.
          */
         virtual uint4 get_ring_timeout()
         { return 600; }

         /**
          * Called when a serial packet link has made the transistion to an off-line state.
          */
         virtual void on_link_offline(uint2 physical_destination);

         /**
          * Generates calls to logging methods for the specified message.
          *
          * @param message Specifies the message to describe.
          *
          * @param event_string Specifies specific detail.
          *
          * @param is_neutral Set to true if the logged events should not affect any error counters.
          */
         void describe_message_event(message_handle &message, char const *event_string, bool is_neutral);

         /**
          * @return Can be overloaded to specify that the link timer should be reset when any data
          * has been received.
          */
         virtual bool should_reset_timer_on_bytes() const
         { return false; }

         /**
          * @return Can be overloaded to specify that the underlying link is using TCP/IP.
          */
         virtual bool using_tcp()
         { return false; }

         /**
          * @return Returns true if there is a serial packet link with the specified physical
          * address.
          *
          * @param address Specifies the physical address of the link.
          */
         bool has_link(uint2 address) const;

      protected:
         /**
          * Specifies the router that owns this port.
          */
         Csi::SharedPtr<Router> router;

         /**
          * Specifies the one shot timer.
          */
         Csi::SharedPtr<OneShot> timer;

      private:
         /**
          * Buffers incoming data until packet boundaries are detected or until packet framing rules
          * are violated.
          */
         StrBin read_buffer;

         /**
          * Used to buffer data that needs to be quoted before being transmitted.
          */
         StrBin quote_buffer;

         /**
          * Specifies the current state of the link.
          */
         enum dialed_state_type
         {
            dialed_state_offline,
            dialed_state_waiting,
            dialed_state_online,
            dialed_state_closing
         } dialed_state;

         /**
          * Set to true if the next character received needs to be unquoted.
          */
         bool unquote_next_character;

         /**
          * Identifies the timer used to perform general maintenance for this port.
          */
         uint4 maintenance_id;

         /**
          * Identifies the timer that is used to force a transmission delay between packets.
          */
         uint4 send_delay_id;

         /**
          * Specifies the collection of serial packet links currently active with this port.
          */
         typedef SerialPacketBaseHelpers::link_type link_type;
         typedef Csi::SharedPtr<link_type> link_handle;
         typedef std::map<uint2, link_handle> links_type;
         links_type links;

         /**
          * Identifies the timer used to transmit beacon messages.
          */
         uint4 beacon_id;

         /**
          * Set to true of commumications are enabled for this port.
          */
         bool comm_enabled;

         /**
          * Specifies a collection of messages that are waiting to be broadcast.
          */
         typedef std::list<message_handle> waiting_broadcasts_type;
         waiting_broadcasts_type waiting_broadcasts;

         /**
          * Identifies the timer used to delay the goodbye message before clearing or shutting down
          * the port.
          */
         uint4 close_port_delay_id;

         /**
          * Used to track the amount of time since we last checked for no carrier.  This timer will
          * be reset each time that characters are received and checked by the maintenance timer.
          */
         uint4 check_no_carrier_base;

         /**
          * Used to keep track of the amount of time elapsed since the last valid packet was
          * received.  If the maintenance timer fires and overforty seconds have elapsed, all links
          * will be forced off-line.
          */
         uint4 receive_watchdog_base;

         /**
          * Identifies the timer used to delay the request to open the link for a period of time
          * after comms have been enabled.
          */
         uint4 delay_link_open_id;

         /**
          * Used to track the time elapsed since the send delay timer was first set.  This is needed
          * because the send delay timer can be reset for half-duplex links. This counter will be
          * used to safeguard against situations where a jabbering logger or modem could keep the
          * send delay timer going continuously.
          */
         uint4 send_delay_base;

      protected:
         /**
          * Specifie the interval at which no carrier should be checked.
          */
         static uint4 const check_no_carrier_interval;

         /**
          * Specifies the amount of time in milliseconds that this port will stay in a rady or
          * finished state before assuming that an error has occurred and moving to an offline
          * state.
          */
         static uint4 const link_timeout;

         /**
          * Specifies the interval in milliseconds at which ports will perform their maintenance
          * cycle.
          */
         static uint4 const maintenance_interval;

         /**
          * Specifies the period of time that we shoudl wait before sending the goodbye command and
          * closing the port.
          */
         static uint4 const close_port_delay;

      private:
         /**
          * Called to handle a validated frame in the read buffer.
          */
         void process_incoming_frame();

         /**
          * Exercises the state machine for links based upon the peer state and physical
          * destination.
          */
         void examine_link_state(SerialPacket::link_state_type peer_link_state, uint2 physical_destination);

         /**
          * Called by on_message_ready() or when the link has made a satte transistion.  Will check
          * to see if writing can occur, getting packets from the router, and sending them on the
          * link.
          */
         void on_ready_to_send();

         /**
          * Updates the expect more sessions for the specified pair of addresses.
          */
         void update_expect_more(uint2 source = 0, uint2 destination = 0, bool expect_more = false);

         /**
          * Transmits a ring message to the specified destination.
          */
         void send_ring(uint2 physical_destination, bool first_ring = false);

         /**
          * @return Returns the numebr of messages that are waiting in the router queue to be sent
          * by this port.
          */
         uint4 waiting_to_send_count();

         /**
          * Sends the next beacon message.
          */
         void send_beacon();

         /**
          * cancels any pending timers and any other overhead associated with the hanging up
          * process.
          */
         void do_hanging_up();
         
         friend class SerialPacketBaseHelpers::link_type;
      };
   };
};


#endif
