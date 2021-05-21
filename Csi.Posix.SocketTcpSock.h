/* Csi.Posix.SocketTcpSock.h

   Copyright (C) 2005, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 30 August 2005
   Last Change: Friday 13 November 2020
   Last Commit: $Date: 2020-11-16 17:23:52 -0600 (Mon, 16 Nov 2020) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Posix_SocketTcpSock_h
#define Csi_Posix_SocketTcpSock_h

#include "Csi.Posix.SocketBase.h"
#include "Csi.Protector.h"
#include "Csi.Thread.h"
#include "Csi.ByteQueue.h"
#include "Csi.Condition.h"
#include "Csi.SocketAddress.h"


typedef int SOCKET;


#ifdef __APPLE__
      #define MSG_NOSIGNAL 0
#endif


namespace Csi
{
   // @group: class forward declarations
   class TlsContext;
   // @endgroup:

   
   namespace Posix
   {
      /**
       * Defines a class that encapsulates a TCP socket connection and provides
       * buffering services for reading from and writing to the socket.  
       */
      class SocketTcpSock:
         public SocketBase,
         private Thread
      {
      protected:
         /**
          * Defines the queue used to buffer bytes that have been read.
          */
         ByteQueue read_buffer;

         /**
          * Defines the queue used to buffer data that needs to be written.
          */
         ByteQueue write_buffer;

         /**
          * Specifies the address to which we are connecting as a client.
          */
         StrAsc client_address;

         /**
          * Specifies the address that wwas resolved that worked.
          */
         SocketAddress resolved_address;

         /**
          * Specifies the TCP port for the service to which we are connecting as a client.
          */
         uint2 client_port;

         /**
          * Set to true if the connection thread must be stopped.
          */
         bool should_quit;

         /**
          * Set to true to indicate that this socket is connected.
          */
         bool is_connected;

         /**
          * Set to true if this connection is using TLS.
          */
         bool using_tls;

         /**
          * Set to true if this connection is for a server.
          */
         bool is_server;

         /**
          * Set to true if data can be written on the socket.
          */
         bool can_send;
         
      public:
         /**
          * @param socket_handle Optionally specifies the already opened socket handle.
          *
          * @param using_tls_ Set to true if this connection should immediately start using TLS.
          *
          * @param is_server Set to true if this connection is servicing the server side.
          */
         SocketTcpSock(
            int socket_handle = -1,
            bool using_tls_ = false,
            bool is_server_ = true);

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
         virtual void open(
            uint4 resolved_address,
            uint2 port);

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
         virtual void write(void const *buff, uint4 buff_len);

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
         virtual void on_read()
         { }

         /**
          * Called when the socket is closing.
          */
         virtual void on_close()
         { }

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
         virtual void on_low_level_read(void const *buff, uint4 buff_len);

         /**
          * This method will be called within the thread immediately after the
          * specified data has been written to the socket.
          *
          * @param buff Specifies the start of the data that has just been written.
          *
          * @param buff_len Specifies the number of byets that have just been written.
          */
         virtual void on_low_level_write(void const *buff, uint4 buff_len)
         { }

         /**
          * Overloads the event handler.
          */
         virtual void receive(SharedPtr<Event> &ev);

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
         void start_tls_server(void const *transmit_before = 0, uint4 before_len = 0);

         /**
          * Called when the TLS connection is closing for this socket.
          */
         virtual void on_closing_tls();

         /**
          * @return Returns true if this side of the connection is the server side.
          */
         bool get_is_server() const
         { return is_server; }

         /**
          * @return Returns true if this connection is configured to use TLS.
          */
         bool get_using_tls() const
         { return using_tls; }

         /**
          * Called to inform this socket that there is encrypted data ready to transmit.
          */
         void on_tls_data_ready()
         { signal_urgent(); }

         /**
          * @return Returns the domain address that was specified for this socket to open.
          */
         StrAsc const &get_domain_name() const
         { return client_address; }

         /**
          * @return Returns true if the socket is in a state to transmit data.
          */
         bool get_can_write() const
         { return can_send; }

         /**
          * @return Returns the number of bytes that are still pending in the write buffer for this
          * socket.
          */
         uint4 get_tx_buff_size();
         
      private:
         /**
          * Overrides the base class function to perform I/O for this socket.
          */
         virtual void execute();

         /**
          * Overrides the base class method to ensure that the socket thread is closed.
          */
         virtual void wait_for_end();
      };
   };
};


#endif
