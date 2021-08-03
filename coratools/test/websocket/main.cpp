/* main.cpp

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 27 October 2015
   Last Change: Saturday 14 November 2015
   Last Commit: $Date: 2015-11-14 12:19:20 -0600 (Sat, 14 Nov 2015) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.HttpClient.WebSocket.h"
#include "Csi.Win32Dispatch.h"
#include "Csi.Win32.WinsockInitialisor.h"
#include "Csi.CommandLine.h"
#include "Csi.Utils.h"
#include <iostream>
#include <deque>


class MyTest: public Csi::HttpClient::WebSocketClient
{
private:
   /**
    * Specifies the websocket
    */
   typedef Csi::HttpClient::WebSocket websock_type;
   Csi::SharedPtr<websock_type> websock;

   /**
    * Specifies the list of files with messages to send.
    */
   typedef std::deque<StrAsc> messages_type;
   messages_type messages;

   /**
    * Specifies the user name
    */
   StrAsc user_name;

   /**
    * Specifies the password
    */
   StrAsc password;

   /**
    * Specifies the server URI
    */
   Csi::Uri uri;

   /**
    * Specifies the last op code received.
    */
   Csi::HttpClient::websock_op_code last_op_code;

   /**
    * Used to format incoming messages.
    */
   StrAsc message;

public:
   /**
    * Constructor
    */
   MyTest(Csi::Uri const &uri_, StrAsc const &user_name_, StrAsc const &password_):
      uri(uri_),
      user_name(user_name_),
      password(password_),
      last_op_code(Csi::HttpClient::websock_op_binary)
   { }

   /**
    * Adds a file name for the message to send.
    */
   void add_message(StrAsc const &file_name)
   { messages.push_back(file_name); }

   /**
    * Overloads the connected event handler.
    */
   virtual void on_connected(websock_type *sender);

   /**
    * Overloads the failure notification
    */
   virtual void on_failure(websock_type *sender, failure_type failure, int http_response);

   /**
    * Overloads the message notification
    */
   virtual void on_message(
      websock_type *sender,
      void const *content,
      uint4 content_len,
      Csi::HttpClient::websock_op_code op_code,
      bool fin);

   /**
    * Starts the connection.
    */
   void start();
};


void MyTest::on_connected(websock_type *sender)
{
   for(auto mi = messages.begin(); mi != messages.end(); ++mi)
   {
      auto &file_name(*mi);
      FILE *message(Csi::open_file(file_name.c_str(), "rb"));
      if(message)
      {
         StrAsc content;
         content.readFile(message, Csi::file_length(message));
         fclose(message);
         if(content.length() > 0)
            websock->send_message(content.c_str(), content.length());
      }
   }
} // on_connected


void MyTest::on_failure(
   websock_type *sender, failure_type failure, int http_resp)
{
   std::cerr << "websock failure: " << failure << " with response " << http_resp << std::endl;
} // on_failure


void MyTest::on_message(
   websock_type *sender,
   void const *content,
   uint4 content_len,
   Csi::HttpClient::websock_op_code op_code,
   bool fin)
{
   message.cut(0);
   if(op_code == Csi::HttpClient::websock_op_text)
      message.append(static_cast<char const *>(content), content_len);
   else
      message.encodeHex(content, content_len);
   std::cout << "message received op=" << op_code << " finished=" << fin << "\n"
             << message << "\n"
             << "message end" << std::endl;
   last_op_code = op_code;
} // on_message


void MyTest::start()
{
   websock_type::log_handle log(new Csi::LogByte("c:\\temp", "websocket$.log"));
   websock.bind(new websock_type);
   log->set_time_based_baling(true, Csi::LgrDate::msecPerHour);
   log->setEnable(true);
   websock->set_log(log);
   websock->connect(this, uri, "com.campbellsci.webdata", user_name, password);
} // start


int main(int argc, char const *argv[])
{
   int rtn(0);
   try
   {
      // process the command line
      Csi::CommandLine parser;
      Csi::Uri server_uri;
      StrAsc temp;
      StrAsc user_name;
      StrAsc password;
      
      Csi::set_command_line(argc, argv);
      parser.add_expected_option("user");
      parser.add_expected_option("password");
      parser.parse_command_line(Csi::get_command_line());
      if(parser.get_argument(temp, 1))
         server_uri = temp;
      else
         throw std::invalid_argument("server URI not specified");
      if(parser.get_option_value("user", temp))
         user_name = temp;
      if(parser.get_option_value("password", temp))
         password = temp;
      
      // initialise the coratools library
      Csi::MessageWindow::initialise(::GetModuleHandle(0));
      Csi::Event::set_dispatcher(new Csi::Win32Dispatch);
      Csi::Win32::WinsockInitialisor sockets_init;

      // start the test
      MyTest test(server_uri, user_name, password);
      for(uint4 i = 2; i < parser.args_size(); ++i)
         test.add_message(parser[i]);
      test.start();

      // start the event dispatch loop
      MSG message;
      while(GetMessage(&message, 0, 0, 0))
      {
         TranslateMessage(&message);
         DispatchMessage(&message);
      }
   }
   catch(std::exception &e)
   {
      std::cerr << "unhandled exception: \"" << e.what() << "\"" << std::endl;
      rtn = 1;
   }

   // clear the coratools library
   Csi::Event::set_dispatcher(0);
   Csi::MessageWindow::uninitialise();
   return rtn;
} // main
