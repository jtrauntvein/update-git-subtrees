/* Csi.TlsCapNegotiator.h

   Copyright (C) 2011, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Wednesday 22 June 2011
   Last Change: Wednesday 22 June 2011
   Last Commit: $Date: 2011-06-23 09:54:39 -0600 (Thu, 23 Jun 2011) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_TlsCapNegotiator_h
#define Csi_TlsCapNegotiator_h

#include "Csi.ByteQueue.h"
#include "OneShot.h"
#include "Csi.Events.h"


namespace Csi
{
   // @group: class forward declarations
   class TlsCapNegotiator;
   // @endgroup:


   ////////////////////////////////////////////////////////////
   // class TlsCapNegotiatorClient
   ////////////////////////////////////////////////////////////
   class TlsCapNegotiatorClient: public Csi::InstanceValidator
   {
   public:
      ////////////////////////////////////////////////////////////
      // on_complete
      ////////////////////////////////////////////////////////////
      enum outcome_type
      {
         outcome_no_encryption = 1,
         outcome_tls_client = 2,
         outcome_tls_server = 3
      };
      virtual void on_complete(
         TlsCapNegotiator *negotiator, outcome_type outcome) = 0;

      ////////////////////////////////////////////////////////////
      // send_data
      ////////////////////////////////////////////////////////////
      virtual void send_data(
         TlsCapNegotiator *negotiator, void const *buff, uint4 buff_len) = 0;
   };


   ////////////////////////////////////////////////////////////
   // class TlsCapNegotiator
   //
   // Defines a component that carries on the negotiations to determine whether
   // TLS should be used on a TCP link and, if so, which role this side should
   // take.  In order to use this component, an application must provide an
   // object that extends class TlsCapNegotiatorClient which provides methods
   // that will be invoked when this component needs to send data and to report
   // its outcome.  Shortly after creation, this component will generate a
   // capabilities message and set a timer to wait for the peer's capabilities
   // message.   If the peer's capabilities are not received within 10 seconds,
   // we will assume that encryption is not supported.  If the capabilities are
   // received, we will determine our role using the following rules:
   //
   //     If our TLS stack is configured to accept client connections and the
   //     peer identifies only client capabilities, we will take the role of a
   //     TLS server.
   //
   //     If our TLS stack is configured only to make client connections and
   //     the peer identifies server capabilities, we will take on the role of
   //     a TLS client.
   //
   //     If both our stack and the peer can accept TLS client connections, our
   //     role will depend upon whether we are a TCP client.  if we are a TCP
   //     client, we will take on the role of a TLS client.
   //
   //     If our stack cannot accept TLS clients and the peer cannot act as a
   //     TLS server,  Encryption will not be used.
   //
   //  The outcomes of these decisions will be reported to the client via its
   //  on_complete() method. 
   ////////////////////////////////////////////////////////////
   class TlsCapNegotiator:
      public OneShotClient,
      public EventReceiver
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      typedef TlsCapNegotiatorClient client_type;
      typedef Csi::SharedPtr<OneShot> timer_handle;
      TlsCapNegotiator(
         client_type *client_,
         bool tcp_client_,
         timer_handle timer_ = 0);

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      virtual ~TlsCapNegotiator();

      ////////////////////////////////////////////////////////////
      // onOneShotFired
      ////////////////////////////////////////////////////////////
      virtual void onOneShotFired(uint4 id);

      ////////////////////////////////////////////////////////////
      // on_data
      ////////////////////////////////////////////////////////////
      void on_data(Csi::ByteQueue &buff);

      ////////////////////////////////////////////////////////////
      // get_start_command
      ////////////////////////////////////////////////////////////
      void const *get_start_command() const;

      ////////////////////////////////////////////////////////////
      // get_start_command_len
      ////////////////////////////////////////////////////////////
      uint4 get_start_command_len() const;
         
      ////////////////////////////////////////////////////////////
      // receive
      ////////////////////////////////////////////////////////////
      virtual void receive(SharedPtr<Event> &ev);
      
   private:
      ////////////////////////////////////////////////////////////
      // report_complete
      ////////////////////////////////////////////////////////////
      void report_complete();

      ////////////////////////////////////////////////////////////
      // make_outcome
      ////////////////////////////////////////////////////////////
      client_type::outcome_type make_outcome();
      
   private:
      ////////////////////////////////////////////////////////////
      // client
      ////////////////////////////////////////////////////////////
      client_type *client;

      ////////////////////////////////////////////////////////////
      // tcp_client
      ////////////////////////////////////////////////////////////
      bool tcp_client;

      ////////////////////////////////////////////////////////////
      // timer
      ////////////////////////////////////////////////////////////
      timer_handle timer;

      ////////////////////////////////////////////////////////////
      // watchdog_id
      ////////////////////////////////////////////////////////////
      uint4 watchdog_id;

      ////////////////////////////////////////////////////////////
      // transmitted_caps
      ////////////////////////////////////////////////////////////
      bool transmitted_caps;

      ////////////////////////////////////////////////////////////
      // got_server_caps
      ////////////////////////////////////////////////////////////
      bool got_server_caps;

      ////////////////////////////////////////////////////////////
      // peer_tls_client
      ////////////////////////////////////////////////////////////
      bool peer_tls_client;

      ////////////////////////////////////////////////////////////
      // peer_tls_server
      ////////////////////////////////////////////////////////////
      bool peer_tls_server;

      ////////////////////////////////////////////////////////////
      // peer_pakbus_unquoted
      ////////////////////////////////////////////////////////////
      bool peer_pakbus_unquoted;

      ////////////////////////////////////////////////////////////
      // reported_complete
      ////////////////////////////////////////////////////////////
      bool reported_complete;
   };
};


#endif

