/* Csi.Win32.WinsockException.cpp

   Copyright (C) 2002, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 05 March 2002
   Last Change: Thursday 22 December 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32.WinsockException.h"
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <sstream>

namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class WinsockException definitions
      ////////////////////////////////////////////////////////////
      WinsockException::WinsockException(
         char const *message,
         uint4 socket_error_):
         MsgExcept(""),
         socket_error(socket_error_)
      {
         // form the initial message
         std::ostringstream out;
         out << message;

         // we need to get the windows socket error
         if(socket_error == 0)
            socket_error = WSAGetLastError();
         if(socket_error != 0)
         {
            out << "\",\"" << socket_error << "\",\"";
            switch(socket_error)
            {
            case WSAEACCES:
               out << "permission denied";
               break;

            case WSAEADDRINUSE:
               out << "address already in use";
               break;

            case WSAEADDRNOTAVAIL:
               out << "cannot assign requested address";
               break;

            case WSAEAFNOSUPPORT:
               out << "TCP/IP support is not configured on this host";
               break;

            case WSAEALREADY:
               out << "operation already in progress";
               break;

            case WSAECONNABORTED:
               out << "software caused connection abort";
               break;

            case WSAECONNREFUSED:
               out << "connection refused";
               break;

            case WSAECONNRESET:
               out << "connection reset by peer";
               break;

            case WSAEDESTADDRREQ:
               out << "destination address required";
               break;

            case WSAEFAULT:
               out << "bad address";
               break;

            case WSAEHOSTDOWN:
               out << "host is down";
               break;

            case WSAEHOSTUNREACH:
               out << "no route to host";
               break;

            case WSAEINPROGRESS:
               out << "operation now in progress";
               break;

            case WSAEINTR:
               out << "interrupted function call";
               break;

            case WSAEINVAL:
               out << "invalid argument";
               break;

            case WSAEISCONN:
               out << "socket is already connected";
               break;

            case WSAEMFILE:
               out << "too many open sockets";
               break;

            case WSAEMSGSIZE:
               out << "message too long";
               break;

            case WSAENETDOWN:
               out << "network is down";
               break;

            case WSAENETRESET:
               out << "network dropped connection on reset";
               break;

            case WSAENETUNREACH:
               out << "network is unreachable";
               break;

            case WSAENOBUFS:
               out << "no buffer space available";
               break;

            case WSAENOPROTOOPT:
               out << "bad protocol option";
               break;

            case WSAENOTCONN:
               out << "socket is not connected";
               break;

            case WSAENOTSOCK:
               out << "socket operation attempted on non-socket";
               break;

            case WSAEOPNOTSUPP:
               out << "operation not supported";
               break;

            case WSAEPFNOSUPPORT:
               out << "protocol family not supported";
               break;

            case WSAEPROCLIM:
               out << "too many processes using winsock";
               break;

            case WSAEPROTONOSUPPORT:
               out << "protocol not supported";
               break;

            case WSAEPROTOTYPE:
               out << "protocol wrong type for socket";
               break;

            case WSAESHUTDOWN:
               out << "cannot send after socket shut down";
               break;

            case WSAETIMEDOUT:
               out << "connection timed out";
               break;

            case WSATYPE_NOT_FOUND:
               out << "class type not found";
               break;

            case WSAEWOULDBLOCK:
               out << "resource is temporarily unavailable";
               break;

            case WSAHOST_NOT_FOUND:
               out << "no such host is known";
               break;

            case WSA_INVALID_HANDLE:
               out << "specified event object is invalid";
               break;

            case WSA_INVALID_PARAMETER:
               out << "one or more parameters are invalid";
               break;

            case WSA_IO_INCOMPLETE:
               out << "overlapped i/o event object not signaled"; 
               break;

            case WSA_IO_PENDING:
               out << "overlapped operations will complete later";
               break;

            case WSA_NOT_ENOUGH_MEMORY:
               out << "insufficient memory available";
               break;

            case WSANOTINITIALISED:
               out << "successful WSAStartup() not yet performed";
               break;

            case WSANO_DATA:
               out << "valid name, no data record of requested type";
               break;

            case WSANO_RECOVERY:
               out << "this is an non-recoverable error";
               break;

            case WSASYSCALLFAILURE:
               out << "system call failure";
               break;

            case WSASYSNOTREADY:
               out << "network subsystem is unavailable";
               break;

            case WSATRY_AGAIN:
               out << "non-authoritative host not found";
               break;

            case WSAVERNOTSUPPORTED:
               out << "winsock.dll version out of range";
               break;

            case WSAEDISCON:
               out << "graceful shutdown in progress";
               break;

            case WSA_OPERATION_ABORTED:
               out << "overlapped operation was aborted";
               break;
            }
         }
         msg.append(
            out.str().c_str(),
            out.str().length());
      } // constructor
   };
};
