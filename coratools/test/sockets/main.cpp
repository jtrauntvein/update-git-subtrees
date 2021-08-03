/* main.cpp<2>

   Copyright (C) 2021, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 27 January 2021
   Last Change: Wednesday 27 January 2021
   Last Commit: $Date: 2021-01-27 10:18:08 -0600 (Wed, 27 Jan 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32Dispatch.h"
#include "Csi.Win32.WinsockInitialisor.h"
#include "Csi.SocketTcpSock.h"
#include "Csi.SocketTcpService.h"
#include "Csi.SocketException.h"
#include "Csi.CommandLine.h"
#include "Csi.Utils.h"
#include <iostream>
#include <deque>


namespace
{
   class TestServer;

   
   /**
    * Defines an object that acts as a TCP client.
    */
   class TestClient: public Csi::SocketTcpSock
   {
   private:
      /**
       * Specifies the server address.
       */
      StrAsc server_address;

      /**
       * Specifies the server port.
       */
      uint2 server_port;

      /**
       * Specifies the buffer used to process data coming from the socket.
       */
      char rx_buff[512];

      /**
       * Specifies the server that created this object.
       */
      TestServer *server;

   public:
      /**
       * Constructor
       *
       * @param args Specifies the command line arguments.
       */
      TestClient(Csi::CommandLine &args):
         server(0)
      {
         StrAsc temp;
         if(!args.get_argument(server_address, 2))
            throw std::invalid_argument("expected the server address");
         if(!args.get_argument(temp, 3))
            throw std::invalid_argument("expected the server port");
         server_port = static_cast<uint2>(std::strtoul(temp.c_str(), 0, 10));
         std::cout << "connecting to " << server_address << " port " << server_port << std::endl;
         open(server_address.c_str(), server_port);
      }

      /**
       * Construct from client connection.
       *
       * @param server_ Specifies the server object that accepted the connection.
       *
       * @param socket Specifies the socket handle.
       */
      TestClient(TestServer *server_, SOCKET socket):
         server(server_),
         SocketTcpSock(socket)
      {
         std::cout << "client connection from " << get_connected_address() << std::endl;
      }

      /**
       * Destructor
       */
      virtual ~TestClient()
      { }

      /**
       * Overrides the base class to report the connection.
       */
      virtual void on_connected(Csi::SocketAddress const &connected_address) override
      {
         std::cout << "connected on interface " << connected_address << std::endl;
      }

      /**
       * Overrides the base class version to handle the read event.
       */
      virtual void on_read() override
      {
         auto rx(get_read_buffer());
         uint4 rcd;
         while((rcd = rx.pop(rx_buff, sizeof(rx_buff))) > 0)
            std::cout.write(rx_buff, rcd);
      }

      /**
       * Overrides the error handler
       */
      virtual void on_socket_error(int error_code) override;
   };


   /**
    * Defines an object that provides a TCP service.
    */
   class TestServer: public Csi::SocketTcpService
   {
   private:
      /**
       * Specifies the port on which this service will listen.
       */
      uint2 server_port;

      /**
       * Specifies the collection of clients currently being serviced.
       */
      typedef Csi::SharedPtr<TestClient> client_handle;
      std::deque<client_handle> clients;

   public:
      /**
       * Constructor
       *
       * @param args Specifies the parsed command line arguments.
       */
      TestServer(Csi::CommandLine &args)
      {
         StrAsc temp;
         if(!args.get_argument(temp, 2))
            throw std::invalid_argument("expected the service port");
         server_port = static_cast<uint2>(strtoul(temp.c_str(), 0, 10));
         std::cout << "listening on port " << server_port << std::endl;
         start_service(server_port);
      }

      /**
       * Destructor
       */
      virtual ~TestServer()
      { }

      /**
       * Removes the specified client object.
       */
      void close_client(TestClient *client)
      {
         auto ci(std::find_if(clients.begin(), clients.end(), Csi::HasSharedPtr<TestClient>(client)));
         if(ci != clients.end())
            clients.erase(ci);
      }

   protected:
      /**
       * Overrides the base class to handle an incoming connection.
       */
      virtual void on_accept(SOCKET new_connection) override
      {
         clients.push_back(new TestClient(this, new_connection));
      }

      /**
       * Overrides the base class to handle a service error.
       */
      virtual void on_socket_error(int error_code) override
      {
         throw Csi::SocketException("listening failed", error_code);
      }
   };


   void TestClient::on_socket_error(int error_code)
   {
      Csi::SocketException error("connection failed", error_code);
      SocketTcpSock::on_socket_error(error_code);
      if(server == 0)
         throw error;
      else
      {
         std::cout << error.what() << std::endl;
         server->close_client(this);
      }
   } // on_socket_error
   
};


int main(int argc, char const *argv[])
{
   int rtn(0);
   try
   {
      // process the command line
      Csi::CommandLine args;
      StrAsc role;
      Csi::set_command_line(argc, argv);
      args.parse_command_line(Csi::get_command_line());
      if(!args.get_argument(role, 1))
         throw std::invalid_argument("expected the test role (client/server)");
      
      // initialise the coratools library components.
      Csi::MessageWindow::initialise(::GetModuleHandle(0));
      Csi::Event::set_dispatcher(new Csi::Win32Dispatch);
      Csi::Win32::WinsockInitialisor sockets_init;

      // start the test
      Csi::SharedPtr<TestClient> client;
      Csi::SharedPtr<TestServer> server;
      if(role == "client")
         client.bind(new TestClient(args));
      else if(role == "server")
         server.bind(new TestServer(args));
      else
         throw std::invalid_argument("invalid role specified");
      
      // use the windows message loop to drive the events engine
      MSG message;
      while(GetMessage(&message, 0, 0, 0))
      {
         TranslateMessage(&message);
         DispatchMessage(&message);
      }
   }
   catch(std::exception &e)
   {
      std::cout << "uncaught exception: " << e.what();
      rtn = 1;
   }
   Csi::Event::set_dispatcher(0);
   Csi::MessageWindow::uninitialise();
   return rtn;
} // main
