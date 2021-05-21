/* Csi.Win32.SocketConnection.h

   Copyright (C) 2000, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 8 August 1996
   Last Change: Monday 25 June 2012
   Last Commit: $Date: 2012-06-26 16:34:54 -0600 (Tue, 26 Jun 2012) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Win32_SocketConnection_h
#define Csi_Win32_SocketConnection_h

#include "Csi.SocketAddress.h"
#include "Csi.MessageWindow.h"
#include "Csi.Messaging.Connection.h"
#include "Csi.ByteQueue.h"
#include "StrAsc.h"
#include <winsock2.h>


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class SocketConnection
      //
      // Derives from class NetConnection and uses the WinSock interface to provide message delivery
      // over TCP/IP
      ////////////////////////////////////////////////////////////
      class SocketConnection: public Messaging::Connection, public MessageWindow
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //
         // This constructor is used by the NetSockListener class when it accepts an
         // incoming connection. Because this constructor is called, the instance
         // will assume that it is running on a server and change its default
         // attach behaviour as a result.
         ////////////////////////////////////////////////////////////
         SocketConnection(SOCKET hSock);

         ////////////////////////////////////////////////////////////
         // network address constructor
         //
         // This constructor should be used by a client application when a socket connection is
         // being set up. The destName parameter should be the IP address of the server (expressed
         // as either an IP address or as a domain name). The destPort parameter is the IP port of
         // the socket that should be connected. Because this constructor is being invoked, the
         // class code will assume that this connection is a client connection.
         //
         // Note that the connection to the server socket will not be made until the attach method
         // is called.
         ////////////////////////////////////////////////////////////
         SocketConnection(const char *destName, uint2 destPort);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SocketConnection();

         //@group Overloaded methods from NetConnection
         
         ////////////////////////////////////////////////////////////
         // sendMessage
         ////////////////////////////////////////////////////////////
         virtual void sendMessage(Messaging::Message *msg);
   
         ////////////////////////////////////////////////////////////
         // attach
         //
         // Creates the connection to the server socket only if this is a client
         // connection. If this is a server connection, then this method will have no
         // other effect than to set the isConnected flag to true.
         ////////////////////////////////////////////////////////////
         virtual void attach();
   
         ////////////////////////////////////////////////////////////
         // detach
         //
         // Closes the socket handle and sets the isConnected flag to false.
         ////////////////////////////////////////////////////////////
         virtual void detach();

         ////////////////////////////////////////////////////////////
         // get_remote_address
         //
         // Returns the address of the remote connection.
         ////////////////////////////////////////////////////////////
         virtual StrAsc get_remote_address();
            
         //@endgroup

      protected:
         ////////////////////////////////////////////////////////////
         // on_message
         ////////////////////////////////////////////////////////////
         virtual LRESULT on_message(uint4 message_id, WPARAM p1, LPARAM p2);

      private:
         ////////////////////////////////////////////////////////////
         // do_next_connect
         ////////////////////////////////////////////////////////////
         void do_next_connect();
   
      private:
         ////////////////////////////////////////////////////////////
         // hSock
         //
         // The socket handle used for both input and output
         ////////////////////////////////////////////////////////////
         SOCKET hSock;

         ////////////////////////////////////////////////////////////
         // destName
         //
         // The internet address of the server (set if this is a client socket)
         ////////////////////////////////////////////////////////////
         StrAsc destName;

         ////////////////////////////////////////////////////////////
         // destPort
         //
         // The port of the server socket (set if this is a client socket)
         ////////////////////////////////////////////////////////////
         unsigned short destPort;

         ////////////////////////////////////////////////////////////
         // isServer
         //
         // Set to true if this socket was constructed with the socket handle
         // constructor.
         ////////////////////////////////////////////////////////////
         bool isServer;

         ////////////////////////////////////////////////////////////
         // isSending
         //
         // Set to true if the socket is waiting for an asynchronous send operation 
         ////////////////////////////////////////////////////////////
         bool isSending;

         ////////////////////////////////////////////////////////////
         // betweenMsgs
         //
         // Set to true if the connection is waiting for the start of a new message
         ////////////////////////////////////////////////////////////
         bool betweenMsgs;

         ////////////////////////////////////////////////////////////
         // isConnected
         //
         // Set to true when the connection is active
         ////////////////////////////////////////////////////////////
         bool isConnected;

         ////////////////////////////////////////////////////////////
         // sendBuff
         //
         // Buffers messages until they can be sent
         ////////////////////////////////////////////////////////////
         ByteQueue sendBuff;

         ////////////////////////////////////////////////////////////
         // rcvBuff
         //
         // Buffers incoming messages until they have been completely received
         ////////////////////////////////////////////////////////////
         ByteQueue rcvBuff;

         ////////////////////////////////////////////////////////////
         // msgLen
         //
         // The length of the message that is currently being read. Is valid only
         // when the betweenMsgs variable is false
         ////////////////////////////////////////////////////////////
         uint4 msgLen;

         ////////////////////////////////////////////////////////////
         // addressses
         ////////////////////////////////////////////////////////////
         SocketAddress::addresses_type addresses;

         ////////////////////////////////////////////////////////////
         // maxTxSize
         //
         // The maximum number of characters that can be transmitted on the socket at
         // one time.
         ////////////////////////////////////////////////////////////
         static const UInt4 MaxTxSize;

         ////////////////////////////////////////////////////////////
         // maxRxSize
         //
         // The maximum number of bytes that will be read from the socket at one time
         ////////////////////////////////////////////////////////////
         static const UInt4 MaxRxSize;

      private:
         ////////////////////////////////////////////////////////////
         // flushTx
         //
         // Utility method that monitors the transmitting process
         ////////////////////////////////////////////////////////////
         void flushTx();

         ////////////////////////////////////////////////////////////
         // onSockSelect
         //
         // Handles the windows message posted by the sockets library that indicates
         // that a select event has occurred.
         ////////////////////////////////////////////////////////////
         LRESULT onSockSelect(WPARAM p1, LPARAM p2);

         ////////////////////////////////////////////////////////////
         // doConnect
         //
         // Initialises a client socket connection. This includes the creation and
         // beginning the attachment of the socket handle.
         ////////////////////////////////////////////////////////////
         void doConnect();

         //@group socket event notification methods

         ////////////////////////////////////////////////////////////
         // onSend
         //
         // Called when the socket library is ready to transmit more data one a
         // socket
         ////////////////////////////////////////////////////////////
         void onSend(int respCode);

         ////////////////////////////////////////////////////////////
         // onReceive
         //
         // Called when data has been received for a socket handle
         ////////////////////////////////////////////////////////////
         void onReceive(int respCode);

         ////////////////////////////////////////////////////////////
         // onConnect
         //
         // Called to indicate that the connection process has completed.
         ////////////////////////////////////////////////////////////
         void onConnect(int respCode);

         ////////////////////////////////////////////////////////////
         // onClose
         //
         // Called to indicate that the socket handle has been shut down.
         ////////////////////////////////////////////////////////////
         void onClose(int respCode);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // popLen
         //
         // Pops the length of the next message from the receive buffer
         ////////////////////////////////////////////////////////////
         uint4 popLen();
      }; 
   };
};

#endif
