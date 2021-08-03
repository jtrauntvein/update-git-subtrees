/* Csi.Win32.SocketTcpSock.cpp

   Copyright (C) 2001, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 22 February 2001
   Last Change: Wednesday 21 November 2018
   Last Commit: $Date: 2020-11-16 17:23:52 -0600 (Mon, 16 Nov 2020) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Csi.Win32.SocketTcpSock.h"
#include "Csi.Win32.WinsockException.h"
#include "Csi.SocketBase.h"
#include "Csi.MaxMin.h"
#include "Csi.TlsContext.h"


namespace Csi
{
   namespace Win32
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // tls_context
         ////////////////////////////////////////////////////////////
         TlsContext *tls_context(0);
      };

      
      ////////////////////////////////////////////////////////////
      // class SocketTcpSock definitions
      ////////////////////////////////////////////////////////////
      SocketTcpSock::SocketTcpSock(
         SOCKET socket_handle,
         bool using_tls_,
         bool is_server_):
         WinSockBase(socket_handle),
         can_write(false),
         is_connected(false),
         read_buffer(2048, true),
         write_buffer(2048, true),
         using_tls(using_tls_),
         is_server(is_server_)
      {
         if(socket_handle != INVALID_SOCKET)
            open((int)socket_handle);
      } // constructor


      SocketTcpSock::~SocketTcpSock()
      { close(); }


      void SocketTcpSock::open(char const *address, uint2 port)
      {
         // we must first close any current connection.
         close();
         connected_address.clear();

         // we now need to resolve the specified address into a list of potential addresses
         domain_name = address;
         SocketAddress::resolve(addresses, address, port);
         is_server = false;
         if(addresses.empty())
            throw WinsockException("address lookup failed");
         do_next_connect(addresses.front());
      } // open
      

      void SocketTcpSock::open(uint4 resolved_address, uint2 port)
      {
         // open the socket handle
         StrAsc address(address_to_str(resolved_address));
         open(address.c_str(), port);
      } // open


      void SocketTcpSock::open(int socket)
      {
         if(socket != socket_handle)
            close();
         if(socket != INVALID_SOCKET) 
         {
            // set up the socket handle so that we get read, write, close and connection failure
            // notifications
            int rcd;
            socket_handle = socket;
            is_server = true;
            rcd = WSAAsyncSelect(
               socket_handle,
               get_window_handle(),
               socket_event,
               FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE);
            if(rcd == SOCKET_ERROR)
            {
               WinsockException e("socket initialisation failure");
               close();
               throw e;
            }
            if(using_tls)
            {
               if(tls_context != 0)
                  tls_context->add_connection(this, !is_server);
               else
                  throw std::invalid_argument("cannot start TLS without a valid context");
            }
            can_write = is_connected = true;
            flush_tx();
            on_connected(connected_address);
         }
      } // open


      void SocketTcpSock::close()
      {
         if(using_tls && tls_context)
            tls_context->remove_connection(this);
         WinSockBase::close();
         using_tls = is_connected = can_write = false;
         read_buffer.pop(read_buffer.size());
         write_buffer.pop(write_buffer.size());
      } // close

      
      void SocketTcpSock::write(void const *buffer, uint4 buffer_len)
      {
         write_buffer.push(buffer, buffer_len); 
         if(!using_tls && can_write)
            flush_tx();
         else if(using_tls)
            tls_context->on_socket_data(this, 0, 0);
      } // write


      void SocketTcpSock::on_read()
      { }


      void SocketTcpSock::on_low_level_read(
         void const *buff, uint4 buff_len)
      {
         read_buffer.push(buff, buff_len);
         on_read();
      } // on_low_level_read


      void SocketTcpSock::set_tls_context(TlsContext *context)
      {
         if(tls_context != 0)
            delete tls_context;
         tls_context = context;
      } // set_tls_context


      TlsContext *SocketTcpSock::get_tls_context()
      { return tls_context; }


      void SocketTcpSock::start_tls_client()
      {
         if(tls_context == 0)
            throw std::invalid_argument("Cannot start TLS without a valid TLS context");
         if(!using_tls)
         {
            using_tls = true;
            tls_context->add_connection(this, true);
         }
         else
            throw std::invalid_argument("TLS already started");
      } // start_tls_client


      void SocketTcpSock::start_tls_server(void const *transmit_before, uint4 before_len)
      {
         if(tls_context == 0)
            throw std::invalid_argument("Cannot start TLS without a valid TLS context");
         if(!using_tls)
         {
            using_tls = true;
            tls_context->add_connection(this, false, transmit_before, before_len);
         }
         else
            throw std::invalid_argument("TLS already started");
      } // start_tls_client


      void SocketTcpSock::on_closing_tls()
      {
         using_tls = false;
      } // on_closing_tls

      uint4 SocketTcpSock::get_tx_buff_size()
      {
         uint4 rtn(0);
         ByteQueue *buffer(&write_buffer);
         if(using_tls)
            buffer = tls_context->get_socket_write_buffer(this);
         if(buffer)
            rtn = buffer->size();
         return rtn;
      } // get_tx_buff_size
      
      void SocketTcpSock::on_socket_event(
         int error_code,
         int event)
      {
         int4 rcd;
         if(error_code == 0)
         {
            switch(event)
            {
            case FD_WRITE:
               can_write = true;
               flush_tx();
               break;
               
            case FD_READ:
               while((rcd = recv(socket_handle, low_level_buff, sizeof(low_level_buff), 0)) > 0)
               {
                  if(!using_tls)
                     on_low_level_read(low_level_buff, rcd);
                  else
                     tls_context->on_socket_data(this, low_level_buff, rcd);
               }
               break;
               
            case FD_CONNECT:
               is_connected = can_write = true;
               flush_tx();
               on_connected(connected_address);
               break;
               
            case FD_CLOSE:
               on_socket_error(error_code);
               break;
            }
         }
         else
         {
            try
            {
               if(event == FD_CONNECT && !addresses.empty())
                  do_next_connect(addresses.front());
               
               else
                  on_socket_error(error_code);
            }
            catch(WinsockException &e)
            { on_socket_error(e.get_socket_error()); }
            catch(std::exception &)
            { on_socket_error(0); }
         }
      } // on_socket_event


      LRESULT SocketTcpSock::on_message(
         uint4 message_id, WPARAM wparam, LPARAM lparam)
      {
         return WinSockBase::on_message(message_id, wparam, lparam);
      } // on_message


      void SocketTcpSock::do_next_connect(SocketAddress const &address_)
      {
         // we must now attempt to connect to each one of these until we have succeeded
         SocketAddress const address(addresses.front());
         int rcd;
         
         addresses.pop_front();
         if(socket_handle != INVALID_SOCKET)
         {
            closesocket(socket_handle);
            socket_handle = INVALID_SOCKET;
         }
         socket_handle = ::socket(address.get_family(), SOCK_STREAM, 0);
         if(socket_handle != INVALID_SOCKET)
         {
            // we need to set up the event notification for the socket
            rcd = WSAAsyncSelect(
               socket_handle, get_window_handle(), socket_event, FD_READ| FD_WRITE| FD_CONNECT| FD_CLOSE);
            if(rcd == SOCKET_ERROR)
            {
               WinsockException e("connect failure");
               close();
               throw e;
            }
            
            // we can now start the connection
            rcd = ::connect(socket_handle, address.get_storage(), address.get_address_len());
            if(rcd == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
            {
               WinsockException e("connect failure");
               close();
               if(addresses.empty())
                  throw e;
            }
            connected_address = address;
            if(rcd != SOCKET_ERROR)
            {
               is_connected = can_write = true;
               flush_tx();
               on_connected(connected_address);
            }
         }
      } // do_next_connect
      
      
      void SocketTcpSock::flush_tx()
      {
         ByteQueue *buffer(&write_buffer);
         fill_write_buffer(write_buffer);
         if(using_tls)
            buffer = tls_context->get_socket_write_buffer(this);
         while(can_write && buffer && !buffer->empty())
         {
            uint4 send_len = csimin<uint4>(sizeof(low_level_buff), buffer->size());
            int rcd;
            
            buffer->copy(low_level_buff, send_len);
            rcd = send(socket_handle, low_level_buff, send_len, 0);
            if(rcd != SOCKET_ERROR)
            {
               if(!using_tls)
                  on_low_level_write(low_level_buff, rcd);
               buffer->pop(rcd);
            }
            if(static_cast<uint4>(rcd) != send_len)
            {
               if(WSAGetLastError() != WSAEWOULDBLOCK)
                  on_socket_error(WSAGetLastError());
               can_write = false;
            }
         }
      } // flush_tx
   };
};

