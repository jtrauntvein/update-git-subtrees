/* main.cpp

   Copyright (C) 2017, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 03 April 2017
   Last Change: Thursday 04 January 2018
   Last Commit: $Date: 2018-01-04 16:48:24 -0600 (Thu, 04 Jan 2018) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Xml.Element.h"
#include "Csi.SmtpSender.h"
#include "Csi.Alarms.EmailProfile.h"
#include "Csi.fstream.h"
#include "Csi.Events.h"
#include "Csi.Win32Dispatch.h"
#include "Csi.Win32.WinsockInitialisor.h"
#include <iostream>


class MyClient: public Csi::SmtpSenderClient
{
public:
   typedef Csi::SmtpSender sender_type;
   virtual void on_complete(sender_type *sender, outcome_type outcome)
   {
      std::cout << "send is complete: ";
      format_outcome(std::cout, outcome);
      std::cout << std::endl;
      Csi::Event::post_quit_message(0);
   }

   virtual void on_bytes_sent(sender_type *sender, void const *buff, uint4 buff_len)
   {
      StrBin content(buff, buff_len);
      content.append(0);
      std::cout << "sent: " << content.getContents() << "\n";
   }

   virtual void on_bytes_received(sender_type *sender, void const *buff, uint4 buff_len)
   {
      StrBin content(buff, buff_len);
      content.append(0);
      std::cout << "received: " << content.getContents() << "\n";
   }

   virtual void on_log(sender_type *sender, StrAsc const &log)
   {
      std::cout << "log: " << log << "\n";
   }
   
};


int main(int argc, char const *argv[])
{
   int rtn(0);
   StrAsc temp;
   try
   {
      // we need to initialise the application
      Csi::MessageWindow::initialise(::GetModuleHandle(0));
      Csi::Event::set_dispatcher(new Csi::Win32Dispatch);
      Csi::Win32::WinsockInitialisor sockets_init;

      // we need to open the test configuration
      Csi::Xml::Element config(L"smtp-sender");
      if(argc < 2)
         throw std::invalid_argument("no test configuration file specified");
      Csi::ifstream input(argv[1], std::ios::binary);
      config.input(input);

      // we need to build up an email profile from the config
      auto profile_xml(config.find_elem(L"profile"));
      Csi::Alarms::EmailProfile profile;
      profile.read(*profile_xml);

      // we ncan now use this profile to configure the SMTP sender
      MyClient client;
      Csi::SmtpSender sender;
      auto subject_xml(config.find_elem(L"subject"));
      auto message_xml(config.find_elem(L"message"));
      auto attachments(config.find_elem(L"attachments", 0, true));
      
      Csi::SmtpSender::set_gateway_model(config.get_attr_wstr(L"gateway-model"));
      Csi::SmtpSender::set_gateway_serial_no(config.get_attr_wstr(L"gateway-serial"));
      sender.set_use_gateway(profile.get_use_gateway());
      sender.set_server_address(profile.get_smtp_server());
      sender.set_user_name(profile.get_smtp_user_name());
      sender.set_password(profile.get_smtp_password());
      sender.set_from_address(profile.get_from_address());
      sender.set_to_addresses(profile.get_to_address());
      sender.set_cc_addresses(profile.get_cc_address());
      sender.set_bcc_addresses(profile.get_bcc_address());
      sender.set_subject(subject_xml->get_cdata_wstr());
      sender.set_message(message_xml->get_cdata_wstr());
      for(auto ai = attachments->begin(); ai != attachments->end(); ++ai)
         sender.add_attachment((*ai)->get_cdata_wstr());
      sender.start(&client);
      
      // we will now carry the message loop and let the application events drive the rest of the
      // flow.
      MSG message;
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
} // main
