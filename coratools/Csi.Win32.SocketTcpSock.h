/* Csi.Win32.SocketTcpSock.h

   Copyright (C) 2001, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 22 February 2001
   Last Change: Wednesday 27 January 2021
   Last Commit: $Date: 2021-01-27 10:18:08 -0600 (Wed, 27 Jan 2021) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Win32_SocketTcpSock_h
#define Csi_Win32_SocketTcpSock_h

#include "Csi.SocketAddress.h"
#include "Csi.Win32.WinSockBase.h"
#include "Csi.ByteQueue.h"
#include "Csi.Condition.h"


namespace Csi
{
   // @group: class forward declarations
   class TlsContext;
   // @endgroup:

   
   namespace Win32
   {
      /**
       * Defines an object that reads and writes data to and from the socket
       * handle into and from buffers. The on_read() and on_write() methods
       * will be invoked whenever new data has become available in the read
       * buffer and when data has been written from the write buffer. An
       * application can use this class by creating a new class that inherits
       * from it and overloads at least the on_read() method.
       */
      class SocketTcpSock: public WinSockBase
      {
      protected:
         /**
          * Buffers all incoming bytes from the socket
          */
         ByteQueue read_buffer;

         /**
          * Buffers all outgoing bytes to the socket
          */
         ByteQueue write_buffer;

      public:
         /**
          * @param socket_handle Optionally specifies the already opened socket handle.
          *
          * @param using_tls_ Set to true if this connection should immediately start using TLS.
          *
          * @param is_server Set to true if this connection is servicing the server side.
          */
         SocketTcpSock(
            SOCKET socket_handle = INVALID_SOCKET,
            bool using_tls_ = false,
            bool is_server = true);

         /**
          * Destructor
          */
         virtual ~SocketTcpSock();

         /**
          * Creates a connection to the server identified by the specified domain name and TCP port.
          *
          * @param address Specifies the IP address or domain name for the server interface.
          *
          * @param port Specifies the TCP port of the server.
          */
         virtual void open(char const *address, uint2 port);

         /**
          * Creates a connection to the specified IPv4 address and TCP port.
          *
          * @param resolved_address Specifies the IPv4 address representation.
          *
          * @param port Specifies the TCP port for the server.
          */
         virtual void open(uint4 resolved_address, uint2 port);

         /**
          * Sets up this connection using an already opened socket handle.
          *
          * @param new_connection Specifies the socket handle that has already been opened.
          */
         virtual void open(int new_connection);

         /**
          * Releases the socket handle and releases any resources associated with the current
          * connection.
          */
         virtual void close();

         /**
          * Adds the specified data to the transmit buffer for this connection and attempts to flush
          * it to the socket if the socket is in a state to receive data.
          *
          * @param buffer Specifies the start of the data to be written.
          *
          * @param buffer_len Specifies the number of bytes to write.
          */
         virtual void write(void const *buffer, uint4 buffer_len);

         /**
          * @return Returns true if this connection is active.
          */
         bool get_is_connected() const
         { return is_connected; }

         /**
          * @return Returns a reference to this connection's read buffer.
          */
         ByteQueue &get_read_buffer()
         { return read_buffer; }

         /**
          * @return Returns a reference to this connection's write buffer.
          */
         ByteQueue &get_write_buffer()
         { return write_buffer; }

         /**
          * Defines a hook that can be overloaded by the application that
          * informs the application that the connection has successfully
          * completed.
          *
          * @param connected_address Specifies the address that was used to make the connection.
          */
         virtual void on_connected(SocketAddress const &connected_address)
         { }

         /**
          * Called when new bytes have been read into the read buffer. If an
          * application overloads this method, it should delegate to this
          * version before doing its own processing so that the buffer will be
          * maintained.
          */
         virtual void on_read();

         /**
          * Handles a winsock event.
          */
         virtual void on_socket_event(
            int error_code,
            int event);

         /**
          * Called by this component at the point where the socket is ready to
          * write data to the socket.  This provides the application a hook
          * that will allow it to insert data into the queue just-in-time.
          * This method may be invoked from a different thread than the one
          * used for message dispatch.  Because of this, an overloaded version
          * must take appropriate synchronisation measures.
          *
          * @param buffer Specifies the write buffer that is ready to be transmitted.
          */
         virtual void fill_write_buffer(ByteQueue &buffer)
         { }

         /**
          * Called when bytes are received from the socket.  This version will add the data to the
          * receive queue and post an event that the application can receive through on_read().
          * Alternatively, the application could implement its own buffering and notification
          * mechanism by overloading this method.
          *
          * @param buff Specifies the start of the data that has been received.
          *
          * @param buff_len Specifies the number of bytes that have been received.
          */
         virtual void on_low_level_read(
            void const *buff,
            uint4 buff_len);

         /**
          * This method will be called within the thread immediately after the
          * specified data has been written to the socket.
          *
          * @param buff Specifies the start of the data that has just been written.
          *
          * @param buff_len Specifies the number of byets that have just been written.
          */
         virtual void on_low_level_write(
            void const *buff,
            uint4 buff_len)
         { }

         /**
          * Initialises the global state of the application for performing client and server TLS
          * connections.
          *
          * @param context Specifies the global state.  If set to null, the application will not be
          * able to handle TLS connections.
          */
         static void set_tls_context(TlsContext *context);

         /**
          * @return Returns the current TLS context.
          */
         static TlsContext *get_tls_context();

         /**
          * Sets up this connection to act as a TLS client.  In order for this method to succeed, a
          * context must have been previously established and set by calling set_tls_context().
          */
         void start_tls_client();

         /**
          * Called when the TLS connection has been successfully created.  
          */
         virtual void on_tls_client_ready()
         { }

         /**
          * Sets up this connection to act as a TLS server.
          *
          * @param transmit_before Specifies data that must be transmitted as clear text immediately
          * before the server hello.  If set to null, no data will be transmitted.
          *
          * @param before_len Specifies the number of bytes in the transmit_before buffer.
          */
         void start_tls_server(void const *transmit_before_ = 0, uint4 before_len = 0);

         /**
          * Called by the TLS context after it has added data to the ciphertext that needs to be
          * transmitted.
          */
         void on_tls_data_ready()
         { flush_tx(); }

         /**
          * Called when the TLS connection is closing for this socket.
          */
         virtual void on_closing_tls();

         /**
          * Overloads the windows message handler.
          */
         virtual LRESULT on_message(
            uint4 message_id, WPARAM wparam, LPARAM lparam);

         /**
          * @return Returns true if this side of the connection is the server side.
          */
         virtual bool get_is_server() const
         { return is_server; }

         /**
          * @return Returns true if this connection is configured to use TLS.
          */
         bool get_using_tls() const
         { return using_tls; }

         /**
          * @return Returns the domain name that was used for a client connection.
          */
         StrAsc const &get_domain_name() const
         { return domain_name; }

         /**
          * @return Returns true if this socket is in a state where it can still transmit data.
          */
         bool get_can_write() const
         { return can_write; }

         /**
          * @return Returns the number of bytes pending in the write buffer.
          */
         uint4 get_tx_buff_size();

         /**
          * @return Returns the currently connected address.
          */
         SocketAddress const &get_connected_address() const
         { return connected_address; }
         
      protected:
         /**
          * Called when the component is going to be attempting to connect
          * using the specified address.  If this method is overloaded by the
          * application, this version must be invoked.
          */
         virtual void do_next_connect(SocketAddress const &address);
         
      private: 
         /**
          * Sends as much of the contents of the write buffer that can be
          * buffered by the winsock layer.
          */
         void flush_tx();

      private:
         /**
          * Set to true if the socket is in a state where it can be written to.
          */
         bool can_write;

         /**
          * Set to true if the connection is complete
          */
         bool is_connected;

         /**
          * Specifies the address to which this socket is connected.
          */
         SocketAddress connected_address;

         /**
          * Specifies the domain name that was used.
          */
         StrAsc domain_name;

         /**
          * Set to true if this socket has started using the TLS interface. 
          */
         bool using_tls;

         /**
          * Specifies the buffer that is used for low level reads and writes to the socket.
          */
         char low_level_buff[2048];

         /**
          * Set to true if this connection is server side.
          */
         bool is_server;

         /**
          * Specifies the set of addresses that have yet to be tried when making a connection.
          */
         SocketAddress::addresses_type addresses;
      };
   };
};

#endif
