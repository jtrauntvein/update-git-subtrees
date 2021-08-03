/* main.cpp

   Copyright (C) 2017, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 20 July 2017
   Last Change: Wednesday 15 November 2017
   Last Commit: $Date: 2017-11-29 17:18:45 -0600 (Wed, 29 Nov 2017) $
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.SoftwareLicense.h"
#include "Csi.Win32Dispatch.h"
#include "Csi.Win32.WinsockInitialisor.h"
#include "Csi.CommandLine.h"
#include "Csi.Utils.h"
#include <iostream>
#include <iomanip>
#include <cctype>


class MyWatcher: public Csi::HttpClient::ConnectionWatcher
{
public:
   /**
    * Overloads the log comment handler
    */
   virtual void on_log_comment(Csi::HttpClient::Connection *sender, StrAsc const &comment)
   {
      std::cout << comment << "\n";
   }

   /**
    * Called when there is data that has been read or written.
    */
   virtual void on_data(Csi::HttpClient::Connection *sender, void const *buff, size_t buff_len, bool received)
   {
      Csi::LgrDate now(Csi::LgrDate::system());
      byte const *bytes(static_cast<byte const *>(buff));
      size_t const max_per_line(16);
      
      for(size_t i = 0; i < buff_len; i += max_per_line)
      {
         size_t j;
         now.format(std::cout, "%H:%M:%S.%3 ");
         if(received)
            std::cout << "R ";
         else
            std::cout << "T ";
         for(j = 0; j < max_per_line && i + j < buff_len; ++j)
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (uint2)bytes[i + j] << ' ';
         while(j++ < max_per_line)
            std::cout << "   ";
         std::cout << " ";
         for(j = 0; j < max_per_line && i + j < buff_len; ++j)
         {
            if(std::isprint(bytes[i + j]) && bytes[i + j] != '\r' && bytes[i + j] != '\n')
               std::cout << static_cast<char>(bytes[i + j]);
            else
               std::cout << '.';
         }
         std::cout << "\n";
      }
   }
};


class MyDelegate: public Csi::SoftwareLicense::LicenseClientDelegate
{
public:
   Csi::Json::ObjectHandle selected_version;
   bool update_after_check;
   bool changelog_after_check;

   MyDelegate():
      update_after_check(false),
      changelog_after_check(false)
   { }
   
   typedef Csi::SoftwareLicense::LicenseClient license_type;
   virtual void on_starting_renewal(license_type *sender)
   {
      Csi::LgrDate now(Csi::LgrDate::system());
      now.format(std::cout, "%Y-%m-%d %H:%M:%S%x: starting license renewal\n");
   }

   virtual void on_renewal_complete(license_type *sender, renewal_outcome_type outcome)
   {
      Csi::LgrDate now(Csi::LgrDate::system());
      now.format(std::cout, "%Y-%m-%d %H:%M:%S%x: license renewal complete with outcome ");
      std::cout << outcome << "\n";
      if(outcome == renewal_outcome_success)
      {
         Csi::Json::Object license_json;
         sender->get_certificate()->write(license_json, false);
         license_json.format(std::cout);
         std::cout << "\n";
      }
   }

   virtual void on_check_version_complete(
      license_type *sender, check_version_outcome_type outcome, Csi::Json::ArrayHandle &versions)
   {
      Csi::LgrDate now(Csi::LgrDate::system());
      Csi::VersionNumber newest_version;

      now.format(std::cout, "%Y-%m-%d %H:%M:%S%x: check version complete: ");
      license_type::format_check_version_outcome(std::cout, outcome);
      std::cout << "\n";
      if(versions != 0)
      {
         versions->format(std::cout);
         selected_version.clear();
         for(auto vi = versions->begin(); vi != versions->end(); ++vi)
         {
            Csi::Json::ObjectHandle version_json(*vi);
            StrAsc version_str(version_json->get_property_str("version"));
            StrAsc version_build(version_json->get_property_str("build"));
            Csi::VersionNumber my_version;
            
            if(version_build.length() > 0)
            {
               version_str.append(".");
               version_str.append(version_build);
            }
            my_version = version_str.c_str();
            if(my_version > newest_version)
            {
               selected_version = version_json;
               newest_version = my_version;
            }
         }
         if(changelog_after_check && selected_version != 0)
            sender->get_changelog(selected_version->get_property_str("version").c_str());
         if(update_after_check && selected_version != 0)
            sender->update(selected_version);
      }
   }

   virtual bool on_update_data(license_type *sender, Csi::HttpClient::Request &request)
   {
      if(request.get_response_code() == 200)
      {
         auto &rx_buff(request.get_receive_buff());
         rx_buff.pop(rx_buff.size());
      }
      return true;
   }
   
   virtual void on_update_complete(license_type *sender, update_outcome_type outcome)
   {
      Csi::LgrDate now(Csi::LgrDate::system());
      now.format(std::cout, "%Y-%m-%d %H:%M:%S%x: update complete: ");
      license_type::format_update_outcome(std::cout, outcome);
      std::cout << "\n";
   }

   virtual void on_changelog_complete(
      license_type *sender, changelog_outcome_type outcome, Csi::Json::ObjectHandle &change_log)
   {
      Csi::LgrDate now(Csi::LgrDate::system());
      now.format(std::cout, "%Y-%m-%d %H:%M:%S%x: change log complete: ");
      license_type::format_changelog_outcome(std::cout, outcome);
      std::cout << "\n";
      if(change_log != 0)
         change_log->format(std::cout);
   }
};


int main(int argc, char const *argv[])
{
   int rtn(0);
   try
   {
      // we need to initialise coratools
      Csi::MessageWindow::initialise(::GetModuleHandle(0));
      Csi::Event::set_dispatcher(new Csi::Win32Dispatch);
      Csi::Win32::WinsockInitialisor sockets_init;
      Csi::CommandLine command_line;
      Csi::SoftwareLicense::LicenseClient::node_server_type server_type = Csi::SoftwareLicense::LicenseClient::node_server_type_production;
      StrAsc temp;
      StrAsc product_model;
      StrAsc license_model;
      StrAsc version;

      enable_trace(false);
      command_line.add_expected_option("node-server");
      Csi::set_command_line(argc, argv);
      command_line.parse_command_line(Csi::get_command_line());
      if(command_line.get_option_value("node-server", temp))
      {
         if(temp == "local")
            server_type = Csi::SoftwareLicense::LicenseClient::node_server_type_local;
         else if(temp == "staging")
            server_type = Csi::SoftwareLicense::LicenseClient::node_server_type_staging;
      }
      if(!command_line.get_argument(product_model, 1))
         throw std::invalid_argument("expected the product model string");
      if(!command_line.get_argument(license_model, 2))
         throw std::invalid_argument("expected the license model string");
      if(!command_line.get_argument(version, 3))
         throw std::invalid_argument("expected the version string");
      
      // we need to create the objects that we will be testing
      Csi::SoftwareLicense::LicenseClient client(product_model, license_model, version.c_str(), nullptr, 60000, server_type);
      MyDelegate delegate;
      uint4 op_index(4);
      client.set_watcher(new MyWatcher);
      client.start(&delegate);
      while(op_index < command_line.args_size())
      {
         StrAsc op_name(command_line[op_index]);
         if(op_name == "addon")
         {
            StrAsc child_model;
            StrAsc child_serial_no;
            if(!command_line.get_argument(child_model, op_index + 1))
               throw std::invalid_argument("expected the child model for the addon command");
            if(!command_line.get_argument(child_serial_no, op_index + 2))
               throw std::invalid_argument("expected the child serial number for the addon command");
            client.register_addon(child_model, child_serial_no);
            op_index += 3;
         }
         else if(op_name == "check")
         {
            client.check_version();
            ++op_index;
         }
         else if(op_name == "update")
         {
            delegate.update_after_check = true;
            ++op_index;
         }
         else if(op_name == "changelog")
         {
            delegate.changelog_after_check = true;
            ++op_index;
         }
      }
      
      // we need to drive the windows message pump
      MSG message;
      while(::GetMessage(&message, 0, 0, 0))
      {
         ::TranslateMessage(&message);
         ::DispatchMessage(&message);
      }
   }
   catch(std::exception &e)
   {
      rtn = 1;
      std::cout << "unhandled exception: " << e.what() << std::endl;
   }
   return rtn;
} // main
