/* Csi.SocketTcpStream.h

   Copyright (C) 2008, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Tuesday 17 June 2008
   Last Change: Tuesday 17 June 2008
   Last Commit: $Date: 2010-10-04 14:01:09 -0600 (Mon, 04 Oct 2010) $
   Last Changed by: $Author: tmecham $

*/

#pragma once
#ifndef Csi_SocketTcpStream_h
#define Csi_SocketTcpStream_h

#include "Csi.SocketTcpSock.h"
#include <iostream>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class OSocketTcpStreamBuff
   ////////////////////////////////////////////////////////////
   class OSocketTcpStreamBuff: public std::streambuf
   {
   private:
      ////////////////////////////////////////////////////////////
      // socket
      ////////////////////////////////////////////////////////////
      SocketTcpSock *socket;

      ////////////////////////////////////////////////////////////
      // write_buffer
      ////////////////////////////////////////////////////////////
      char write_buffer[1024];

      friend class OSocketTcpStream;

   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      OSocketTcpStreamBuff(SocketTcpSock *socket_):
         socket(socket_)
      { setp(write_buffer, write_buffer + sizeof(write_buffer) - 1); }

      ////////////////////////////////////////////////////////////
      // overflow
      ////////////////////////////////////////////////////////////
      virtual int_type overflow(int_type ch_)
      {
         char ch = static_cast<char>(ch_);
         socket->write(&ch, 1);
         return ch;
      }

      ////////////////////////////////////////////////////////////
      // xsputn
      ////////////////////////////////////////////////////////////
      virtual std::streamsize xsputn(
         char const *buff, std::streamsize buff_len)
      {
         socket->write(buff, static_cast<uint4>(buff_len));
         return buff_len;
      }
   };


   ////////////////////////////////////////////////////////////
   // class OSocketTcpStream
   ////////////////////////////////////////////////////////////
   class OSocketTcpStream: public std::ostream
   {
   protected:
      ////////////////////////////////////////////////////////////
      // buffer
      ////////////////////////////////////////////////////////////
      OSocketTcpStreamBuff buffer;
      
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      OSocketTcpStream(SocketTcpSock *socket):
         buffer(socket),
         std::ostream(&buffer)
      { }
   };
};


#endif
