/* Csi.PakBus.SerialPacketBase.cpp

   Copyright (C) 2001, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 12 July 2001
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.SerialPacketBase.h"
#include "Csi.PakBus.Router.h"
#include "Csi.PakBus.Message.h"
#include "Csi.PakBus.SerialPacket.h"
#include "Csi.PakBus.SerialDecode.h"
#include "Csi.Utils.h"
#include "Csi.MaxMin.h"
#include "Csi.StrAscStream.h"
#include <iomanip>
#include <assert.h>


namespace Csi
{
   namespace PakBus
   {
      using namespace SerialDecode;

      
      namespace SerialPacketBaseHelpers
      {
         ////////////////////////////////////////////////////////////
         // class address_pair_type
         ////////////////////////////////////////////////////////////
         class address_pair_type
         {
         private:
            ////////////////////////////////////////////////////////////
            // composite
            //
            // Represents the composite of the source and destination addresses with the source
            // address stored in the two most signficant bytes and the destination address stored in
            // the two lest significant bytes.
            ////////////////////////////////////////////////////////////
            uint4 composite;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            address_pair_type(uint2 source = 0, uint2 destination = 0)
            { composite = (static_cast<uint4>(source) << 2) + destination; }

            ////////////////////////////////////////////////////////////
            // less comparison operator
            ////////////////////////////////////////////////////////////
            bool operator <(address_pair_type const &other) const
            { return composite < other.composite; }
         };

         
         ////////////////////////////////////////////////////////////
         // class link_type
         //
         // Encapsulates the state of the link between this port and another neighbour. 
         ////////////////////////////////////////////////////////////
         class link_type: public OneShotClient
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            link_type(
               SerialPacketBase *owner_,
               uint2 physical_destination_);

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~link_type();

            ////////////////////////////////////////////////////////////
            // on_link_ready
            //
            // Called when the port's link has made the transistion from a waiting to an on-line
            // state. 
            ////////////////////////////////////////////////////////////
            void on_link_ready();
            
            ////////////////////////////////////////////////////////////
            // on_message_ready
            //
            // Called when a message has been added to the router queue for the port and physical
            // destination 
            ////////////////////////////////////////////////////////////
            void on_message_ready();

            ////////////////////////////////////////////////////////////
            // on_message_aborted
            ////////////////////////////////////////////////////////////
            void on_message_aborted();

            ////////////////////////////////////////////////////////////
            // onOneShotFired
            ////////////////////////////////////////////////////////////
            virtual void onOneShotFired(uint4 event_id);

            ////////////////////////////////////////////////////////////
            // process_incoming_frame
            //
            // Updates the internal state of this link based upon the contents of the frame.
            ////////////////////////////////////////////////////////////
            void process_incoming_frame(SerialPacket &frame);
            
            ////////////////////////////////////////////////////////////
            // examine_link_state
            ////////////////////////////////////////////////////////////
            void examine_link_state(
               SerialPacket::link_state_type peer_link_state);

            ////////////////////////////////////////////////////////////
            // on_ready_to_send
            //
            // Called when the link is in a state where messages can be safely sent.
            ////////////////////////////////////////////////////////////
            void on_ready_to_send(bool send_if_ringing);

            ////////////////////////////////////////////////////////////
            // should_keep_link
            //
            // Returns true if the link should be left alive.
            ////////////////////////////////////////////////////////////
            bool should_keep_link();

            ////////////////////////////////////////////////////////////
            // on_maintenance_timer
            //
            // Called periodically by the port to check the status of the transactions to determine
            // if the conversation should be carried on.
            ////////////////////////////////////////////////////////////
            void on_maintenance_timer();

            ////////////////////////////////////////////////////////////
            // has_session
            ////////////////////////////////////////////////////////////
            bool has_session(
               uint2 source_address,
               uint2 destination_address);

            ////////////////////////////////////////////////////////////
            // is_online
            //
            // Evaluates through the link_state member whether this link is online.
            ////////////////////////////////////////////////////////////
            bool is_online() const;

            ////////////////////////////////////////////////////////////
            // is_finished
            ////////////////////////////////////////////////////////////
            bool is_finished() const;

            ////////////////////////////////////////////////////////////
            // reset_session_timer
            ////////////////////////////////////////////////////////////
            void reset_session_timer(
               uint2 source,
               uint2 dest);

         private:
            ////////////////////////////////////////////////////////////
            // send_ring
            ////////////////////////////////////////////////////////////
            void send_ring(bool first_ring);

            ////////////////////////////////////////////////////////////
            // update_expect_more
            ////////////////////////////////////////////////////////////
            void update_expect_more(
               uint2 source,
               uint2 destination,
               ExpectMoreCodes::expect_more_code_type expect_more);

            ////////////////////////////////////////////////////////////
            // send_serial_packet
            ////////////////////////////////////////////////////////////
            void send_serial_packet(
               SerialPacket &packet,
               SerialPacket::link_state_type send_link_state);

            ////////////////////////////////////////////////////////////
            // waiting_to_send_count
            ////////////////////////////////////////////////////////////
            uint4 waiting_to_send_count();

            ////////////////////////////////////////////////////////////
            // describe_message_event
            //
            // Posts a comms status log message describing a message received or sent event
            ////////////////////////////////////////////////////////////
            enum message_event_type
            {
               message_event_sending,
               message_event_received,
            };
            void describe_message_event(
               Csi::SharedPtr<Message> &message,
               message_event_type message_event);

            ////////////////////////////////////////////////////////////
            // send_finished
            //
            // Sends a finished packet to the physical destination.  This packet might contain the
            // goodbye message if we must get off line.
            ////////////////////////////////////////////////////////////
            void send_finished();
            
         private:
            ////////////////////////////////////////////////////////////
            // owner
            ////////////////////////////////////////////////////////////
            SerialPacketBase *owner;
            
            ////////////////////////////////////////////////////////////
            // physical_destination
            ////////////////////////////////////////////////////////////
            uint2 physical_destination;

            ////////////////////////////////////////////////////////////
            // watch_dog_timer_id
            //
            // Identifies the 40 second link watch dog timer.  This timer should be reset/armed
            // whenever a message from the physical destination is received. If it fires, the link
            // should be assumed to be invalid.
            ////////////////////////////////////////////////////////////
            uint4 watch_dog_timer_id;

            ////////////////////////////////////////////////////////////
            // ringing_timer_id
            //
            // Measures the delay between each ring attempt until the remote node responds.
            ////////////////////////////////////////////////////////////
            uint4 ringing_timer_id;

            ////////////////////////////////////////////////////////////
            // ringing_retry_count
            //
            // Measures the number of ring attempts that have been made on this node so far.
            ////////////////////////////////////////////////////////////
            uint4 ringing_retry_count;

            ////////////////////////////////////////////////////////////
            // before_finish_id
            //
            // Used to delay the sending of the final finished message enough time to allow devices
            // like the CR200 to clear out their input buffers.
            ////////////////////////////////////////////////////////////
            uint4 before_finish_id;

            ////////////////////////////////////////////////////////////
            // link_state
            //
            // Keeps track of the current state between this node and the remote node identified by
            // the physical_address.  
            ////////////////////////////////////////////////////////////
            enum link_state_type
            {
               link_state_waiting_for_resource,
               link_state_offline,
               link_state_ringing,
               link_state_ready,
               link_state_finished,
            };
            link_state_type link_state;

            ////////////////////////////////////////////////////////////
            // expect_more_addresses
            //
            // Keeps track of timers associated with each session represented by an address pair.
            // Used to help determine when the link should stay on-line.
            ////////////////////////////////////////////////////////////
            typedef std::map<address_pair_type, uint4> expect_more_addresses_type;
            expect_more_addresses_type expect_more_addresses;

            ////////////////////////////////////////////////////////////
            // router
            //
            // Reference to the router with which the owner is associated.  This reference is set
            // in the constructor from the owner's reference.
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<Router> router;

            ////////////////////////////////////////////////////////////
            // timer
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<OneShot> timer;

            ////////////////////////////////////////////////////////////
            // has_been_paused
            //
            // Keeps track of whether this link has been paused.  This member can be set to true if
            // examine_link_state() detects a pause request.  It is set back to false each time that
            // examine_link_state() is invoked.  It is used to determine what value should be
            // assigned to the link watch dog timer. 
            ////////////////////////////////////////////////////////////
            bool has_been_paused;
            
            friend class Csi::PakBus::SerialPacketBase;
         };


         ////////////////////////////////////////////////////////////
         // class event_kill_link
         ////////////////////////////////////////////////////////////
         class event_kill_link: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // link_id
            //
            // Specifies the link that should be deleted.
            ////////////////////////////////////////////////////////////
            uint2 link_id;

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_kill_link(SerialPacketBase *port, uint2 link_id_):
               Event(event_id,port),
               link_id(link_id_)
            { }

         public:
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(SerialPacketBase *port, uint2 link_id)
            {
               try{ (new event_kill_link(port,link_id))->post(); }
               catch(Event::BadPost &) { }
            }
         };


         uint4 const event_kill_link::event_id =
         Event::registerType("Csi::PakBus::SerialPacketBase::event_kill_link");
      };

      
      ////////////////////////////////////////////////////////////
      // class SerialPacketBase definitions
      ////////////////////////////////////////////////////////////
      uint4 const SerialPacketBase::check_no_carrier_interval = 5000;
      uint4 const SerialPacketBase::link_timeout = 40000;
      uint4 const SerialPacketBase::maintenance_interval = 1000;
      uint4 const SerialPacketBase::close_port_delay = 250;
      

      SerialPacketBase::SerialPacketBase(
         router_handle &router_,
         timer_handle &timer_):
         router(router_),
         timer(timer_),
         unquote_next_character(false),
         maintenance_id(0),
         send_delay_id(0),
         dialed_state(dialed_state_offline),
         beacon_id(0),
         comm_enabled(false),
         close_port_delay_id(0),
         check_no_carrier_base(counter(0)),
         receive_watchdog_base(0),
         delay_link_open_id(0),
         send_delay_base(0)
      { }


      SerialPacketBase::SerialPacketBase(timer_handle timer_):
         timer(timer_),
         unquote_next_character(false),
         maintenance_id(0),
         send_delay_id(0),
         dialed_state(dialed_state_offline),
         beacon_id(0),
         comm_enabled(false),
         close_port_delay_id(0),
         check_no_carrier_base(counter(0)),
         delay_link_open_id(0)
      { }

      
      SerialPacketBase::~SerialPacketBase()
      {
         if(beacon_id)
            timer->disarm(beacon_id);
         if(maintenance_id)
            timer->disarm(maintenance_id);
         if(send_delay_id)
            timer->disarm(send_delay_id);
         if(delay_link_open_id)
            timer->disarm(delay_link_open_id);
      } // destructor

      
      void SerialPacketBase::on_message_ready(
         uint2 physical_destination,
         priority_type priority)
      {
         // does a link state record exist for the specified destination?
         links_type::iterator li = links.find(physical_destination);
         link_handle link;
         if(li != links.end())
            link = li->second;
         else
         {
            link.bind(new link_type(this,physical_destination));
            links[physical_destination] = link;
         }
         
         if(dialed_state == dialed_state_offline)
         {
            dialed_state = dialed_state_waiting;
            on_needs_to_dial(priority);
            link->link_state = link_type::link_state_waiting_for_resource;
         }
         else if(dialed_state == dialed_state_waiting)
            link->link_state = link_type::link_state_waiting_for_resource;
         if(dialed_state == dialed_state_online)
            link->on_message_ready(); 
      } // on_message_ready


      void SerialPacketBase::on_message_aborted(uint2 physical_destination)
      {
         // look up the link state record
         links_type::iterator li = links.find(physical_destination);
         if(li != links.end())
            li->second->on_message_aborted();
      } // on_message_aborted


      void SerialPacketBase::broadcast_message(message_handle &message)
      {
         waiting_broadcasts.push_back(message);
         if(dialed_state != dialed_state_waiting)
         {
            if(dialed_state == dialed_state_offline)
            {
               dialed_state = dialed_state_waiting;
               on_needs_to_dial(message->get_priority());
            }
            else
               on_ready_to_send();
         }
      } // broadcast_message


      bool SerialPacketBase::has_session(
         uint2 source_address,
         uint2 destination_address)
      {
         bool rtn = false;
         for(links_type::iterator li = links.begin();
             !rtn && li != links.end();
             ++li)
            rtn = li->second->has_session(source_address,destination_address);
         return rtn;
      } // has_session


      bool SerialPacketBase::link_is_active()
      {
         return dialed_state == dialed_state_online;
      } // link_is_active


      void SerialPacketBase::on_neighbour_lost(uint2 physical_address)
      {
         // look up the associated link
         links_type::iterator li = links.find(physical_address);
         if(li != links.end())
         {
            // we will use this hint to expedite the removal of the link rather than waiting for it
            // to time out.   In order to do this, we need to set the link state to off-line so that
            // the link can be removed when the event is processed.
            li->second->link_state = link_type::link_state_offline;
            li->second->expect_more_addresses.clear();
            on_link_offline(physical_address);
         }
      } // on_neighbour_lost


      void SerialPacketBase::reset_session_timer(
         uint2 source,
         uint2 dest)
      {
         for(links_type::iterator li = links.begin();
             li != links.end();
             ++li)
         {
            li->second->reset_session_timer(source,dest);
         }
      } // reset_session_timer

      
      void SerialPacketBase::onOneShotFired(uint4 timer_id)
      {
         if(timer_id == maintenance_id)
         {
            // iterate through the links and prune those that are in an off-line state
            using namespace SerialPacketBaseHelpers;
            maintenance_id = timer->arm(this,maintenance_interval);
            if(!links.empty())
            {
               // check to see if we should check the read buffer for a no carrier signal
               if(counter(check_no_carrier_base) >= check_no_carrier_interval &&
                   read_buffer.length() > 0 &&
                   has_no_carrier(read_buffer))
               {
                  log_comms_fault("carrier lost");
                  on_link_failed();
                  on_hanging_up();
               }
               else if(link_is_dialed() &&
                       counter(receive_watchdog_base) > link_timeout * 10)
               {
                  log_comms_fault("link timed out");
                  on_link_failed();
                  on_hanging_up();
               }
               else
               {
                  for(links_type::iterator li = links.begin();
                      li != links.end();
                      ++li)
                  {
                     using namespace SerialPacketBaseHelpers;
                     if(li->second->link_state == link_type::link_state_offline &&
                        !li->second->should_keep_link())
                        event_kill_link::create_and_post(this,li->first);
                     else
                        li->second->on_maintenance_timer();
                  }
               }
            }
            else if(must_close_link() ||
                    (link_is_dialed() && !should_keep_link()))
               do_hanging_up();
         }
         else if(timer_id == send_delay_id)
         {
            send_delay_id = send_delay_base = 0;
            on_ready_to_send();
         }
         else if(timer_id == beacon_id)
         {
            beacon_id = 0;
            send_beacon();
         }
         else if(timer_id == close_port_delay_id)
         {
            close_port_delay_id = 0;
            dialed_state = dialed_state_offline;
            on_hanging_up();
         }
         else if(timer_id == delay_link_open_id)
         {
            delay_link_open_id = 0;
            if(dialed_state == dialed_state_offline &&
               comm_enabled &&
               !link_is_dialed())
            {
               on_needs_to_dial(Priorities::low);
            }
         }
      } // onOneShotFired


      void SerialPacketBase::receive(SharedPtr<Event> &ev)
      {
         using namespace SerialPacketBaseHelpers;
         if(ev->getType() == event_kill_link::event_id)
         {
            // we need to look up the link that was specified in the event
            event_kill_link *event = static_cast<event_kill_link *>(ev.get_rep());
            links_type::iterator li = links.find(event->link_id);

            if(li != links.end())
            {
               // we don't want to remove this link unless it is in an off-line state.  Conditions
               // might have change while we were waiting for this event.
               if(!li->second->should_keep_link())
                  links.erase(li);
            }

            // this link may have been in a finished state and, as such, would have kept other links
            // from transmitting.  Because it is now closing down, we will inform the other links
            // that they now have an opportunity to transmit.
            for(li = links.begin(); li != links.end(); ++li)
               li->second->on_ready_to_send(false);

            // we might need to close down now
            if(links.empty() &&
               ((!should_keep_link() && link_is_dialed()) || must_close_link()))
               do_hanging_up();
         }
      } // receive


      bool SerialPacketBase::get_next_out_message(
         SharedPtr<Message> &message,
         uint2 physical_destination)
      {
         // if we are already waiting for the send delay, we will get no more messages from the
         // router at this time.
         bool rtn = false;
         if(send_delay_id == 0)
         {
            rtn = router->get_next_port_message(
               this,physical_destination,message);
            if(rtn)
            {
               uint4 delay_interval = 5;
               if(should_reset_timer_on_bytes() && message->get_expected_response_interval())
                  delay_interval = message->get_expected_response_interval();
               send_delay_id = timer->arm(this,delay_interval);
               send_delay_base = counter(0);
            }
            else
            {
               log_debug(
                  "SerialPacketBase::get_next_out_message",
                  "no message is waiting");
            }
         }
         else
         {
            log_debug(
               "SerialPacketBase::get_next_out_message",
               "send delay is pending");
         }
         if(!rtn)
            message.clear(); 
         return rtn;
      } // get_next_out_message


      bool SerialPacketBase::should_keep_link()
      {
         bool rtn = comm_enabled &&
            router->port_is_needed(this) &&
            !waiting_broadcasts.empty();

         for(links_type::iterator li = links.begin();
             !rtn && li != links.end();
             ++li)
            rtn = li->second->should_keep_link();
         if(rtn)
            rtn = !must_close_link();
         return rtn;
      } // should_keep_link

      
      void SerialPacketBase::on_link_ready()
      {
         // initialise this port
         dialed_state = dialed_state_online;
         maintenance_id = timer->arm(this,maintenance_interval);
         receive_watchdog_base = counter(0);

         // we need to notify the router that this port has entered an on-line
         router->on_port_ready(this);

         // notify all link objects that the port is ready to write
         for(links_type::iterator li = links.begin();
             li != links.end();
             ++li)
            li->second->on_link_ready();

         // we want to make sure that a beacon will go out.  We will do so by setting the timer
         // (assuming that it already isn't set to a short interval.  This will give other
         // things a chance to go out before the beacon intrudes itself.
         if(beacon_id != 0)
            timer->disarm(beacon_id);
         if(get_beacon_interval() != 0xffff)
            beacon_id = timer->arm(this,250); 
         
         // check to see if there are messages that should be broadcast
         while(!waiting_broadcasts.empty())
         {
            // since the link is newly initialised, we need to send a sequence of synch bytes to
            // make sure that the message will be received.
            if(!using_tcp())
               write_data(&synch_byte, 1, 6, 35);

            // form the serial packet to be sent
            message_handle message(waiting_broadcasts.front());
            waiting_broadcasts.pop_front();
            SerialPacket packet(*message);
            send_serial_packet(
               packet,
               Router::broadcast_address,
               SerialPacket::link_state_off_line);
         }
      } // on_link_ready


      void SerialPacketBase::on_link_failed()
      {
         // report debug log messages indicating entry and exit of this method
         log_debug("Csi::PakBus::SerialPacketBase::on_link_failed","entering method");
         
         // destroy all of the remaining link records
         links.clear();
         router->on_port_delivery_failure(this);

         // make sure that the timers are cancelled
         dialed_state = dialed_state_offline;
         if(maintenance_id)
            timer->disarm(maintenance_id);
         if(send_delay_id)
            timer->disarm(send_delay_id);
         maintenance_id = send_delay_id = send_delay_base = 0;

         // even though everything else is ending, we still need to make sure that the beacon will
         // be carried.  This will give us an opportunity to retry the connection at some future
         // event.
         if(beacon_id == 0 && get_beacon_interval() != 0xffff)
            beacon_id = timer->arm(this,get_beacon_interval()*1000); 
         read_buffer.cut(0);
         on_read_buffer_reset();
         log_debug("Csi::PakBus::SerialPacketBase::on_link_failed","leaving method");
      } // on_link_failed


      void SerialPacketBase::on_data_read(
         void const *buffer,
         uint4 buffer_len)
      {
         uint4 bytes_read = 0;
         byte const *buff = static_cast<byte const *>(buffer);
         decode_outcome_type decode_outcome;
         
         check_no_carrier_base = counter(0);
         if(should_reset_timer_on_bytes())
         {
            receive_watchdog_base = counter(0);
            for(links_type::iterator li = links.begin();
                li != links.end();
                ++li)
               timer->reset(li->second->watch_dog_timer_id);
            if(send_delay_id != 0)
               timer->reset(send_delay_id);
            else
            {
               send_delay_id = timer->arm(this,get_worst_case_response() / 2);
               send_delay_base = counter(0);
            }
            if(send_delay_base != 0 && counter(send_delay_base) >= 60000)
            {
               timer->disarm(send_delay_id);
               send_delay_base = 0;
            }
         }
         while(bytes_read < buffer_len)
         {
            // decode the next segment.  If a synch byte was found, it should appear at the end of
            // the read buffer after decoding is finished.
            bytes_read += decode_quoted_data( 
               read_buffer,
               unquote_next_character,
               decode_outcome,
               buff + bytes_read,
               buffer_len - bytes_read);
            if(decode_outcome == decode_synch_found)
            {
               // if the synch byte was found and there are no more bytes to process, we will clear
               // the send delay timer so that more data can be sent.
               if(bytes_read >= buffer_len && send_delay_id != 0)
               {
                  timer->disarm(send_delay_id);
                  send_delay_base = 0;
               }
               
               // the presence of the serial synch byte indicates the possibility that a legal
               // packet is now stored in the read buffer. The contents will be considered only if
               // there are enough bytes in the buffer for a packet header.
               if(read_buffer.length() >= SerialPacket::min_header_len + 2)
               {
                  // the signature for the entire frame must be zero (due to presence of the
                  // signature nullifiers at the end of the packet.
                  if(calcSigFor(read_buffer.getContents(),read_buffer.length()) == 0)
                     process_incoming_frame();
                  else
                  {
                     if(has_no_carrier(read_buffer))
                     {
                        log_comms_fault("link lost");
                        on_link_failed();
                        on_hanging_up();
                     }
                     else
                        log_comms_fault("PakBus framing error\",\"Invalid low level signature");
                  }
                  read_buffer.cut(0);
                  on_read_buffer_reset();
               }
               else
               {
                  if(has_no_carrier(read_buffer))
                  {
                     log_comms_fault("link lost");
                     on_link_failed();
                     on_hanging_up();
                  }
                  else
                  {
                     read_buffer.cut(0);
                     on_read_buffer_reset();
                  }
               }
            }
            else if(decode_outcome == decode_quote_error ||
                    decode_outcome == decode_packet_too_long)
            {
               if(has_no_carrier(read_buffer))
               {
                  log_comms_fault("link lost");
                  on_link_failed();
                  on_hanging_up();
               }
               else
               {
                  log_comms_fault("PakBus framing error\",\"frame is misquoted");
                  read_buffer.cut(0);
                  on_read_buffer_reset();
               }
            }
         }
      } // on_data_read


      void SerialPacketBase::on_comm_enabled_change(bool comm_enabled_)
      {
         // we will only pay attention to this event if the state has trully changed.
         if(comm_enabled != comm_enabled_)
         {
            // if communications have become enabled and this is not a dialed link, the settings
            // seem to indicate that we must claim the link.  We have encountered problems in the
            // server, however, where this method can be called before all of the settings are
            // finalised and we wind up claiming the wrong link.  In order to prevent this, we will
            // set a timer for a fairly liberal time to delay claiming the link with the hope that
            // the dust will be settled by the time the timer fires.
            comm_enabled = comm_enabled_;
            if(comm_enabled &&
               dialed_state == dialed_state_offline &&
               delay_link_open_id == 0 &&
               !link_is_dialed())
            {
               delay_link_open_id = timer->arm(this,15000);
            }
         }
      } // on_comm_enabled_change


      void SerialPacketBase::on_beacon_interval_change()
      {
         if(comm_enabled)
         {
            if(beacon_id != 0)
               timer->disarm(beacon_id);
            beacon_id = timer->arm(this, 250);
         }
      } // on_beacon_interval_change


      bool SerialPacketBase::waiting_for_off()
      {
         bool rtn = false;
         for(links_type::iterator li = links.begin();
             li != links.end() && !rtn;
             ++li)
            rtn = li->second->is_finished();
         return rtn;
      } // waiting_for_off

      
      void SerialPacketBase::process_incoming_frame()
      {
         // construct a serial packet using all of the contents of the read buffer except for the
         // last two bytes (the signature nullifier). This packet will not outlive this method
         // invocation so it is OK for us to "borrow" the storage from the read buffer.
         SerialPacket frame(
            read_buffer.getContents(),
            (uint4)read_buffer.length() - 2,
            false);

         // we will delegate to the should_process_message() method to determine if this frame
         // should be ignored.
         if(should_process_message(
               frame.get_source_physical_address(),
               frame.get_destination_physical_address(),
               router.get_rep()))
         {
            // if the frame has the source address, then
            // does a link record exist for the node that originated this frame?
            receive_watchdog_base = counter(0);
            links_type::iterator li = links.find(frame.get_source_physical_address());
            link_handle link;
            if(li == links.end())
            {
               link.bind(new link_type(this,frame.get_source_physical_address()));
               if(frame.get_link_state() == SerialPacket::link_state_ring ||
                  frame.get_link_state() == SerialPacket::link_state_ready ||
                  frame.get_link_state() == SerialPacket::control_capabilities)
                  link->link_state = link_type::link_state_ready;
               else
                  link->link_state = link_type::link_state_offline;
               links[frame.get_source_physical_address()] = link;
            }
            else
               link = li->second;

            // this frame will be treated as a beacon if it contains a full header and its hop count
            // is equal to one
            if(frame.get_headerLen() == SerialPacket::max_header_len)
            {
               router->on_beacon(
                  this,
                  frame.get_source_physical_address(),
                  frame.get_destination_physical_address() == router->broadcast_address);
            }
            
            // we can now allow the link object to process the frame
            link->process_incoming_frame(frame);
         }
      } // process_incoming_frame


      void SerialPacketBase::send_serial_packet(
         SerialPacket &packet,
         uint2 destination_physical_address,
         SerialPacket::link_state_type link_state)
      {
         // calculate the signature nullifier for the serial packet and append it.
         packet.set_destination_physical_address(destination_physical_address);
         packet.set_source_physical_address(router->get_this_node_address());
         packet.set_link_state(link_state);
         packet.addUInt2(
            calcSigNullifier(
               calcSigFor(packet.getMsg(),packet.length())),
            !is_big_endian());

         // write the quoted version of the packet
         byte const *message_src = reinterpret_cast<byte const *>(packet.getMsg());
         quote_buffer.cut(0);
         quote_buffer.append(&synch_byte, 1);
         for(uint4 i = 0; i < packet.length(); ++i)
         {
            byte temp = message_src[i];
            if(temp == synch_byte || temp == quote_byte)
            {
               quote_buffer.append(&quote_byte, 1);
               temp += 0x20;
            }
            quote_buffer.append(&temp, 1);
         }
         quote_buffer.append(&synch_byte, 1);
         write_data(quote_buffer.getContents(), (uint4)quote_buffer.length());
         on_packet_written();
      } // send_serial_packet


      bool SerialPacketBase::has_link(uint2 address) const
      {
         links_type::const_iterator li(links.find(address));
         return li != links.end();
      } // has_link


      void SerialPacketBase::on_ready_to_send()
      {
         // the maintenance timer should be started if it isn't already
         if(!maintenance_id)
            maintenance_id = timer->arm(this,maintenance_interval);

         // if there are any waiting broadcasts, they should be sent now
         while(!waiting_broadcasts.empty())
         {
            if(!using_tcp())
               write_data(&synch_byte, 1, 6, 35);
            message_handle message(waiting_broadcasts.front());
            waiting_broadcasts.pop_front();
            describe_message_event(
               message,
               "broadcasting",
               false);
            SerialPacket packet(*message);
            send_serial_packet(
               packet,
               Router::broadcast_address,
               SerialPacket::link_state_off_line);
         }
         
         // check to make sure that there is something to send
         if(waiting_to_send_count() > 0)
         {
            // we will iterate through all of the links until a packet is sent
            for(links_type::iterator li = links.begin();
                send_delay_id == 0 && li != links.end();
                ++li)
               li->second->on_ready_to_send(false);
         }
      } // on_ready_to_send


      uint4 SerialPacketBase::waiting_to_send_count()
      { return router->count_messages_for_port(this, 0) + (uint4)waiting_broadcasts.size();  }


      void SerialPacketBase::on_link_offline(uint2 physical_destination)
      { 
         using namespace SerialPacketBaseHelpers;
         event_kill_link::create_and_post(this,physical_destination);
      } // on_link_offline


      void SerialPacketBase::describe_message_event(
         message_handle &message,
         char const *event_string,
         bool is_neutral)
      {
         OStrAscStream comms_message;
         comms_message << event_string << "\",\"";
         message->describe_message(comms_message);
         log_comms_status(
            comms_message.str().c_str(),
            is_neutral);
      } // describe_message_event


      void SerialPacketBase::send_beacon()
      {
         // we will do nothing if the beacon interval is set to infinity
         uint2 beacon_interval = get_beacon_interval(); 
         if(beacon_interval != 0xffff) 
         {
            // we need to evaluate while doing this what our new beacon interval shall be.  by
            // default, it will be the same as the setting.  However, if we are waiting for the dial
            // or we are waiting for the links to leave a finished state, we might accellerate the
            // schedule.
            uint4 next_beacon_interval = static_cast<uint4>(beacon_interval)*1000;
            
            // we need to be on-line in order to beacon
            if(dialed_state == dialed_state_online || dialed_state == dialed_state_closing)
            {
               // we shouldn't send the beacon if any of the links are in a finished state.  We also
               // need to evaluate whether the baud rate synchronisation characters should be sent.
               bool send_synch_bytes = true;
               bool send_the_beacon = dialed_state == dialed_state_online;
               
               for(links_type::iterator li = links.begin();
                   li != links.end() && send_synch_bytes && send_the_beacon;
                   ++li)
               {
                  send_synch_bytes = !li->second->is_online();
                  send_the_beacon = !li->second->is_finished();
               }

               if(send_the_beacon)
               {
                  // we need to make sure that the baud rate gets synched
                  if(send_synch_bytes && !using_tcp())
                     write_data(&synch_byte, 1, 6, 35);
                  
                  // form the beacon and send it.
                  SerialPacket beacon(SerialPacket::max_header_len);
                  OStrAscStream comms_message;

                  comms_message << "sending beacon\",\""
                                << "src: " << router->get_this_node_address() << "\",\""
                                << "dest: " << Router::broadcast_address << "\",\""
                                << "proto: PakCtrl\",\"empty";
                  log_comms_status(
                     comms_message.str().c_str(),
                     true);
                  beacon.set_expect_more(ExpectMoreCodes::neutral);
                  beacon.set_destination(Router::broadcast_address);
                  beacon.set_source(router->get_this_node_address());
                  send_serial_packet(
                     beacon,Router::broadcast_address,
                     SerialPacket::link_state_off_line);
                  if(send_delay_id == 0)
                  {
                     send_delay_id = timer->arm(this,100);
                     send_delay_base = counter(0);
                  }
               }
               else
                  // we will wait 1/4 second to see if the condition clears up.
                  next_beacon_interval = 250;
            }
            else if(dialed_state != dialed_state_waiting && !link_is_dialed())
            {
               dialed_state = dialed_state_waiting;
               on_needs_to_dial(Priorities::low);
               next_beacon_interval = 0;
            }

            // re-arm the beacon timer
            if(next_beacon_interval > 0)
               beacon_id = timer->arm(
                  this,
                  static_cast<uint4>(get_beacon_interval()) * 1000); 
         } 
      } // send_beacon


      void SerialPacketBase::do_hanging_up()
      {
         if(close_port_delay_id == 0)
         {
            if(maintenance_id)
            {
               timer->disarm(maintenance_id);
               maintenance_id = 0;
            }
            if(send_delay_id)
            {
               timer->disarm(send_delay_id);
               send_delay_id = send_delay_base = 0;
            }
            links.clear();
            waiting_broadcasts.clear();
            dialed_state = dialed_state_offline;
            read_buffer.cut(0);
            on_read_buffer_reset();
            on_hanging_up();
         }
      } // do_hanging_up


      namespace SerialPacketBaseHelpers
      {
         ////////////////////////////////////////////////////////////
         // class link_type definitions
         ////////////////////////////////////////////////////////////
         link_type::link_type(
            SerialPacketBase *owner_,
            uint2 physical_destination_):
            link_state(link_state_offline),
            ringing_retry_count(0),
            ringing_timer_id(0),
            watch_dog_timer_id(0),
            before_finish_id(0),
            owner(owner_),
            physical_destination(physical_destination_),
            has_been_paused(false)
         {
            router = owner->router;
            timer = owner->timer;
            assert(router.get_reference_count() > 1);
         } // constructor


         link_type::~link_type()
         {
            if(timer != 0)
            {
               if(watch_dog_timer_id)
                  timer->disarm(watch_dog_timer_id);
               if(ringing_timer_id)
                  timer->disarm(ringing_timer_id);
               if(before_finish_id)
                  timer->disarm(before_finish_id);
            }
         } // destructor


         void link_type::on_link_ready()
         {
            if(link_state == link_state_waiting_for_resource)
               send_ring(true);
         } // on_link_ready


         void link_type::on_message_ready()
         {
            link_state_type old_link_state = link_state;
            
            if(link_state == link_state_ready)
               on_ready_to_send(false);
            else if(link_state == link_state_offline || link_state == link_state_ringing)
               send_ring(true);
         } // on_message_ready


         void link_type::on_message_aborted()
         {
            link_state_type old_link_state = link_state;
            if(waiting_to_send_count() == 0)
            {
               if(!should_keep_link())
               {
                  if(link_state == link_state_waiting_for_resource)
                     owner->on_link_offline(physical_destination);
                  else if(link_state != link_state_offline && link_state != link_state_finished)
                     send_finished(); 
               }
            }
         } // on_message_aborted

         
         void link_type::onOneShotFired(uint4 timer_id)
         {
            if(timer_id == ringing_timer_id)
            {
               ringing_timer_id = 0;
               if(waiting_to_send_count() > 0)
               {
                  if(link_state == link_state_ringing && ++ringing_retry_count <= 4)
                     send_ring(false);
                  else if(ringing_retry_count >= 4)
                  {
                     link_state = link_state_offline;
                     router->on_port_delivery_failure(owner,physical_destination);
                     owner->on_link_offline(physical_destination);
                  }
               }
               else
               {
                  link_state = link_state_offline;
                  owner->on_link_offline(physical_destination);
               }
            }
            else if(timer_id == watch_dog_timer_id)
            {
               watch_dog_timer_id = 0;
               link_state = link_state_offline;
               owner->on_link_offline(physical_destination);
            }
            else if(timer_id == before_finish_id)
            {
               before_finish_id = 0;
               if(link_state == link_state_finished)
               {
                  SerialPacket empty;
                  send_serial_packet(empty,SerialPacket::link_state_finished);
               }
            }
         } // onOneShotFired

         
         void link_type::process_incoming_frame(SerialPacket &frame)
         {
            // a short frame will not have anything to be routed either
            if(frame.get_headerLen() == SerialPacket::max_header_len)
            {
               // create a message object to pass to the router
               SharedPtr<Message> message(frame.make_pakbus_message());
               message->set_port(owner);
               describe_message_event(message,message_event_received);
               if(message->get_destination() == Router::broadcast_address)
                  message->set_destination(router->get_this_node_address());
               if(frame.get_link_state() != SerialPacket::control_capabilities)
               {
                  update_expect_more(
                     message->get_source(),
                     message->get_destination(),
                     message->get_expect_more());
               }

               // we need to make note of the message receipt in the comms log
               router->on_port_message(owner,message);
            }
            
            // we need to examine the link state bits in the frame as that will affect the state
            // of this link
            if(frame.get_destination_physical_address() != Router::broadcast_address)
            {
               examine_link_state(frame.get_link_state());

               if(link_state != link_state_offline)
               {
                  // we should also use this opportunity to reset the watch dog timer (or set it if
                  // it doesn't exist)
                  if(watch_dog_timer_id)
                     timer->reset(watch_dog_timer_id);
                  else
                     watch_dog_timer_id = timer->arm(this,owner->link_timeout);
               }
            }
         } // process_incoming_frame


         void link_type::examine_link_state(
            SerialPacket::link_state_type peer_link_state)
         {
            // this method is called when we have received a frame from the peer. Because of this,
            // we should discontinue the ringing schedule because we now have the logger's
            // attention.
            if(ringing_timer_id)
               timer->disarm(ringing_timer_id);
            has_been_paused = false;
            
            // now we can examine the state and react to it if needed
            switch(peer_link_state)
            {
            case SerialPacket::link_state_off_line:
               link_state = link_state_offline;
               owner->on_link_offline(physical_destination);
               break;
               
            case SerialPacket::link_state_ring:
               if(link_state == link_state_offline ||
                  link_state == link_state_ringing ||
                  link_state == link_state_finished ||
                  link_state == link_state_ready)
               {
                  owner->log_debug("examine_link_state","rung by peer"); 
                  link_state = link_state_ready;
                  if(waiting_to_send_count() == 0)
                  {
                     SerialPacket empty;
                     send_serial_packet(empty,SerialPacket::link_state_ready);
                  }
                  else
                  {
                     link_state = link_state_ready;
                     on_ready_to_send(false);
                  }
               }
               break;
               
            case SerialPacket::link_state_ready:
               if(link_state == link_state_ready && !should_keep_link())
                  send_finished();
               else if(
                  link_state == link_state_offline ||
                  link_state == link_state_ringing ||
                  link_state == link_state_finished)
               {
                  // we are being rung by the logger
                  link_state = link_state_ready;
                  on_ready_to_send(false);
               }
               else if(link_state == link_state_finished && !should_keep_link())
               {
                  SerialPacket empty;
                  send_serial_packet(empty, SerialPacket::link_state_off_line);
                  link_state = link_state_offline;
                  owner->on_link_offline(physical_destination);
               }
               break;
               
            case SerialPacket::link_state_finished:
               if(should_keep_link())
               {
                  if(waiting_to_send_count() > 0)
                  {
                     link_state = link_state_ringing;
                     ringing_retry_count = 0;
                     on_ready_to_send(true);
                  }
                  else
                  {
                     ringing_retry_count = 0;
                     send_ring(false);
                  }
               }
               else
               {
                  SerialPacket empty;
                  owner->send_serial_packet(
                     empty,
                     physical_destination,
                     SerialPacket::link_state_off_line);
                  if(watch_dog_timer_id)
                  {
                     timer->disarm(watch_dog_timer_id);
                     watch_dog_timer_id = 0;
                  }
                  link_state = link_state_offline;
                  owner->on_link_offline(physical_destination);
               }
               break;
               
            case SerialPacket::link_state_pause:
               if(link_state != link_state_finished)
               {
                  SerialPacket empty;
                  has_been_paused = true;
                  link_state = link_state_finished;
                  send_serial_packet(empty,SerialPacket::link_state_finished);
               }
               else if(link_state == link_state_offline)
               {
                  SerialPacket empty;
                  send_serial_packet(empty,SerialPacket::link_state_off_line);
               }
               break; 
            }
         } // examine_link_state


         void link_type::on_ready_to_send(bool send_if_ringing)
         {
            bool ok_to_send = true;
            if(waiting_to_send_count() == 0)
            {
               ok_to_send = false;
            }
            else if(link_state != link_state_ready &&
                    !(link_state == link_state_ringing && send_if_ringing))
            {
               static char const *link_state_names[] = {
                  "waiting for resource",
                  "offline",
                  "ringing",
                  "ready",
                  "finished"
               };
               OStrAscStream temp;
               temp << "link state not ready\",\"" << link_state_names[link_state];
               owner->log_debug(
                  "SerialPacketBase::link_type::on_ready_to_send",
                  temp.str().c_str());
               ok_to_send = false;
               if(link_state != link_state_finished && ringing_timer_id == 0)
                  send_ring(true);
            }
            else if(owner->waiting_for_off())
            {
               owner->log_debug(
                  "SerialPacketBase::link_type::on_ready_to_send",
                  "waiting for off-line");
               ok_to_send = false;
            } 
            if(ok_to_send)
            {
               // get the next message for the port
               Csi::SharedPtr<Message> message;
               if(owner->get_next_out_message(message,physical_destination))
               {
                  // update the expect more list based upon the message contents
                  update_expect_more(
                     message->get_source(),
                     message->get_destination(),
                     message->get_expect_more());

                  // we can now determine what our new link state will be and what we will report to
                  // the link peer
                  SerialPacket::link_state_type reported_link_state;
                  link_state_type old_link_state = link_state;

                  if(should_keep_link())
                  {
                     link_state = link_state_ready;
                     reported_link_state = SerialPacket::link_state_ready;
                  }
                  else
                  {
                     reported_link_state = SerialPacket::link_state_finished;
                     link_state = link_state_finished; 
                  }

                  // we need to log a comms message when the message is sent
                  describe_message_event(message,message_event_sending);

                  // form the serial frame to send the message
                  SerialPacket temp(*message);
                  send_serial_packet(temp,reported_link_state);
               }
            }
         } // on_ready_to_send


         bool link_type::should_keep_link()
         {
            bool rtn = waiting_to_send_count() > 0;
            expect_more_addresses_type::iterator emi = expect_more_addresses.begin();

            while(!rtn && emi != expect_more_addresses.end())
            {
               if(Csi::counter(emi->second) > owner->link_timeout)
               {
                  expect_more_addresses_type::iterator demi = emi++;
                  expect_more_addresses.erase(demi);
               }
               else
                  rtn = true;
            }
            if(rtn)
               rtn = !owner->must_close_link();
            return rtn;
         } // should_keep_link


         void link_type::on_maintenance_timer()
         {
            if(!should_keep_link())
            {
               if(link_state == link_state_ready)
                  send_finished();
               else if(link_state == link_state_offline || link_state == link_state_ringing)
               {
                  if(ringing_timer_id)
                     timer->disarm(ringing_timer_id);
                  link_state = link_state_offline;
                  owner->on_link_offline(physical_destination);
               }
            }
            else if(link_state == link_state_ready)
               on_ready_to_send(false);
            else if(link_state == link_state_offline)
               send_ring(true);
         } // on_maintenance_timer


         bool link_type::has_session(
            uint2 source_address,
            uint2 destination_address)
         {
            using namespace SerialPacketBaseHelpers;
            expect_more_addresses_type::iterator emi =
               expect_more_addresses.find(
                  address_pair_type(source_address,destination_address));
            return emi != expect_more_addresses.end();
         } // has_session


         bool link_type::is_online() const
         {
            return link_state != link_state_waiting_for_resource &&
               link_state != link_state_offline;
         } // is_online


         bool link_type::is_finished() const
         { return link_state == link_state_finished; }


         void link_type::reset_session_timer(
            uint2 source,
            uint2 dest)
         {
            expect_more_addresses_type::iterator ei = expect_more_addresses.find(
               address_pair_type(source,dest));
            if(ei != expect_more_addresses.end())
               ei->second = counter(0);
         } // reset_session_timer


         void link_type::send_ring(bool first_ring)
         {
            if(ringing_timer_id == 0)
            {
               // if this is the first ring, we need to send the baud rate synchronisation characters
               // as well.
               link_state = link_state_ringing;
               if(first_ring)
               {
                  ringing_retry_count = 0;
                  if(!owner->using_tcp())
                     owner->write_data(&synch_byte, 1, 5, 35);
               }
               
               // send the packet
               OStrAscStream log;
               SerialPacket empty;
               
               send_serial_packet(empty,SerialPacket::link_state_ring);
               log << "remote: " << physical_destination << "\",\""
                   << "retries: " << ringing_retry_count;
               owner->log_debug(
                  "send_ring",
                  log.str().c_str());
               ringing_timer_id = timer->arm(
                  this,
                  Csi::csimin(
                     static_cast<uint4>(10000),
                     Csi::csimax(
                        owner->get_ring_timeout(),
                        static_cast<uint4>(600))));
            }
            else
            {
               OStrAscStream log;
               log << "remote: " << physical_destination << "\",\""
                   << "retries: " << ringing_retry_count;
               owner->log_debug(
                  "send_ring",
                  log.str().c_str());
            } 
         } // send_ring


         void link_type::update_expect_more(
            uint2 source,
            uint2 destination,
            ExpectMoreCodes::expect_more_code_type expect_more)
         {
            // we shouldn't update anything if either of the addresses is a broadcast address.  We
            // also should ignore the event if the expect more field indicates neutrality
            if(source != Router::broadcast_address &&
               destination != Router::broadcast_address &&
               expect_more != ExpectMoreCodes::neutral)
            {
               address_pair_type key(source,destination);
               if(expect_more == ExpectMoreCodes::expect_more)
                  expect_more_addresses[key] = counter(0);
               else if(expect_more == ExpectMoreCodes::last)
                  expect_more_addresses.erase(key);
               else if(expect_more == ExpectMoreCodes::expect_more_opposite)
               {
                  address_pair_type opposite_key(destination,source);
                  expect_more_addresses[opposite_key] = counter(0);
                  expect_more_addresses.erase(key);
               }
            }
         } // update_expect_more
         

         void link_type::send_serial_packet(
            SerialPacket &packet,
            SerialPacket::link_state_type send_link_state)
         {
            // send the packet
            owner->send_serial_packet(packet,physical_destination,send_link_state);

            // we need to arm/set/reset the watch dog timer for the link.  If the link is in a
            // finished state, we will use a shorter value for the timeout.
            uint4 timeout_interval = owner->link_timeout;

            if(watch_dog_timer_id)
               timer->disarm(watch_dog_timer_id);
            if(link_state != link_state_offline)
            {
               OStrAscStream msg;
               
               if(link_state == link_state_finished)
                  timeout_interval = 5000;
               msg << "watch dog timeout set at " << timeout_interval;
               owner->log_debug(
                  "Csi::PakBus::SerialPacketBase::link_type",
                  msg.str().c_str());
               watch_dog_timer_id = timer->arm(this,timeout_interval);
            }
         } // send_serial_packet


         uint4 link_type::waiting_to_send_count()
         {
            return
               router->count_messages_for_port(
                  owner,physical_destination);
         } // waiting_to_send_count


         void link_type::describe_message_event(
            Csi::SharedPtr<Message> &message,
            message_event_type message_event)
         {
            static char const *message_event_strings[] = {
               "sending message",
               "received message"
            };
            owner->describe_message_event(
               message,
               message_event_strings[message_event],
               message_event == message_event_sending);
         } // describe_message_event


         void link_type::send_finished()
         {
            if(before_finish_id == 0)
            {
               if(!owner->must_close_link())
               {
                  link_state = link_state_finished;
                  if(owner->send_delay_id == 0)
                     before_finish_id = timer->arm(this,1000);
                  else
                     link_state = link_state_ready;
               }
               else if(owner->send_delay_id == 0)
               {
                  SerialPacket empty;
                  link_state = link_state_offline;
                  send_serial_packet(empty,SerialPacket::link_state_off_line);
                  owner->on_link_offline(physical_destination);
               }
            }
         } // send_finished
      };
   };
};
