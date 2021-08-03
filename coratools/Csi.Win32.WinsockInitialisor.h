/* Csi.Win32.WinsockInitialisor.h

   Copyright (C) 2000, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 08 April 2000
   Last Change: Tuesday 27 October 2015
   Last Commit: $Date: 2015-10-27 10:51:54 -0600 (Tue, 27 Oct 2015) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Win32_WinsockInitialisor_h
#define Csi_Win32_WinsockInitialisor_h

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <assert.h>
#include "Csi.Win32.WinsockException.h"

#pragma comment(lib, "ws2_32.lib")


namespace Csi
{
   namespace Win32
   {
      ////////// class WinsockInitialisor
      // Defines a default constructor that initialises the WinSock library to the levels needed by
      // the messaging tools. Also defines a destructor that performs the required clean-up. An
      // application can best use this class by creating an automatic instance in its main() so that
      // the cleanup takes place automiatically when the object goes out of scope.
      class WinsockInitialisor
      {
      public:
         ////////// constructor
         WinsockInitialisor()
         {
            WSAData socket_data;
            if(WSAStartup(MAKEWORD(1,1),&socket_data))
               throw WinsockException("Socket Initialisation failed");
         }

         ////////// destructor
         virtual ~WinsockInitialisor()
         { 
            int rcd = WSACleanup(); 
            assert(rcd == 0);
         } // destructor
      };
   };
};

#endif
