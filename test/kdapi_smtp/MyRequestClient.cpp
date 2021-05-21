#include "MyRequestClient.h"


MyRequestClient::MyRequestClient(Csi::Json::ObjectHandle json_doc_):
   json_doc(json_doc_),
   total_sent_bytes(0),
   the_request(0)
{
   oneshot.bind(new OneShot);
   Csi::OStrAscStream json_out;
   json_doc->format(json_out);
   send_buff.push(json_out.str().c_str(), json_out.str().length());
}


MyRequestClient::~MyRequestClient()
{ }


bool MyRequestClient::on_failure(Csi::HttpClient::Request *request)
{
   std::cout << "HTTP Failure: " << request->get_response_code()
      << " " << request->get_response_description()
      << std::endl;

   Csi::Event::post_quit_message(0);
   return true;
}


void MyRequestClient::on_connected(Csi::HttpClient::Request *request)
{
   the_request = request;
   std::cout << "on_connected()" << std::endl;
   request->set_content_type("application/json");
   oneshot->arm(this, 100);
}


void MyRequestClient::on_response_header(Csi::HttpClient::Request *request)
{
   std::cout << "response header parsed: "
      << request->get_response_code()
      << " " << request->get_response_description()
      << std::endl;
}


void MyRequestClient::on_response_data(Csi::HttpClient::Request *request)
{
   Csi::ByteQueue &buff(request->get_receive_buff());
   char temp[1024];
   uint4 count;

   while((count = buff.pop(temp, sizeof(temp))) > 0)
   {
      std::cout.write(temp, count);
   }
}


bool MyRequestClient::on_response_complete(Csi::HttpClient::Request *request)
{
   std::cout << "on_response_complete" << std::endl;

   Csi::Event::post_quit_message(0);
   return true;
}


void MyRequestClient::onOneShotFired(uint4 id)
{
   //static const uint4 send_size = 512;
   static const uint4 send_size = USHRT_MAX;
   if(the_request)
   {
      char temp[send_size];
      uint4 popped_bytes = send_buff.pop(temp, send_size);
      the_request->add_bytes(temp, popped_bytes, false);
      total_sent_bytes += send_size;
   }

   if(send_buff.size() == 0)
      the_request->add_bytes(0, 0, true);
   else
      oneshot->arm(this, 1000);
}