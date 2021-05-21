/* Csi.TlsContext.h

   Copyright (C) 2010, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 27 October 2010
   Last Change: Tuesday 14 June 2016
   Last Commit: $Date: 2016-06-14 15:07:02 -0600 (Tue, 14 Jun 2016) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_TlsContext_h
#define Csi_TlsContext_h

#include "Csi.SocketTcpSock.h" 
#include "Csi.Events.h"
#include "Csi.Protector.h"
#include <map>
#include <stdexcept>


namespace Csi
{
   namespace TlsContextHelpers
   {
      /**
       * Forward declaration for the object that will manage the structures used for the TLS stack. 
       */
      class TlsContextImpl;

      /**
       * Forward declaration for the object that will represent the state of a TLS connection.
       */
      class TlsConnection;

      /**
       * Defines an exception that can be thrown while attempting to initialise the context.
       */
      class TlsException: public MsgExcept
      {
      public:
         /**
          * Specifies the error code that was reported.
          */
         int error_code;

         /**
          * @param error_code_ Specifies the error code.
          *
          * @param more Specifies extra text.
          */
         TlsException(int error_code_, char const *more = "");
      };
   };

   
   /**
    * Defines a singleton used by all socket objects for TLS server and client services.
    */
   class TlsContext: public Csi::EventReceiver
   {
   public:
      /**
       * @param certificate_name_ Specifies the name of the file that contains the server
       * certificate.
       *
       * @param private_key_name_ Specifies the name of the file that contains the server private
       * key.
       *
       * @param password_ Specifies the password used for decrypting the private key file.
       */
      TlsContext(
         StrAsc const &certificate_name_ = "",
         StrAsc const &private_key_name_ = "", 
         StrAsc const &password_ = "");

      /**
       * Destructor
       */
      virtual ~TlsContext();
      
      /**
       * Adds a connection to be managed by this context.
       *
       * @param connection Specifies the connection to manage.
       *
       * @param is_client Set to true if this is for a client connection.
       *
       * @param transmit_before Optionally specifies data that should be transmitted before the TLS
       * handshake.
       *
       * @param before_len Specifies the number of bytes of the transmit_before buffer to send.
       */
      void add_connection(
         SocketTcpSock *connection,
         bool is_client,
         void const *transmit_before = 0,
         uint4 before_len = 0);

      /**
       * Removes the specified connection from the list managed by this context.
       */
      void remove_connection(SocketTcpSock *connection);

      /**
       * Overloads the base class to handle events.
       */
      virtual void receive(SharedPtr<Event> &ev);

      /**
       * Can be overloaded to handle the event where the TLS context has been initialised.
       *
       * @param level Specifies the level at which TLS connections can be serviced.
       *
       * @param message Specifies any message for the user or application.
       */
      enum service_level_type
      {
         service_level_none,
         service_level_client,
         service_level_client_server
      };
      virtual void on_context_initialised(
         service_level_type level, StrAsc const &message)
      { }

      /**
       * @return Returns true if the context has been configiured to accept client connections.
       */
      virtual bool get_accepts_client_connections() const
      { return accepts_client_connections; }

      /**
       * Handles requests from the mbedtls ssl bio callback for sending data to the socket.
       *
       * @param connection  Specifies the affected connection.
       *
       * @param buff Specifies the buffer to which the data will be written.
       *
       * @param buff_len Specifies the maximum amount of data to write.
       */
      int do_write(SocketTcpSock *connection, void const *buff, size_t buff_len);

      /**
       * Handles request from the mbedtls ssl bio callback for reading data from the socket.
       *
       * @param connection Specifies the affected connection.
       *
       * @param buff Specifies the destination buffer.
       *
       * @param buff_len Specifies the maximum number of bytes that should be written.
       */
      int do_read(SocketTcpSock *connection, void *buff, size_t buff_len);

      /**
       * Handles data that has been received from the specified socket.
       *
       * @param socket Specifies the socket for which data has been received.
       *
       * @param buff Specifies the data that has been received.
       *
       * @param buff_len Specifies the number of bytes that have been received.
       */
      void on_socket_data(SocketTcpSock *socket, void const *buff, size_t buff_len);

      /**
       * @return Returns the encrypted write buffer for the specified socket.
       *
       * @param socket Specifies the socket to find.  Returns null if the socket is not valid.
       */
      ByteQueue *get_socket_write_buffer(SocketTcpSock *socket);

   private:
      /**
       * Specifies the name of the private key file.
       */
      StrAsc const private_key_name;

      /**
       * Specifies the name of the host certificate.
       */
      StrAsc const certificate_name;

      /**
       * Specifies the password used to decrypt an encrypted private key.
       */
      StrAsc const password;

      /**
       * Set to true if this context has been initialised to accept client connections.
       */
      bool accepts_client_connections;

      /**
       * Reference to the object that maintains the memory for structures used by this global TLS
       * context.
       */
      typedef TlsContextHelpers::TlsContextImpl impl_type;
      SharedPtr<impl_type> implementation;

      /**
       * Collection of active connections managed by this context.
       */
      typedef TlsContextHelpers::TlsConnection connection_type;
      typedef SharedPtr<connection_type> connection_handle;
      typedef std::map<SocketTcpSock *, connection_handle> connections_type;
      Protector<connections_type> connections;

      /**
       * Defines a low level buffer that is used for reading to or writing to mbedtls.
       */
      byte low_level_buff[1024];
   };
};


#endif
