/* Csi.Win32.SocketConnection.cpp

   Copyright (C) 2000, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 8 August 1996
   Last Change: Tuesday 19 December 2017
   Last Commit: $Date: 2017-12-19 16:29:39 -0600 (Tue, 19 Dec 2017) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Csi.Win32.SocketConnection.h"
#include "Csi.Win32.WinsockException.h"
#include "Csi.SocketAddress.h"
#include "Csi.Messaging.Defs.h"
#include "Csi.Messaging.Router.h"
#include "Csi.Messaging.Message.h"
#include "Csi.OsException.h"
#include "Csi.MaxMin.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"
#include <assert.h>


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class SocketConnection  definitions
      ////////////////////////////////////////////////////////////

      const UINT WM_SockSelect = RegisterWindowMessageW(L"WM_SockSelect");


      const uint4 SocketConnection::MaxTxSize = 2048;


      const uint4 SocketConnection::MaxRxSize = 2048;


      SocketConnection::SocketConnection(SOCKET hSock_):
         MessageWindow("Server Socket Connection"),
         hSock(hSock_),
         isServer(true),
         isSending(false),
         destPort(0),
         msgLen(0),
         betweenMsgs(true),
         isConnected(true),
         sendBuff(MaxTxSize),
         rcvBuff(MaxRxSize)
      {
         // register the interesting socket events
         int rcd;
         rcd = WSAAsyncSelect(hSock,get_window_handle(),WM_SockSelect,
                              FD_READ|FD_WRITE|FD_CONNECT|FD_CLOSE);
         if(rcd == SOCKET_ERROR)
            throw OsException("Socket select error");
      } // server constructor


      SocketConnection::SocketConnection(const char *destName_, unsigned short destPort_):
         MessageWindow("Client Socket Connection"),
         isServer(false),
         isSending(false),
         isConnected(false),
         betweenMsgs(true),
         msgLen(0),
         hSock(INVALID_SOCKET),
         destName(destName_),
         destPort(destPort_),
         sendBuff(MaxTxSize),
         rcvBuff(MaxRxSize)
      { }


      SocketConnection::~SocketConnection()
      {
         if(hSock != INVALID_SOCKET)
            closesocket(hSock);
      } // destructor


      void SocketConnection::sendMessage(Messaging::Message *msg)
      {
         // add the message length and contents to the send buffer
         long msgLen = htonl(msg->getLen());

         assert(msg->getLen() >= 4);
         sendBuff.push(&msgLen,sizeof(msgLen));
         sendBuff.push(msg->getMsg(),msg->getLen());

         // make sure that the send process gets underway
         flushTx();
      } // sendMessage


      void SocketConnection::attach()
      {
         // set up the connection
         isConnected = false;
         if(isServer == false)
            doConnect();
         else
            isConnected = true;
      } // attach


      void SocketConnection::detach()
      {
         isConnected = false;
         closesocket(hSock);
         hSock = INVALID_SOCKET;
      } // detach


      StrAsc SocketConnection::get_remote_address()
      {
         StrAsc rtn;
         if(hSock != INVALID_SOCKET)
         {
            struct sockaddr_storage address;
            int address_len(sizeof(address));
            int rcd = getpeername(
               hSock, reinterpret_cast<struct sockaddr *>(&address), &address_len);
            if(rcd != SOCKET_ERROR)
            {
               SocketAddress addr(reinterpret_cast<struct sockaddr *>(&address), address_len);
               Csi::OStrAscStream temp;
               temp << addr;
               rtn = temp.str();
            }
         }
         return rtn;
      } // get_remote_address


      LRESULT SocketConnection::on_message(uint4 message_id, WPARAM p1, LPARAM p2)
      {
         LRESULT rtn;
         if(Csi::Messaging::Router::is_valid_instance(router) &&
            message_id == WM_SockSelect)
         {
            int respCode = WSAGETSELECTERROR(p2);
            int event = WSAGETSELECTEVENT(p2);
      
            switch(event)
            {
            case FD_WRITE:
               onSend(respCode);
               break;
         
            case FD_READ:
               onReceive(respCode);
               break;
         
            case FD_CONNECT:
               onConnect(respCode);
               break;
         
            case FD_CLOSE:
               onClose(respCode);
               break; 
            } 
            rtn = 0;
         }
         else
            rtn = MessageWindow::on_message(message_id,p1,p2);
         return rtn;
      } // onSockSelect


      void SocketConnection::doConnect()
      {
         // we need to resolve the address of the server to a set of possible addresses
         SocketAddress::resolve(addresses, destName.c_str(), destPort);
         if(addresses.empty())
            throw OsException("name resolution failed");
         do_next_connect(); 
      } // doConnect


      void SocketConnection::onSend(int respCode) 
      {
         // check for errors
         if(respCode != 0)
         {
            router->onConnClosed();
            return;
         }

         // check to see if there is more to send
         isSending = false;
         if(!sendBuff.empty())
            flushTx();
      } // onSend


      void SocketConnection::onReceive(int respCode) 
      {
         try
         {
            // check for errors
            if(respCode != 0)
            {
               router->onConnClosed();
               throw WinsockException("receive failure",respCode);
            }
            
            // get what is available
            int4 rcd;
            static char tempRxBuff[MaxRxSize];
            
            while((rcd = recv(hSock,tempRxBuff,sizeof(tempRxBuff),0)) > 0)
               rcvBuff.push(tempRxBuff,rcd);
            
            // process the messages in what is available
            bool quitLoop = false;
            
            while(quitLoop == false)
            {
               // process message beginnings
               if(betweenMsgs == true && rcvBuff.size() >= 4)
               {
                  // treat a zero in message length as a heart beat
                  msgLen = popLen();
                  if(msgLen < 4)
                     continue;
                  else
                     betweenMsgs = false;
               }
               
               // process message contents
               if(betweenMsgs == false && msgLen <= rcvBuff.size())
               {
                  // we will route all messages except the heartbeat message
                  Messaging::Message msg(rcvBuff,msgLen);
                  
                  //@bugfix 17 May 1999 by Jon Trauntvein
                  // The betweenMsgs flag needs to be set to true before the message is dispatched
                  //@endbugfix
                  betweenMsgs = true;
                  if(msg.getMsgType() != Messaging::Messages::type_heart_beat)
                     router->rcvMessage(&msg); 
               } // send the message to the router
               
               // decide if there is enough to continue
               if((betweenMsgs == false && msgLen > rcvBuff.size()) ||
                  (betweenMsgs == true && rcvBuff.size() < 4))
                  quitLoop = true;
            } // process the messages in the buffer
         }
         catch(std::exception &e)
         {
            trace("Csi::Win32::SocketConnection::onReceive -- connect failure: %s",e.what());
            router->onConnClosed(Csi::Messaging::Router::closed_unknown_failure);
         }
      } // onReceive


      void SocketConnection::onConnect(int respCode) 
      {
         // check for errors
         if(respCode != 0)
         {
            closesocket(hSock);
            hSock = INVALID_SOCKET;
            if(addresses.empty())
               router->onConnClosed();
            else 
               do_next_connect();
         }
         else
         {
            isConnected = true;
            flushTx();
         }
      } // onConnect


      void SocketConnection::onClose(int respCode) 
      {
         closesocket(hSock);
         hSock = INVALID_SOCKET;
         isConnected = false;
         router->onConnClosed();
      } // onClose


      void SocketConnection::flushTx()
      {
         while(!isSending && isConnected && !sendBuff.empty())
         {
            // copy characters from the output queue into a buffer. The buffer is static to prevent
            // repeated stack allocations. This is safe because this function will only be called from the
            // main thread
            static char buff[MaxTxSize];
            uint4 sendLen = Csi::csimin<uint4>(sizeof(buff),sendBuff.size());
            int rcd;

            sendBuff.copy(buff,sendLen);
            rcd = send(hSock,buff,sendLen,0);
            if(rcd != SOCKET_ERROR)
               sendBuff.pop(rcd);
            if(uint4(rcd) != sendLen)
               isSending = true;
         }
         resetTxWd();
      } // flushTx


      uint4 SocketConnection::popLen()
      {
         // we will expect the first four bytes in the receive buffer to be the binary value of the
         // length in network order.
         uint4 len;
         uint4 rtn;

         rcvBuff.pop(&len,sizeof(len));
         rtn = ntohl(len);

         // we must also do bounds checking on the value returned.  It must be greater than or equal
         // to four (the minimum size messaging packet).  It must also be smaller than the amount of
         // available virtual memory (it makes no sense to consume that much if the program is going
         // to crash anyway).
         if(rtn < 4)
            throw MsgExcept("Invalid message length value received");
         if(rtn > get_available_virtual_memory())
            throw MsgExcept("Not enough memory to buffer the message");
         return rtn;
      }


      void SocketConnection::do_next_connect()
      {
         if(!addresses.empty())
         {
            // we need to create the socket
            SocketAddress address(addresses.front());
            addresses.pop_front();
            hSock = socket(address.get_family(), SOCK_STREAM, 0);
            if(hSock == INVALID_SOCKET && addresses.empty())
               throw OsException("socket creation failure");
            if(hSock != INVALID_SOCKET)
            {
               // we need to set up event notifications for the socket
               int rcd(
                  WSAAsyncSelect(
                     hSock, get_window_handle(), WM_SockSelect, FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE));
               if(rcd == SOCKET_ERROR)
                  throw OsException("select error");
               
               // connect the socket
               rcd = connect(hSock, address.get_storage(), address.get_address_len());
               if(rcd != SOCKET_ERROR)
                  onConnect(0);
               else
               {
                  if(WSAGetLastError() != WSAEWOULDBLOCK)
                  {
                     OsException e("connect failure");
                     closesocket(hSock);
                     hSock = INVALID_SOCKET;
                     if(!addresses.empty())
                        do_next_connect();
                     else
                        throw e;
                  }
               }
            }
         }
      } // do_next_connect
   };
};
