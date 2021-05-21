/* Csi.TlsCapNegotiator.cpp

   Copyright (C) 2011, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 22 June 2011
   Last Change: Thursday 23 June 2011
   Last Commit: $Date: 2011-06-23 09:54:39 -0600 (Thu, 23 Jun 2011) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.TlsCapNegotiator.h"
#include "Csi.TlsContext.h"


namespace Csi
{
   namespace
   {
      uint4 const event_transmit_caps(
         Event::registerType("Csi::TlsNegotiator::event_transmit_caps"));
      uint4 const event_complete(
         Event::registerType("Csi::TlsCapNegotiator::event_complete"));
   };

   
   ////////////////////////////////////////////////////////////
   // class TlsCapNegotiator definitions
   ////////////////////////////////////////////////////////////
   TlsCapNegotiator::TlsCapNegotiator(
      client_type *client_,
      bool tcp_client_,
      timer_handle timer_):
      client(client_),
      tcp_client(tcp_client_),
      timer(timer_),
      watchdog_id(0),
      transmitted_caps(false),
      peer_tls_client(false),
      peer_tls_server(false),
      peer_pakbus_unquoted(false),
      got_server_caps(false),
      reported_complete(false)
   {
      if(timer == 0)
         timer = new OneShot;
      Event::create(event_transmit_caps, this)->post();
   } // constructor


   TlsCapNegotiator::~TlsCapNegotiator()
   {
      if(timer != 0)
      {
         if(watchdog_id)
            timer->disarm(watchdog_id);
         timer.clear();
      }
   } // destructor

   
   namespace
   {
      byte const synch_byte(0xBE);
      byte const caps_message(0x01);
      byte const start_tls_client_message(0x02);
      byte const cap_tls_client = 0x01;
      byte const cap_tls_server = 0x02;
      byte const cap_pakbus_unquoted = 0x04;
      byte const start_tls_client_command[3] =
      { synch_byte, start_tls_client_message, synch_byte };
   };
   

   void TlsCapNegotiator::onOneShotFired(uint4 id)
   {
      if(id == watchdog_id)
      {
         watchdog_id = 0;
         got_server_caps = true;
         report_complete();
      }
   } // onOneShotFired


   void TlsCapNegotiator::on_data(ByteQueue &buff)
   {
      if(client_type::is_valid_instance(client))
      {
         uint4 start_sync_pos(buff.find(&synch_byte, 1));

         if(start_sync_pos + 2 < buff.size())
         {
            byte message_type(buff[start_sync_pos + 1]);
            if(message_type == caps_message)
            {
               byte peer_caps(buff[start_sync_pos + 2]);
               
               got_server_caps = true;
               if(buff[start_sync_pos + 3] == synch_byte)
               {
                  peer_tls_server = ((peer_caps & cap_tls_server) != 0);
                  peer_tls_client = ((peer_caps & cap_tls_client) != 0);
                  peer_pakbus_unquoted = ((peer_caps & cap_pakbus_unquoted) != 0);
                  buff.pop(start_sync_pos + 4);
               }
               switch(make_outcome())
               {
               case client_type::outcome_tls_server:
                  timer->disarm(watchdog_id);
                  report_complete();
                  break;
                  
               case client_type::outcome_tls_client:
                  // we need to wait for the server to send the start tls command.  we will re-arm
                  // the watch dog to wait for this.
                  timer->reset(watchdog_id);
                  if(buff.size() > 0)
                     on_data(buff);
                  break;
                  
               default:
                  report_complete();
                  break;
               }
            }
            else if(message_type == start_tls_client_message)
            {
               if(buff[start_sync_pos + 2] == synch_byte)
               {
                  buff.pop(3);
                  report_complete();
               }
            }
            else
               report_complete();
         }
         else if(buff.size() > 0)
         {
            got_server_caps = true;
            report_complete();
         }
      }
   } // on_data


   void const *TlsCapNegotiator::get_start_command() const
   { return start_tls_client_command; }


   uint4 TlsCapNegotiator::get_start_command_len() const
   { return sizeof(start_tls_client_command); }


   void TlsCapNegotiator::receive(SharedPtr<Event> &ev)
   {
      if(ev->getType() == event_transmit_caps)
      {
         if(client_type::is_valid_instance(client))
         {
            byte cap_command[4] = { synch_byte, caps_message, 0, synch_byte };
            Csi::TlsContext *context(Csi::SocketTcpSock::get_tls_context());
            
            transmitted_caps = true;
            if(context)
            {
               cap_command[2] = cap_tls_client;
               if(context->get_accepts_client_connections())
                  cap_command[2] += cap_tls_server;
               watchdog_id = timer->arm(this, 10000);
            }
            client->send_data(this, cap_command, sizeof(cap_command));
            report_complete();
         }
      }
      if(ev->getType() == event_complete)
      {
         if(client_type::is_valid_instance(client))
            client->on_complete(this, make_outcome());
      }
   } // receive
   

   void TlsCapNegotiator::report_complete()
   {
      if(client_type::is_valid_instance(client) &&
         transmitted_caps &&
         got_server_caps &&
         !reported_complete)
      {
         if(watchdog_id)
            timer->disarm(watchdog_id);
         reported_complete = true;
         Event::create(event_complete, this)->post();
      }
   } // report_outcome


   TlsCapNegotiatorClient::outcome_type TlsCapNegotiator::make_outcome()
   {
      client_type::outcome_type outcome(client_type::outcome_no_encryption);
      Csi::TlsContext *context(SocketTcpSock::get_tls_context());
      
      if(context)
      {
         if(context->get_accepts_client_connections())
         {
            if(peer_tls_server && !tcp_client)
               outcome = client_type::outcome_tls_client;
            else
               outcome = client_type::outcome_tls_server;
         }
         else
         {
            if(peer_tls_server)
               outcome = client_type::outcome_tls_client;
         }
      }
      return outcome;
   } // make_outcome
};


