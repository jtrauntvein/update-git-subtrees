/* main.cpp

   Copyright (C) 2009, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Monday 22 June 2009
   Last Change: Monday 22 June 2009
   Last Commit: $Date: 2009-06-23 10:03:40 -0600 (Tue, 23 Jun 2009) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.DeviceMapper.h"
#include "Cora.LgrNet.SettingsEnumerator.h"
#include "Cora.Device.SettingsEnumerator.h"
#include "Csi.SocketConnection.h"
#include "Csi.CommandLine.h"
#include "Csi.Utils.h"
#include "Csi.Win32Dispatch.h"
#include "Csi.Win32.WinsockInitialisor.h"
#include <iostream>
#include <algorithm>


namespace
{
   ////////////////////////////////////////////////////////////
   // class Application
   ////////////////////////////////////////////////////////////
   class Application:
      public Cora::LgrNet::DeviceMapper::client_type,
      public Cora::LgrNet::SettingsEnumerator::client_type,
      public Cora::Device::SettingsEnumerator::client_type
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      Application(StrAsc const &address, uint2 port);
      
      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      virtual ~Application();
      
      ////////////////////////////////////////////////////////////
      // on_setting_changed (lgrnet)
      ////////////////////////////////////////////////////////////
      typedef Cora::LgrNet::SettingsEnumerator server_settings_type;
      virtual void on_setting_changed(
         server_settings_type *settings,
         server_settings_type::client_type::setting_handle &setting);

      ////////////////////////////////////////////////////////////
      // on_failure (server settings)
      ////////////////////////////////////////////////////////////
      virtual void on_failure(
         server_settings_type *settings, server_settings_type::client_type::failure_type failure);
      
      ////////////////////////////////////////////////////////////
      // on_device_added
      ///////////////////////////////////////////////////////////
      typedef Cora::LgrNet::DeviceMapper mapper_type;
      virtual void on_device_added(
         mapper_type *mapper, device_handle &device);
      
      ////////////////////////////////////////////////////////////
      // on_device_renamed
      ////////////////////////////////////////////////////////////
      virtual void on_device_renamed(
         mapper_type *mapper, device_handle &device, StrUni const &old_name);
      
      ////////////////////////////////////////////////////////////
      // on_device_parent_change
      ////////////////////////////////////////////////////////////
      virtual void on_device_parent_change(
         mapper_type *mapper, device_handle &device, uint4 old_parent_id);
      
      ////////////////////////////////////////////////////////////
      // on_failure
      ////////////////////////////////////////////////////////////
      virtual void on_failure(
         mapper_type *mapper, mapper_type::client_type::failure_type failure);
      
      ////////////////////////////////////////////////////////////
      // on_failure (device settings)
      ////////////////////////////////////////////////////////////
      typedef Cora::Device::SettingsEnumerator device_settings_type;
      virtual void on_failure(
         device_settings_type *lister, device_settings_type::client_type::failure_type failure);
      
      ////////////////////////////////////////////////////////////
      // on_setting_changed
      ////////////////////////////////////////////////////////////
      virtual void on_setting_changed(
         device_settings_type *lister, Csi::SharedPtr<Cora::Setting> &setting);
      
   private:
      ////////////////////////////////////////////////////////////
      // server_settings
      ////////////////////////////////////////////////////////////
      Csi::SharedPtr<server_settings_type> server_settings;
      
      ////////////////////////////////////////////////////////////
      // mapper
      ////////////////////////////////////////////////////////////
      Csi::SharedPtr<mapper_type> mapper;
      
      ////////////////////////////////////////////////////////////
      // device_listers
      ////////////////////////////////////////////////////////////
      typedef Csi::SharedPtr<device_settings_type> device_lister_handle;
      typedef std::list<device_lister_handle> device_listers_type;
      device_listers_type device_listers;
   };
   
   
   
   Application::Application(StrAsc const &address, uint2 port)
   {
      Csi::SharedPtr<Csi::Messaging::Router> router(
         new Csi::Messaging::Router(
            new Csi::SocketConnection(address.c_str(), port)));
      server_settings.bind(new server_settings_type);
      server_settings->start(this, router);
   } // constructor
   
   
   Application::~Application()
   {
      server_settings.clear();
      mapper.clear();
      device_listers.clear();
   } // destructor
   
   
   void Application::on_setting_changed(
      server_settings_type *settings,
      server_settings_type::client_type::setting_handle &setting)
   {
      std::cout << "LgrNet Setting Changed: "
                << Cora::LgrNet::Settings::setting_id_to_str(setting->get_identifier())
                << " (" << setting->get_identifier() << ")\n"
                << "{\n";
      setting->format(std::cout);
      std::cout << "\n}" << std::endl;
      if(mapper == 0)
      {
         mapper.bind(new mapper_type);
         mapper->start(this, server_settings.get_rep());
      }
   } // on_setting_changed


   void Application::on_failure(
      server_settings_type *settings,
      server_settings_type::client_type::failure_type failure)
   {
      throw Csi::MsgExcept("server settings enumerator failed");
   } // on_failure
   
   
   void Application::on_device_added(
      mapper_type *mapper, device_handle &device)
   {
      device_lister_handle lister(new device_settings_type);
      lister->set_device_name(device->get_name());
      lister->start(this, mapper);
      device_listers.push_back(lister);
   } // on_device_added


   struct device_has_name
   {
      StrUni const &device_name;
      device_has_name(StrUni const &device_name_):
         device_name(device_name_)
      { }

      bool operator ()(Csi::SharedPtr<Cora::Device::SettingsEnumerator> &device)
      {
         return device->get_device_name() == device_name;
      }
   };
   
   
   void Application::on_device_renamed(
      mapper_type *mapper, device_handle &device, StrUni const &old_name)
   {
      // delete the old device lister
      device_listers_type::iterator di = std::find_if(
         device_listers.begin(), device_listers.end(), device_has_name(old_name));
      if(di != device_listers.end())
         device_listers.erase(di);

      // make a new device lister
      device_lister_handle lister(new device_settings_type);
      lister->set_device_name(device->get_name());
      lister->start(this, mapper); 
      device_listers.push_back(lister); 
   } // on_device_renamed
   
   
   void Application::on_device_parent_change(
      mapper_type *mapper, device_handle &device, uint4 old_parent_id)
   {
      // delete the old device lister
      device_listers_type::iterator di = std::find_if(
         device_listers.begin(), device_listers.end(), device_has_name(device->get_name()));
      if(di != device_listers.end())
         device_listers.erase(di);

      // make a new device lister
      device_lister_handle lister(new device_settings_type);
      lister->set_device_name(device->get_name());
      lister->start(this, mapper);
      device_listers.push_back(lister);
   } // on_device_parent_change
   
   
   void Application::on_failure(
      mapper_type *mapper, mapper_type::client_type::failure_type failure)
   {
      throw Csi::MsgExcept("server connection failed");
   } // on_failure
   
   
   void Application::on_failure(
      device_settings_type *lister, device_settings_type::client_type::failure_type failure)
   {
      device_listers_type::iterator di = std::find_if(
         device_listers.begin(), device_listers.end(), device_has_name(lister->get_device_name()));
      if(di != device_listers.end())
         device_listers.erase(di);
   } // on_failure
   
   
   void Application::on_setting_changed(
      device_settings_type *lister, Csi::SharedPtr<Cora::Setting> &setting)
   {
      std::cout << "Device setting changed: " << lister->get_device_name() << " -- "
                << Cora::Device::Settings::setting_id_to_str(setting->get_identifier())
                << " (" << setting->get_identifier() << ")\n{\n";
      setting->format(std::cout);
      std::cout << "\n}" << std::endl;
   } // on_setting_changed
};

   
int main(int argc, char const *argv[])
{
   int rtn = 0;
   try
   {
      // initialise the coratools components
      Csi::MessageWindow::initialise(::GetModuleHandle(0));
      Csi::Event::set_dispatcher(new Csi::Win32Dispatch);
      Csi::Win32::WinsockInitialisor sockets_init;
      
      // we will now drive the message loop forever.
      Application app("localhost", 6789);
      MSG message;
      while(GetMessage(&message, 0, 0, 0))
      {
         TranslateMessage(&message);
         DispatchMessage(&message);
      }
   }
   catch(std::exception &e)
   {
      std::cout << "unhandled exception: " << e.what() << std::endl;
      rtn = 1;
   }
   return rtn;
} // main



