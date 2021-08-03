/* main.cpp

   Copyright (C) 2010, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 11 October 2010
   Last Change: Tuesday 21 June 2011
   Last Commit: $Date: 2011-06-21 09:15:32 -0600 (Tue, 21 Jun 2011) $
   Last Changed by: $Author: jon $

*/

#pragma warning(disable : 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.HttpClient.h"
#include "Csi.Xml.Element.h"
#include "Csi.TlsContext.h"
#ifdef _WIN32
#include "Csi.Win32.WinsockInitialisor.h"
#include "Csi.Win32Dispatch.h"
#else
#include "Csi.SimpleDispatch.h"
#endif
#include <fstream>


////////////////////////////////////////////////////////////
// class RequestBase
////////////////////////////////////////////////////////////
class RequestBase: public Csi::HttpClient::RequestClient
{
protected:
   ////////////////////////////////////////////////////////////
   // request
   ////////////////////////////////////////////////////////////
   typedef Csi::SharedPtr<Csi::HttpClient::Request> request_handle;
   request_handle request;

   ////////////////////////////////////////////////////////////
   // input_file
   ////////////////////////////////////////////////////////////
   StrAsc input_file_name;

   ////////////////////////////////////////////////////////////
   // output_file
   ////////////////////////////////////////////////////////////
   StrAsc output_file_name;
   Csi::SharedPtr<std::ofstream> output_file;
   
public:
   ////////////////////////////////////////////////////////////
   // constructor
   ////////////////////////////////////////////////////////////
   typedef Csi::HttpClient::Request request_type;
   typedef Csi::HttpClient::Connection connection_type;
   RequestBase(connection_type &connection, Csi::Xml::Element &request_xml);

   ////////////////////////////////////////////////////////////
   // on_failure
   ////////////////////////////////////////////////////////////
   virtual bool on_failure(request_type *request);

   ////////////////////////////////////////////////////////////
   // on_connected
   ////////////////////////////////////////////////////////////
   virtual void on_connected(request_type *request);

   ////////////////////////////////////////////////////////////
   // on_response_header
   ////////////////////////////////////////////////////////////
   virtual void on_response_header(request_type *request);

   ////////////////////////////////////////////////////////////
   // on_response_data
   ////////////////////////////////////////////////////////////
   virtual void on_response_data(request_type *request);

   ////////////////////////////////////////////////////////////
   // on_response_complete
   ////////////////////////////////////////////////////////////
   virtual bool on_response_complete(request_type *request);

   
};


////////////////////////////////////////////////////////////
// class Application
////////////////////////////////////////////////////////////
class Application
{
public:
   ////////////////////////////////////////////////////////////
   // constructor
   ////////////////////////////////////////////////////////////
   Application(Csi::Xml::Element &requests_xml);

   ////////////////////////////////////////////////////////////
   // destructor
   ////////////////////////////////////////////////////////////
   virtual ~Application();

   ////////////////////////////////////////////////////////////
   // on_request_complete
   ////////////////////////////////////////////////////////////
   void on_request_complete(RequestBase *request);

private:
   ////////////////////////////////////////////////////////////
   // connection
   ////////////////////////////////////////////////////////////
   typedef Csi::HttpClient::Connection connection_type;
   Csi::SharedPtr<connection_type> connection;

   ////////////////////////////////////////////////////////////
   // requests
   ////////////////////////////////////////////////////////////
   typedef Csi::SharedPtr<RequestBase> request_handle;
   typedef std::list<request_handle> requests_type;
   requests_type requests;
   
} *the_application = 0;


Application::Application(Csi::Xml::Element &requests)
{
   using namespace Csi::Xml;
   the_application = this;
   connection.bind(new connection_type);
   for(Element::iterator ri = requests.begin(); ri != requests.end(); ++ri)
      this->requests.push_back(new RequestBase(*connection, **ri)); 
} // constructor


Application::~Application()
{
} // destructor


void Application::on_request_complete(RequestBase *request)
{
   if(!requests.empty() && requests.front() == request)
      requests.pop_front();
   else
      assert(false);
   if(requests.empty())
      Csi::Event::post_quit_message(0);
} // on_request_complete


////////////////////////////////////////////////////////////
// class RequestBase definitions
////////////////////////////////////////////////////////////
RequestBase::RequestBase(
   connection_type &connection, Csi::Xml::Element &request_xml)
{
   // we need to determine the method for this request
   request_type::method_type method(request_type::method_get);
   if(request_xml.get_name() == L"GET")
      method = request_type::method_get;
   else if(request_xml.get_name() == L"POST")
      method = request_type::method_post;
   else if(request_xml.get_name() == L"PUT")
      method = request_type::method_put;
   else
      throw std::invalid_argument("invalid request method specified");
   
   // we need to parse the input file name (if any) from the request
   if(request_xml.has_attribute(L"input"))
      input_file_name = request_xml.get_attr_str(L"input");
   if(request_xml.has_attribute(L"output"))
      output_file_name = request_xml.get_attr_str(L"output");
   
   // we can now create the request object
   request.bind(new request_type(this, request_xml.get_attr_str(L"uri"), method));
   if(input_file_name.length() == 0)
   {
      StrAsc send_content(request_xml.get_cdata_str());
      if(send_content.length() > 0)
         request->add_bytes(send_content.c_str(), send_content.length(), true);
   }
   else
      request->add_bytes(0, 0, false);
   if(request_xml.has_attribute(L"user") || request_xml.has_attribute(L"password"))
   {
      request->set_authentication(
         request_xml.get_attr_str(L"user"), request_xml.get_attr_str(L"password"));
   }
   connection.add_request(request);
} // constructor


bool RequestBase::on_failure(request_type *request)
{
   std::cerr << "HTTP connection failure for request" << std::endl;
   the_application->on_request_complete(this);
   return false;
} // on_failure


void RequestBase::on_connected(request_type *request)
{
   std::cerr << "request connected" << std::endl;
   if(input_file_name.length() > 0)
   {
      // we can now send the file data for the request
      FILE *input(fopen(input_file_name.c_str(), "rb"));
      if(input)
      {
         char buff[1024];
         uint4 bytes_read;
         while((bytes_read = fread(buff, 1, sizeof(buff), input)) > 0)
            request->add_bytes(buff, bytes_read, false);
      } 
      request->add_bytes(0, 0, true);
   }
} // on_connected


void RequestBase::on_response_header(request_type *request)
{
   std::cerr << "response header parsed: "
             << request->get_response_code()
             << " " << request->get_response_description()
             << std::endl; 
} // on_response_header


void RequestBase::on_response_data(request_type *request)
{
   Csi::ByteQueue &buff(request->get_receive_buff());
   char temp[1024];
   uint4 count;
   
   while((count = buff.pop(temp, sizeof(temp))) > 0)
   {
      if(output_file_name.length() > 0)
      {
         if(output_file == 0)
            output_file.bind(new std::ofstream(output_file_name.c_str(), std::ios::binary));
         output_file->write(temp, count);
      }
      else
         std::cout.write(temp, count);
   }
} // on_response_data


bool RequestBase::on_response_complete(request_type *request)
{
   output_file.clear();
   the_application->on_request_complete(this);
   return true;
} // on_response_complete


int main(int argc, char *argv[])
{
   int rtn(0);
   try
   {
      // we need to initialise coratools library components
#ifdef WIN32
      Csi::MessageWindow::initialise(::GetModuleHandle(0));
      Csi::Event::set_dispatcher(new Csi::Win32Dispatch);
      Csi::Win32::WinsockInitialisor sockets_init; 
#else
      Csi::SimpleDispatch dispatcher(new Csi::SimpleDispatcher);
      Csi::Event::set_dispatcher(dispatcher);
#endif
      Csi::SocketTcpSock::set_tls_context(new Csi::TlsContext());
      
      // We need to parse the input
      Csi::Xml::Element requests_xml(L"requests");
      if(argc < 2)
         throw std::invalid_argument("expected the input file name");
      std::ifstream input(argv[1], std::ios::binary);
      if(!input)
         throw Csi::OsException("failed to open the input file");
      requests_xml.input(input);

      // we are now ready to perform some application processing
      Application app(requests_xml);
#ifdef WIN32
      MSG message;
      while(GetMessage(&message, 0, 0, 0))
      {
         TranslateMessage(&message);
         DispatchMessage(&message);
      }
#else
      while(dispatcher->do_dispatch())
         0;
#endif
      
   }
   catch(std::exception &e)
   {
      rtn = 1;
      std::cerr << "Application exception caught: " << e.what();
   }
   return rtn;
} // main
