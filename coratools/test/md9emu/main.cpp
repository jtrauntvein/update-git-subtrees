/* main.cpp

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 22 January 2015
   Last Change: Thursday 22 January 2015
   Last Commit: $Date: 2015-01-23 18:06:11 -0600 (Fri, 23 Jan 2015) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.SerialPortBase.h"
#include "Csi.SocketTcpService.h"
#include "Csi.SocketTcpSock.h"
#include "Csi.SocketException.h"
#include "Csi.Win32.WinsockInitialisor.h"
#include "Csi.Win32Dispatch.h"
#include "Csi.CommandLine.h"
#include "Csi.Utils.h"
#include <iostream>


namespace
{
   class Application;
   class Proxy;

   class Port: public Csi::SerialPortBase
   {
   public:
      Port(Proxy *proxy_, StrAsc const &port);
      
   protected:
      virtual void on_error(char const *message);
      virtual void on_read();
      
   private:
      Proxy *proxy;
   };
   
   
   class Proxy: public Csi::SocketTcpSock
   {
   public:
      /**
       * Constructor
       *
       * @param application_ Specifies the application.
       *
       * @param socket_handle Specifies the socket handle.
       *
       * @param port_name_ Specifies the serial port name.
       */
      Proxy(Application *application_, SOCKET socket_handle, StrAsc const &port_name_);

   protected:
      /**
       * Overloads the base class version to handle a socket read event.
       */
      virtual void on_read();

      /**
       * Overloads the base class version to handle a socket error.
       */
      virtual void on_socket_error(int error_code);

   private:
      Application *application;
      StrAsc const port_name;
      enum state_type
      {
         state_local,
         state_read_id,
         state_transparent
      } state;
      byte rx_buff[1024];
      Csi::SharedPtr<Port> port;
      StrAsc digits_buff;
      friend class Port;
   };

   
   class Application: public Csi::SocketTcpService
   {
   public:
      /**
       * Constructor
       */
      Application(uint2 tcp_port_, StrAsc const &serial_port_):
         tcp_port(tcp_port_),
         serial_port(serial_port_)
      {
         set_allow_ipv6(true);
         start_service(tcp_port);
      }

   protected:
      /**
       * Overloads the base class version
       */
      virtual void on_accept(SOCKET new_connection)
      {
         if(proxy == 0)
            proxy.bind(new Proxy(this, new_connection, serial_port));
         else
            ::closesocket(new_connection);
      } // on_accept
      

      /**
       * Overloads the base class version
       */
      virtual void on_socket_error(int error_code)
      {
         Csi::SocketException e("socket listen failed", error_code);
         std::cout << e.what() << std::endl;
         Csi::Event::post_quit_message(1);
      }
      
   private:
      /**
       * Specifies the TCP service port.
       */
      uint2 tcp_port;

      /**
       * Specifies the serial port
       */
      StrAsc serial_port;

      /**
       * Specifies the current connection.
       */
      Csi::SharedPtr<Proxy> proxy;

      friend class Proxy;
   };

   
   Proxy::Proxy(Application *application_, SOCKET socket_handle, StrAsc const &port_name):
      SocketTcpSock(socket_handle),
      application(application_),
      state(state_local)
   {
      port.bind(new Port(this, port_name));
   } // consructor


   void Proxy::on_read()
   {
      // do any book keeping in the base class. 
      SocketTcpSock::on_read();

      // now we can process the data
      Csi::ByteQueue &queue(get_read_buffer());
      while(queue.size() > 0)
      {
         if(state == state_local)
         {
            if(queue[0] == 'S')
            {
               write("S", 1);
               queue.pop(1);
               state = state_read_id;
            }
            else if(queue[0] == '\r')
            {
               queue.pop(1);
               write("\r\n#", 3);
            }
         }
         else if(state == state_read_id)
         {
            if(queue[0] == '\r')
            {
               write("\r\n$", 3);
               queue.pop(1);
               state = state_transparent;
            }
            else
            {
               byte ch;
               queue.pop(&ch, 1);
               write(&ch, 1);
            }
         }
         else if(state == state_transparent)
         {
            // we might want to drop digits from a final storage collect command.
            char ch;
            queue.pop(&ch, 1);
            if(ch >= '0' && ch <= '9')
            {
               digits_buff.append(ch);
               port->write(&ch, 1);
            }
            else if(ch == 'F')
            {
               if(digits_buff.length() > 0)
               {
                  uint4 arg(strtoul(digits_buff.c_str(), 0, 10));
                  if(arg < 200)
                     port->write(&ch, 1);
                  digits_buff.cut(0);
               }
            }
            else
            {
               digits_buff.cut(0);
               port->write(&ch, 1);
            }
         }
      }
   } // on_read


   void Proxy::on_socket_error(int error_code)
   {
      Csi::SocketException e("proxy shutting down", error_code);
      std::cout << e.what() << std::endl;
      application->proxy.clear();
   } // on_socket_error


   Port::Port(Proxy *proxy_, StrAsc const &port):
      proxy(proxy_)
   { open(port, 9600); }


   void Port::on_error(char const *message)
   {
      std::cout << "serial port failure: " << message << std::endl;
      proxy->on_socket_error(0);
   } // on_error


   void Port::on_read()
   {
      byte buff[1024];
      uint4 count;
      while((count = read(buff, sizeof(buff))) > 0)
         proxy->write(buff, count);
   } // on_read
};



int main(int argc, char const *argv[])
{
   int rtn(0);
   try
   {
      // initialise event dispatch
      Csi::MessageWindow::initialise(::GetModuleHandle(0));
      Csi::Event::set_dispatcher(new Csi::Win32Dispatch);
      Csi::Win32::WinsockInitialisor sockets_init;
      
      // parse the command line
      Csi::CommandLine parser;
      Csi::set_command_line(argc, argv);
      StrAsc temp;
      StrAsc serial_port;
      uint4 tcp_port(0);
      parser.parse_command_line(Csi::get_command_line());
      if(!parser.get_argument(temp, 1))
         throw std::invalid_argument("expected the TCP port");
      if(!parser.get_argument(serial_port, 2))
         throw std::invalid_argument("expected the serial port");
      tcp_port = strtoul(temp.c_str(), 0, 10);
      if(tcp_port == 0 || tcp_port > 65535)
         throw std::invalid_argument("invalid TCP port");
      
      // we can now drive the event dispatcher.
      MSG message;
      Application app(static_cast<uint2>(tcp_port), serial_port);
      while(::GetMessage(&message, 0, 0, 0))
      {
         ::TranslateMessage(&message);
         ::DispatchMessage(&message);
      }
   }
   catch(std::exception &e)
   {
      std::cout << "unhandled exception: " << e.what() << std::endl;
      rtn = 1;
   }
   return rtn;
}
