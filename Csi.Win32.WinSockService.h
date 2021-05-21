/* Csi.Win32.WinSockService.h

   Copyright (C) 2001, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 22 February 2001
   Last Change: Tuesday 08 October 2013
   Last Commit: $Date: 2013-10-08 11:10:20 -0600 (Tue, 08 Oct 2013) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Win32_WinSockService_h
#define Csi_Win32_WinSockService_h

#include "Csi.Win32.WinSockBase.h"
#include "Csi.Thread.h"
#include "Csi.Events.h"
#include "Csi.Condition.h"
#include "Csi.SocketAddress.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class WinSockService
      //
      // Defines a base class for a windows socket service. This type of object is designed to be
      // bound to a specific port and wait for incoming connections. When a connection occurs, the
      // new, pure virtual method, on_accept(), will be invoked. An application will use this class
      // by creating a derived class that implements its own behavious for on_accept() and,
      // possibly, on_socket_error()
      ////////////////////////////////////////////////////////////
      class WinSockService: public Thread, public EventReceiver
      {
      protected:
         ////////////////////////////////////////////////////////////
         // service_port
         //
         // The port that this service should be bound to
         ////////////////////////////////////////////////////////////
         uint2 service_port;

         ////////////////////////////////////////////////////////////
         // local_only
         ////////////////////////////////////////////////////////////
         bool local_only;

         ////////////////////////////////////////////////////////////
         // should_close
         ////////////////////////////////////////////////////////////
         bool should_close;

         ////////////////////////////////////////////////////////////
         // start_condition
         ////////////////////////////////////////////////////////////
         Condition start_condition;

         ////////////////////////////////////////////////////////////
         // listening_addresses
         ////////////////////////////////////////////////////////////
         SocketAddress::addresses_type listening_addresses;

         ////////////////////////////////////////////////////////////
         // last_error
         ////////////////////////////////////////////////////////////
         StrAsc last_error;

         ////////////////////////////////////////////////////////////
         // logger
         //
         // Holds a reference (null by default) that will report log status messages
         // from the service.
         ////////////////////////////////////////////////////////////
         EventReceiver *logger;

         ////////////////////////////////////////////////////////////
         // allow_ipv6
         //
         // Set to true if IPv6 connections should be allowed.
         ////////////////////////////////////////////////////////////
         bool allow_ipv6;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //
         // The service will not begin until the application calls start_service().
         ////////////////////////////////////////////////////////////
         WinSockService();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~WinSockService()
         { stop_service(); }

         ////////////////////////////////////////////////////////////
         // start_service
         //
         // Starts the service. Will throw a std::exception derived object if the service cannot be
         // started.
         //
         // The service_port_ parameter should be set to zero if the application needs to specify
         // that the kernel provide the port assignment. In this case, this method will assign
         // service_port the value of the port that was reported by getsockname(). Note that,
         // according to the winsock documentation, this may not work if a wild card address is
         // assigned for the bind address.
         //
         // If the application specifies the bind_address_ parameter, this parameter should be in
         // the form of a dotted address notation. No attempt to resolve a domain name will be
         // made.
         ////////////////////////////////////////////////////////////
         virtual void start_service(
            uint2 service_port_, bool local_only = false);

         ////////////////////////////////////////////////////////////
         // stop_service
         //
         // stops the service if it has been successfully started.
         ////////////////////////////////////////////////////////////
         virtual void stop_service();

         ////////////////////////////////////////////////////////////
         // get_service_port
         ////////////////////////////////////////////////////////////
         uint2 get_service_port() const
         { return service_port; }

         ////////////////////////////////////////////////////////////
         // get_local_only
         ////////////////////////////////////////////////////////////
         bool get_local_only() const
         { return local_only; }

         ////////////////////////////////////////////////////////////
         // wait_for_end
         ////////////////////////////////////////////////////////////
         virtual void wait_for_end()
         { stop_service(); }

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(SharedPtr<Event> &ev);

         ////////////////////////////////////////////////////////////
         // get_listening_addresses
         ////////////////////////////////////////////////////////////
         typedef SocketAddress::addresses_type addresses_type;
         addresses_type const &get_listening_addresses() const
         { return listening_addresses; }

         ////////////////////////////////////////////////////////////
         // get_last_error
         ////////////////////////////////////////////////////////////
         StrAsc const &get_last_error() const
         { return last_error; }

         ////////////////////////////////////////////////////////////
         // set_logger
         //
         // Sets an event receiver to handle events of type
         // SocketServiceLogEvent from this service.  Setting the value to null
         // will prevent further messages from being sent. 
         ////////////////////////////////////////////////////////////
         virtual void set_logger(EventReceiver *logger_)
         {
            logger = logger_;
         }

         ////////////////////////////////////////////////////////////
         // get_allow_ipv6
         ////////////////////////////////////////////////////////////
         bool get_allow_ipv6() const
         { return allow_ipv6; }

         ////////////////////////////////////////////////////////////
         // set_allow_ipv6
         ////////////////////////////////////////////////////////////
         void set_allow_ipv6(bool allow_ipv6_)
         { allow_ipv6 = allow_ipv6_; }
         
      protected:
         ////////////////////////////////////////////////////////////
         // on_accept
         //
         // Called when a new connection has been accepted. The derived class must overload this
         // method.
         ////////////////////////////////////////////////////////////
         virtual void on_accept(SOCKET new_connection) = 0;

         ////////////////////////////////////////////////////////////
         // on_socket_error
         ////////////////////////////////////////////////////////////
         virtual void on_socket_error(int error_code) = 0;

         ////////////////////////////////////////////////////////////
         // execute
         ////////////////////////////////////////////////////////////
         virtual void execute();
      };
   };
};

#endif
