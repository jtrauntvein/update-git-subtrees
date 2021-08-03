/* Cora.LgrNet.XmlNetworkMapChanger.cpp

   Copyright (C) 2007, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 19 June 2007
   Last Change: Monday 27 January 2020
   Last Commit: $Date: 2020-01-27 11:26:11 -0600 (Mon, 27 Jan 2020) $
   Last Changed by: $Author: jon $

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.XmlNetworkMapChanger.h"
#include "Cora.Device.DeviceSettingFactory.h"
#include "Cora.LgrNet.XmlNetworkMapMaker.h"
#include "Cora.LgrNet.LgrNetSettingFactory.h"
#include "Cora.LgrNet.Defs.h"
#include "Cora.Device.Defs.h"
#include "Csi.MsgExcept.h"
#include "Csi.StrAscStream.h"
#include "Csi.Html.List.h"
#include "Csi.Html.Text.h"
#include "coratools.strings.h"
#include "boost/format.hpp"


namespace Cora
{
   namespace LgrNet
   {
      namespace
      {
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef XmlNetworkMapChanger changer_type;
            changer_type *changer;
            typedef changer_type::client_type client_type;
            client_type *client;
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;
            typedef changer_type::changes_type changes_type;
            changes_type changes;

            static void cpost(
               changer_type *changer,
               client_type *client,
               outcome_type outcome,
               changes_type &changes)
            {
               event_complete *event = new event_complete(
                  changer, client, outcome, changes);
               event->post();
            }

         private:
            event_complete(
               changer_type *changer_,
               client_type *client_,
               outcome_type outcome_,
               changes_type &changes_):
               Event(event_id, changer_),
               client(client_),
               changer(changer_),
               outcome(outcome_),
               changes(changes_)
            { }
         };


         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::LgrNet::XmlNetworkMapChanger::event_complete");


         class event_change_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef XmlNetworkMapChanger changer_type;
            changer_type *changer;
            typedef changer_type::client_type client_type;
            client_type *client;
            typedef Csi::Xml::Element::value_type change_type;
            change_type change;

            static void cpost(
               changer_type *changer,
               client_type *client,
               change_type &change)
            {
               event_change_complete *event = new event_change_complete(changer, client, change);
               event->post();
            }

         private:
            event_change_complete(
               changer_type *changer_,
               client_type *client_,
               change_type &change_):
               Event(event_id, changer_),
               changer(changer_),
               client(client_),
               change(change_)
            { }
         };


         uint4 const event_change_complete::event_id =
            Csi::Event::registerType("Cora::LgrNet::XmlNetworkMapChanger::event_change_complete");
         

         StrAsc anchor_code_to_string(uint4 anchor_code)
         {
            StrAsc rtn(anchor_type_to_string(anchor_code));
            return rtn;
         }
      };

      
      StrUni const XmlNetworkMapChanger::changes_name(L"network-map-changes");
      StrUni const XmlNetworkMapChanger::add_device_name(L"add-device");
      StrUni const XmlNetworkMapChanger::move_branch_name(L"move-branch");
      StrUni const XmlNetworkMapChanger::set_device_settings_name(L"set-device-settings");
      StrUni const XmlNetworkMapChanger::device_name_name(L"device-name");
      StrUni const XmlNetworkMapChanger::device_type_name(L"device-type");
      StrUni const XmlNetworkMapChanger::anchor_name(L"anchor");
      StrUni const XmlNetworkMapChanger::anchor_type_name(L"anchor-type");
      StrUni const XmlNetworkMapChanger::change_complete_name(L"complete");
      StrUni const XmlNetworkMapChanger::setting_id_name(L"id");
      StrUni const XmlNetworkMapChanger::setting_name(L"setting");
      StrUni const XmlNetworkMapChanger::set_lgrnet_settings_name(L"set-lgrnet-settings");
      StrUni const XmlNetworkMapChanger::add_modem_name(L"add-modem-type");
      StrUni const XmlNetworkMapChanger::modem_name_name(L"name");
      StrUni const XmlNetworkMapChanger::modem_reset_name(L"reset");
      StrUni const XmlNetworkMapChanger::modem_init_name(L"init");
      StrUni const XmlNetworkMapChanger::change_modem_name(L"change-modem-type");
      

      namespace
      {
         struct device_has_name
         {
            StrUni const &name;
            device_has_name(StrUni const &name_): name(name_) { }
            bool operator ()(Csi::Xml::Element::value_type &elem)
            {
               return
                  elem->get_name() == XmlNetworkMapMaker::netmap_device_name &&
                  elem->get_attr_wstr(XmlNetworkMapMaker::netmap_device_name_name) == name;
            }
         };


         struct setting_has_id
         {
            uint4 id;
            setting_has_id(uint4 id_): id(id_) { }
            bool operator ()(Csi::Xml::Element::value_type &elem)
            {
               return
                  elem->get_name() == XmlNetworkMapMaker::netmap_device_setting_name &&
                  elem->get_attr_uint4(XmlNetworkMapMaker::netmap_device_setting_id_name) == id;
            }
         };


         struct modem_has_name
         {
            StrUni const &name;
            modem_has_name(StrUni const &name_):
               name(name_)
            { }

            bool operator()(Csi::Xml::Element::value_type &elem)
            {
               return
                  elem->get_name() == XmlNetworkMapMaker::netmap_modem_type_name &&
                  elem->get_attr_wstr(XmlNetworkMapMaker::netmap_modem_type_name_name) == name;
            }
         };
      };

      
      XmlNetworkMapChanger::changes_type
      XmlNetworkMapChanger::reconcile_network_maps(
         Csi::Xml::Element &source_map,
         Csi::Xml::Element &target_map)
      {
         // we need to compare the lgrnet settings of the source and the target to determine what
         // changes need to be written
         using namespace Csi::Xml;
         changes_type rtn(new Csi::Xml::Element(changes_name));
         Element::iterator source_settings_it = source_map.find(XmlNetworkMapMaker::netmap_lgrnet_settings_name);
         Element::iterator target_settings_it = target_map.find(XmlNetworkMapMaker::netmap_lgrnet_settings_name);

         if(target_settings_it != target_map.end())
         {
            // the source has specified lgrnet settings to be set.  For each of these, we will need
            // to compare with those in the target and to generate a command to set those that are
            // different.
            Element::value_type change_settings_xml;
            Element::value_type &target_settings = *target_settings_it;

            for(Element::iterator tsi = target_settings->begin(); tsi != target_settings->end(); ++tsi)
            {
               Element::value_type &target_setting = *tsi;
               uint4 target_setting_id = target_setting->get_attr_uint4(
                  XmlNetworkMapMaker::netmap_lgrnet_setting_id_name);
               bool change_this_setting = false;
               
               if(source_settings_it != source_map.end())
               {
                  Element::value_type &source_settings = *source_settings_it;
                  Element::value_type source_setting = source_settings->find_elem_if(
                     setting_has_id(target_setting_id), false);
                  if(source_setting != 0 &&
                     target_setting->get_cdata_wstr() != source_setting->get_cdata_wstr())
                     change_this_setting = true;
               }
               else
                  change_this_setting = true;
               if(change_this_setting)
               {
                  if(change_settings_xml == 0)
                     change_settings_xml = rtn->add_element(set_lgrnet_settings_name);
                  Element::value_type change_setting_cmd(
                     change_settings_xml->add_element(setting_name));
                  change_setting_cmd->set_attr_uint4(target_setting_id, setting_id_name);
                  change_setting_cmd->set_cdata_wstr(target_setting->get_cdata_wstr());
               }
            }
         }

         // we need to compare the custom modems between the two structures.  If there are any
         // modems in the target that are not in the source, we will generate a command to add the
         // modem.  If there are any modems in the source that are different from the same modem in
         // the target, a command will be generated to change the modem
         Element::iterator custom_modems_source_it(source_map.find(XmlNetworkMapMaker::netmap_modem_types_name));
         Element::iterator custom_modems_target_it(target_map.find(XmlNetworkMapMaker::netmap_modem_types_name));
         if(custom_modems_target_it != target_map.end())
         {
            Element::value_type target_modems(*custom_modems_target_it);
            Element::value_type source_modems(*custom_modems_source_it);
            for(Element::iterator tmi = target_modems->begin(); tmi != target_modems->end(); ++tmi)
            {
               Element::value_type &target_modem(*tmi);
               StrUni const modem_name(target_modem->get_attr_wstr(XmlNetworkMapMaker::netmap_modem_type_name_name));
               StrAsc const target_reset(target_modem->get_attr_str(XmlNetworkMapMaker::netmap_modem_reset_name));
               StrAsc const target_init(target_modem->get_attr_str(XmlNetworkMapMaker::netmap_modem_init_name));
               
               Element::value_type source_modem(source_modems->find_elem_if(modem_has_name(modem_name)));
               if(source_modem == 0)
               {
                  Element::value_type add_command(rtn->add_element(add_modem_name));
                  add_command->set_attr_wstr(modem_name, modem_name_name);
                  add_command->set_attr_str(target_reset, modem_reset_name);
                  add_command->set_attr_str(target_init, modem_init_name);
               }
               else
               {
                  StrAsc const source_reset(source_modem->get_attr_str(XmlNetworkMapMaker::netmap_modem_reset_name));
                  StrAsc const source_init(source_modem->get_attr_str(XmlNetworkMapMaker::netmap_modem_init_name));
                  if(source_reset != target_reset || source_init != target_init)
                  {
                     Element::value_type change_command(rtn->add_element(change_modem_name));
                     change_command->set_attr_wstr(modem_name, modem_name_name);
                     change_command->set_attr_str(target_reset, modem_reset_name);
                     change_command->set_attr_str(target_init, modem_init_name);
                  }
               }
            }
         }
         
         // we will use an iterative algorithm to perform a breadth first traversal of the target
         // map.  To seed the iteration, we need to push on all of the device elements of the
         // target_map object.
         std::list<Csi::Xml::Element::value_type> device_queue;

         for(Element::iterator ti = target_map.begin(); ti != target_map.end(); ++ti)
         {
            Element::value_type &child = *ti;
            if(child->get_name() == XmlNetworkMapMaker::netmap_device_name)
               device_queue.push_back(child);
         }

         // now that the queue is seeded, we can iterate until the queue is empty
         while(!device_queue.empty())
         {
            // we need to first add all of the child devices to this device so that they will be
            // considered in a subsequent iteration.
            Element::value_type target_device = device_queue.front();
            StrUni target_parent_name;
            StrUni target_device_name(target_device->get_attr_wstr(XmlNetworkMapMaker::netmap_device_name_name));

            if(target_device->get_parent()->get_name() == XmlNetworkMapMaker::netmap_device_name)
               target_parent_name = target_device->get_parent()->get_attr_wstr(
                  XmlNetworkMapMaker::netmap_device_name_name);
            device_queue.pop_front();
            for(Element::iterator ci = target_device->begin(); ci != target_device->end(); ++ci)
            {
               Element::value_type &child = *ci;
               if(child->get_name() == XmlNetworkMapMaker::netmap_device_name)
                  device_queue.push_back(child);
            }

            // we now need to search for this device in the source map.  Whether it is present
            // depends upon whether we will add or, if needed, move the device.
            Element::value_type source_device(source_map.find_elem_if(device_has_name(target_device_name)));
            if(source_device != 0)
            {
               // we need to make certain that the target device and source device have the same
               // type
               uint4 target_type = target_device->get_attr_uint4(XmlNetworkMapMaker::netmap_device_type_name);
               uint4 source_type = source_device->get_attr_uint4(XmlNetworkMapMaker::netmap_device_type_name);
               
               if(target_type != source_type)
               {
                  Csi::OStrAscStream temp;
                  temp << boost::format("The type of device %1%, %2%, is not the same as the type of %3%, %4%") %
                     target_device->get_attr_wstr(XmlNetworkMapMaker::netmap_device_name_name) %
                     target_type %
                     source_device->get_attr_wstr(XmlNetworkMapMaker::netmap_device_name_name) %
                     source_type;
                  throw Csi::MsgExcept(temp.str().c_str());
               }
               
               // we need to insure that the source device has the same parent.  If it doesn't we
               // need to generate a move instruction.
               if(target_parent_name.length() > 0)
               {
                  StrUni source_parent_name(
                     source_device->get_parent()->get_attr_wstr(XmlNetworkMapMaker::netmap_device_name_name));
                  if(source_parent_name != target_parent_name)
                  {
                     Element::value_type move_command(rtn->add_element(move_branch_name));
                     move_command->set_attr_wstr(target_device_name, device_name_name);
                     move_command->set_attr_wstr(target_parent_name, anchor_name);
                     move_command->set_attr_uint4(BranchMover::anchor_as_child, anchor_type_name); 
                  }
               }
            }
            else
            {
               // we need to generate an instruction to add the the device as a child to the target
               // parent
               Element::value_type add_command(rtn->add_element(add_device_name));
               add_command->set_attr_wstr(
                  target_device->get_attr_wstr(XmlNetworkMapMaker::netmap_device_name_name),
                  device_name_name);
               add_command->set_attr_wstr(
                  target_device->get_attr_wstr(XmlNetworkMapMaker::netmap_device_type_name),
                  device_type_name);
               add_command->set_attr_wstr(target_parent_name, anchor_name);
               add_command->set_attr_uint4(
                  target_parent_name.length() > 0 ? anchor_as_child : anchor_after, anchor_type_name);
            }

            // now that we have added whatever commands are needed to ensure that the source device
            // is created, we need to add a command that will change any settings that are different
            // between the source device.
            Element::value_type target_settings(
               target_device->find_elem(XmlNetworkMapMaker::netmap_device_settings_name));
            Element::value_type source_settings;
            Element::value_type set_settings_cmd(new Element(set_device_settings_name));

            set_settings_cmd->set_attr_wstr(target_device_name, device_name_name);
            if(source_device != 0)
               source_settings = source_device->find_elem(XmlNetworkMapMaker::netmap_device_settings_name);
            for(Element::iterator tsi = target_settings->begin(); tsi != target_settings->end(); ++tsi)
            {
               Element::value_type &target_setting = *tsi;
               uint4 target_setting_id = target_setting->get_attr_uint4(
                  XmlNetworkMapMaker::netmap_device_setting_id_name);
               Element::value_type source_setting;

               if(source_settings != 0)
                  source_setting = source_settings->find_elem_if(
                     setting_has_id(target_setting_id), false);
               if(source_setting == 0 ||
                  target_setting->get_cdata_wstr() != source_setting->get_cdata_wstr())
               {
                  Element::value_type change_setting_cmd(
                     set_settings_cmd->add_element(setting_name));
                  change_setting_cmd->set_attr_uint4(target_setting_id, setting_id_name);
                  change_setting_cmd->set_cdata_wstr(target_setting->get_cdata_wstr());
               }
            }
            if(!set_settings_cmd->empty())
               rtn->add_element(set_settings_cmd);
         }
         return rtn;
      } // reconcile_network_maps


      void XmlNetworkMapChanger::describe_changes(
         Csi::Html::Tag &tag,
         changes_type &changes)
      {
         using namespace XmlNetworkMapChangerStrings;
         using namespace Csi::Xml;
         if(!changes->empty())
         {
            Csi::Html::Tag::value_type changes_list(tag.add_tag(new Csi::Html::List(true)));
            for(Element::iterator ci = changes->begin(); ci != changes->end(); ++ci)
            {
               Element::value_type &change_xml = *ci;
               Csi::OStrAscStream change_desc;

               if(change_xml->get_name() == set_lgrnet_settings_name)
               {
                  change_desc << my_strings[strid_set_lgrnet_settings_description];
                  change_desc << "\n<blockquote><table border=\"1\">\n"
                              << "<tr><th><b>" << my_strings[strid_setting_id] << "</b></th>"
                              << "<th><b>" << my_strings[strid_setting_value] << "</b></th></tr>\n";
                  for(Element::iterator si = change_xml->begin(); si != change_xml->end(); ++si)
                  {
                     Element::value_type &setting = *si;
                     change_desc << "<tr><td>"
                                 << LgrNet::Settings::setting_id_to_str(
                                    setting->get_attr_uint4(setting_id_name))
                                 << "</td>"
                                 << "<td><tt>" << setting->get_cdata_wstr() << "</tt></td></tr>\n";
                  }
                  change_desc << "</table></blockquote>"; 
               }
               else if(change_xml->get_name() == add_device_name)
               {
                  StrUni add_anchor_name(change_xml->get_attr_wstr(anchor_name));
                  change_desc << boost::format(my_strings[strid_add_device_description].c_str()) %
                     change_xml->get_attr_wstr(device_name_name) %
                     DeviceTypes::device_type_to_str(change_xml->get_attr_uint4(device_type_name)) %
                     (add_anchor_name.length() ? anchor_code_to_string(change_xml->get_attr_uint4(anchor_type_name)) : "") %
                     add_anchor_name;
               }
               else if(change_xml->get_name() == move_branch_name)
               {
                  change_desc << boost::format(my_strings[strid_move_branch_description].c_str()) %
                     change_xml->get_attr_wstr(device_name_name) %
                     anchor_code_to_string(change_xml->get_attr_uint4(anchor_type_name)) %
                     change_xml->get_attr_wstr(anchor_name);
               }
               else if(change_xml->get_name() == set_device_settings_name)
               {
                  change_desc << boost::format(my_strings[strid_set_device_settings_description].c_str()) %
                     change_xml->get_attr_wstr(device_name_name);
                  change_desc << "\n<blockquote><table border=\"1\">\n"
                              << "<tr><th><b>" << my_strings[strid_setting_id] << "</b></th>"
                              << "<th><b>" << my_strings[strid_setting_value] << "</b></th></tr>\n";
                  for(Element::iterator si = change_xml->begin(); si != change_xml->end(); ++si)
                  {
                     Element::value_type &setting = *si;
                     change_desc << "<tr><td>"
                                 << Device::Settings::setting_id_to_str(
                                    setting->get_attr_uint4(setting_id_name))
                                 << "</td>"
                                 << "<td><tt>" << setting->get_cdata_wstr() << "</tt></td></tr>\n";
                  }
                  change_desc << "</table></blockquote>"; 
               }
               else if(change_xml->get_name() == add_modem_name)
               {
                  change_desc << boost::format(my_strings[strid_add_modem_description].c_str()) %
                     change_xml->get_attr_str(modem_name_name) %
                     change_xml->get_attr_str(modem_reset_name) %
                     change_xml->get_attr_str(modem_init_name);
               }
               else if(change_xml->get_name() == change_modem_name)
               {
                  change_desc << boost::format(my_strings[strid_change_modem_description].c_str()) %
                     change_xml->get_attr_str(modem_name_name) %
                     change_xml->get_attr_str(modem_reset_name) %
                     change_xml->get_attr_str(modem_init_name);
               }
               changes_list->add_tag(new Csi::Html::Text(change_desc.str()));
            }
         }
      } // describe_changes


      namespace
      {
         struct change_not_ok
         {
            bool operator ()(Csi::Xml::Element::value_type &change)
            {
               return
                  !change->has_attribute(XmlNetworkMapChanger::change_complete_name) ||
                  !change->get_attr_bool(XmlNetworkMapChanger::change_complete_name);
            }
         };
      };

      
      void XmlNetworkMapChanger::describe_failure(
         std::ostream &out,
         client_type::outcome_type outcome,
         Csi::Xml::Element &changes)
      {
         using namespace XmlNetworkMapChangerStrings;
         switch(outcome)
         {
         case client_type::outcome_invalid_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_session_broken:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_server_security_blocked:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;
            
         case client_type::outcome_network_locked:
            out << my_strings[strid_failure_network_locked];
            break;
            
         case client_type::outcome_unrecognised_change: 
         case client_type::outcome_invalid_device_name:
         case client_type::outcome_unattachable:
         case client_type::outcome_unsupported_device_type:
         case client_type::outcome_invalid_setting:
         {
            // we need to find the breaking place in the changes list
            using namespace Csi::Xml;
            Element::value_type failed_change = changes.find_elem_if(change_not_ok());
            if(failed_change != 0)
            {
               switch(outcome)
               {
               case client_type::outcome_unrecognised_change:
                  out << boost::format(my_strings[strid_failure_unrecognised_change].c_str()) %
                     failed_change->get_name();
                  break;
                  
               case client_type::outcome_invalid_device_name:
                  out << boost::format(my_strings[strid_failure_invalid_device_name].c_str()) %
                     failed_change->get_attr_wstr(device_name_name);
                  break;
                  
               case client_type::outcome_unattachable:
                  out << boost::format(my_strings[strid_failure_unattachable].c_str()) %
                     failed_change->get_attr_wstr(device_name_name) %
                     anchor_code_to_string(
                        failed_change->get_attr_uint4(anchor_type_name));
                  break;
                  
               case client_type::outcome_unsupported_device_type:
                  out << boost::format(my_strings[strid_failure_unsupported_device_type].c_str()) %
                     DeviceTypes::device_type_to_str(failed_change->get_attr_uint4(device_type_name)) %
                     failed_change->get_attr_wstr(device_name_name);
                  break;
                  
               case client_type::outcome_invalid_setting:
                  out << boost::format(my_strings[strid_failure_invalid_setting].c_str()) %
                     failed_change->get_attr_wstr(device_name_name);
                  break;
               }
            }
            else
               ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
         }

         case client_type::outcome_invalid_change:
            out << my_strings[strid_invalid_change];
            break;

         case client_type::outcome_invalid_modem_name:
            out << my_strings[strid_failure_invalid_modem_name];
            break;
            
         case client_type::outcome_modem_read_only:
            out << my_strings[strid_failure_modem_read_only];
            break;
            
         case client_type::outcome_invalid_modem_strings:
            out << my_strings[strid_failure_invalid_modem_strings];
            break;

         case client_type::outcome_too_many_stations:
            DeviceAdder::describe_outcome(out, DeviceAdder::client_type::outcome_too_many_stations);
            break;
            
         default:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // describe_failure

      
      void XmlNetworkMapChanger::finish()
      {
         adder.clear();
         mover.clear();
         device_settings_setter.clear();
         lgrnet_settings_setter.clear();
         changes.clear();
         client = 0;
         state = state_standby;
      } // finish


      void XmlNetworkMapChanger::on_complete(
         LgrNet::SettingsSetter *setter,
         LgrNet::SettingsSetterClient::outcome_type outcome_)
      {
         if(outcome_ == LgrNet::SettingsSetterClient::outcome_success ||
            outcome_ == LgrNet::SettingsSetterClient::outcome_some_errors)
         {
            Csi::Xml::Element::value_type &change = *current_change;
            change->set_attr_bool(true, change_complete_name);
            ++current_change;
            do_next_change();
         }
         else
         {
            client_type::outcome_type outcome = client_type::outcome_unknown;
            switch(outcome_)
            {
            case LgrNet::SettingsSetterClient::outcome_invalid_logon:
               outcome = client_type::outcome_invalid_logon;
               break;
               
            case LgrNet::SettingsSetterClient::outcome_some_errors:
               outcome = client_type::outcome_invalid_setting;
               break;
               
            case LgrNet::SettingsSetterClient::outcome_session_failed:
               outcome = client_type::outcome_session_broken;
               break;
               
            case LgrNet::SettingsSetterClient::outcome_security_blocked:
               outcome = client_type::outcome_server_security_blocked;
               break;
               
            case LgrNet::SettingsSetterClient::outcome_unsupported:
               outcome = client_type::outcome_unsupported;
               break;
               
            case LgrNet::SettingsSetterClient::outcome_network_locked:
               outcome = client_type::outcome_network_locked;
               break;
            }
            event_complete::cpost(this, client, outcome, changes);
         }
      } // on_complete (lgrnet settings setter)

      
      void XmlNetworkMapChanger::on_complete(
         DeviceAdder *adder,
         DeviceAdderClient::outcome_type outcome_)
      {
         if(outcome_ == DeviceAdderClient::outcome_success)
         {
            Csi::Xml::Element::value_type &change = *current_change;
            change->set_attr_bool(true, change_complete_name);
            event_change_complete::cpost(this, client, change);
            ++current_change;
            do_next_change();
         }
         else
         {
            client_type::outcome_type outcome = client_type::outcome_unknown;
            switch(outcome_)
            {
            case DeviceAdderClient::outcome_invalid_logon:
               outcome = client_type::outcome_invalid_logon;
               break;
               
            case DeviceAdderClient::outcome_session_broken:
               outcome = client_type::outcome_session_broken;
               break;
               
            case DeviceAdderClient::outcome_unsupported:
               outcome = client_type::outcome_unsupported;
               break;
               
            case DeviceAdderClient::outcome_server_security_blocked:
               outcome = client_type::outcome_server_security_blocked;
               break;
               
            case DeviceAdderClient::outcome_invalid_device_name:
               outcome = client_type::outcome_invalid_device_name;
               break;
               
            case DeviceAdderClient::outcome_unattachable:
               outcome = client_type::outcome_unattachable;
               break;
               
            case DeviceAdderClient::outcome_unsupported_device_type:
               outcome = client_type::outcome_unsupported_device_type;
               break;
               
            case DeviceAdderClient::outcome_network_locked:
               outcome = client_type::outcome_network_locked;
               break;

            case DeviceAdderClient::outcome_too_many_stations:
               outcome = client_type::outcome_too_many_stations;
               break;
            }
            event_complete::cpost(this, client, outcome, changes);
         }
      } // on_complete (adder)


      void XmlNetworkMapChanger::on_complete(
         BranchMover *mover,
         BranchMoverClient::outcome_type outcome_)
      {
         if(outcome_ == BranchMoverClient::outcome_success)
         {
            Csi::Xml::Element::value_type &change = *current_change;
            change->set_attr_bool(true, change_complete_name);
            event_change_complete::cpost(this, client, change);
            ++current_change;
            do_next_change();
         }
         else
         {
            client_type::outcome_type outcome = client_type::outcome_unknown;
            switch(outcome_)
            {
            case BranchMoverClient::outcome_invalid_logon:
               outcome = client_type::outcome_invalid_logon;
               break;
               
            case BranchMoverClient::outcome_session_broken:
               outcome = client_type::outcome_session_broken;
               break;
               
            case BranchMoverClient::outcome_unsupported:
               outcome = client_type::outcome_unsupported;
               break;
               
            case BranchMoverClient::outcome_server_security_blocked:
               outcome = client_type::outcome_server_security_blocked;
               break;
               
            case BranchMoverClient::outcome_specified_device_not_found:
               outcome = client_type::outcome_invalid_device_name;
               break;
               
            case BranchMoverClient::outcome_specified_anchor_not_found:
            case BranchMoverClient::outcome_unattachable:
               outcome = client_type::outcome_unattachable;
               break;
               
            case BranchMoverClient::outcome_network_locked:
               outcome = client_type::outcome_network_locked;
               break;
            }
            event_complete::cpost(this, client, outcome, changes);
         }
      } // on_complete (mover)


      void XmlNetworkMapChanger::on_complete(
         Device::SettingsSetter *setter,
         Device::SettingsSetterClient::outcome_type outcome_)
      {
         if(outcome_ == Device::SettingsSetterClient::outcome_success ||
            outcome_ == Device::SettingsSetterClient::outcome_some_errors)
         {
            Csi::Xml::Element::value_type &change = *current_change;
            change->set_attr_bool(true, change_complete_name);
            event_change_complete::cpost(this, client, change);
            ++current_change;
            do_next_change();
         }
         else
         {
            client_type::outcome_type outcome = client_type::outcome_success;
            switch(outcome_)
            {
            case Device::SettingsSetterClient::outcome_some_errors:
               outcome = client_type::outcome_invalid_setting;
               break;
               
            case Device::SettingsSetterClient::outcome_invalid_logon:
               outcome = client_type::outcome_invalid_logon;
               break;
               
            case Device::SettingsSetterClient::outcome_invalid_device_name:
               outcome = client_type::outcome_invalid_device_name;
               break;
               
            case Device::SettingsSetterClient::outcome_session_failed:
               outcome = client_type::outcome_session_broken;
               break;
               
            case Device::SettingsSetterClient::outcome_security_blocked:
               outcome = client_type::outcome_server_security_blocked;
               break;
               
            case Device::SettingsSetterClient::outcome_unsupported:
               outcome = client_type::outcome_unsupported;
               break;
               
            case Device::SettingsSetterClient::outcome_network_locked:
               outcome = client_type::outcome_network_locked;
               break;
            }
            event_complete::cpost(this, client, outcome, changes);
         }
      } // on_complete (settings setter)


      void XmlNetworkMapChanger::on_complete(
         ModemTypeAdder *adder,
         ModemTypeAdder::client_type::outcome_type outcome_)
      {
         if(outcome_ == ModemTypeAdder::client_type::outcome_success)
         {
            Csi::Xml::Element::value_type &change = *current_change;
            change->set_attr_bool(true, change_complete_name);
            event_change_complete::cpost(this, client, change);
            ++current_change;
            do_next_change();
         } 
         else
         {
            client_type::outcome_type outcome(client_type::outcome_unknown);
            switch(outcome_)
            {
            case ModemTypeAdder::client_type::outcome_failure_invalid_logon:
               outcome = client_type::outcome_invalid_logon;
               break;
               
            case ModemTypeAdder::client_type::outcome_failure_session:
               outcome = client_type::outcome_session_broken;
               break;
               
            case ModemTypeAdder::client_type::outcome_failure_unsupported:
               outcome = client_type::outcome_unsupported;
               break;
               
            case ModemTypeAdder::client_type::outcome_failure_security:
               outcome = client_type::outcome_server_security_blocked;
               break;
               
            case ModemTypeAdder::client_type::outcome_failure_invalid_modem_name:
               outcome = client_type::outcome_invalid_modem_name;
               break;
               
            case ModemTypeAdder::client_type::outcome_failure_network_locked:
               outcome = client_type::outcome_network_locked;
               break;
            }
            event_complete::cpost(this, client, outcome, changes);
         }
      } // on_complete


      void XmlNetworkMapChanger::on_complete(
         ModemTypeChanger *changer,
         ModemTypeChanger::client_type::outcome_type outcome_)
      {
         if(outcome_ == ModemTypeChanger::client_type::outcome_success)
         {
            Csi::Xml::Element::value_type &change = *current_change;
            change->set_attr_bool(true, change_complete_name);
            event_change_complete::cpost(this, client, change);
            ++current_change;
            do_next_change();
         } 
         else
         {
            client_type::outcome_type outcome(client_type::outcome_unknown);
            switch(outcome_)
            {
            case ModemTypeChanger::client_type::outcome_failure_invalid_logon:
               outcome = client_type::outcome_invalid_logon;
               break;
               
            case ModemTypeChanger::client_type::outcome_failure_session:
               outcome = client_type::outcome_session_broken;
               break;
               
            case ModemTypeChanger::client_type::outcome_failure_unsupported:
               outcome = client_type::outcome_unsupported;
               break;
               
            case ModemTypeChanger::client_type::outcome_failure_security:
               outcome = client_type::outcome_server_security_blocked;
               break;
               
            case ModemTypeChanger::client_type::outcome_failure_invalid_modem_name:
               outcome = client_type::outcome_invalid_modem_name;
               break;
               
            case ModemTypeChanger::client_type::outcome_failure_read_only:
               outcome = client_type::outcome_modem_read_only;
               break;
               
            case ModemTypeChanger::client_type::outcome_failure_invalid_strings:
               outcome = client_type::outcome_invalid_modem_strings;
               break;
               
            case ModemTypeChanger::client_type::outcome_failure_network_locked:
               outcome = client_type::outcome_network_locked;
               break;
            }
            event_complete::cpost(this, client, outcome, changes);
         }
      } // on_complete


      void XmlNetworkMapChanger::on_corabase_ready()
      {
         current_change = changes->begin();
         state = state_active;
         do_next_change();
      } // on_corabase_ready


      void XmlNetworkMapChanger::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type outcome = client_type::outcome_unknown;
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case corabase_failure_session:
            outcome = client_type::outcome_session_broken;
            break;
            
         case corabase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case corabase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;
         }
         event_complete::cpost(this,client,outcome,changes);
      } // on_corabase_failure


      void XmlNetworkMapChanger::on_corabase_session_failure()
      { on_corabase_failure(corabase_failure_session); }


      void XmlNetworkMapChanger::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            finish();
            if(client_type::is_valid_instance(event->client))
               event->client->on_complete(this,event->outcome,*(event->changes));
         }
         else if(ev->getType() == event_change_complete::event_id)
         {
            event_change_complete *event = static_cast<event_change_complete *>(ev.get_rep());
            if(state == state_active && client_type::is_valid_instance(event->client))
               event->client->on_change_complete(this, *event->change);
         }
      } // receive


      void XmlNetworkMapChanger::do_next_change()
      {
         if(current_change != changes->end())
         {
            Csi::Xml::Element::value_type &change = *current_change;
            if(change->get_name() == add_device_name)
               do_add_device(change);
            else if(change->get_name() == move_branch_name)
               do_move_branch(change);
            else if(change->get_name() == set_device_settings_name)
               do_set_device_settings(change);
            else if(change->get_name() == set_lgrnet_settings_name)
               do_set_lgrnet_settings(change);
            else if(change->get_name() == add_modem_name)
               do_add_modem(change);
            else if(change->get_name() == change_modem_name)
               do_change_modem(change);
            else
               event_complete::cpost(
                  this, client, client_type::outcome_unrecognised_change, changes);
         }
         else
            event_complete::cpost(
               this, client, client_type::outcome_success, changes);
      } // do_next_change


      void XmlNetworkMapChanger::do_set_lgrnet_settings(
         Csi::Xml::Element::value_type &change)
      {
         using namespace Csi::Xml;
         try
         {
            if(!change->empty())
            {
               Cora::LgrNet::LgrNetSettingFactory factory;
               lgrnet_settings_setter.bind(new LgrNet::SettingsSetter);
               for(Element::iterator si = change->begin(); si != change->end(); ++si)
               {
                  Element::value_type &setting_xml = *si;
                  LgrNet::SettingsSetter::setting_handle setting(
                     factory.make_setting(
                        setting_xml->get_attr_uint4(setting_id_name)));
                  setting->read(setting_xml->get_cdata_str().c_str());
                  lgrnet_settings_setter->add_setting(setting);
               }
               lgrnet_settings_setter->start(this, this);
            }
            else
            {
               change->set_attr_bool(true, change_complete_name);
               ++current_change;
               do_next_change();
            }
         }
         catch(std::exception &)
         { event_complete::cpost(this, client, client_type::outcome_invalid_change, change); }
      } // do_set_lgrnet_settings


      void XmlNetworkMapChanger::do_add_device(
         Csi::Xml::Element::value_type &change)
      {
         try
         {
            adder.bind(new DeviceAdder);
            adder->set_device_name(change->get_attr_wstr(device_name_name));
            adder->set_device_type(
               static_cast<DevTypeCode>(change->get_attr_uint4(device_type_name)));
            adder->set_anchor_name(change->get_attr_wstr(anchor_name));
            adder->set_anchor_code(
               static_cast<anchor_code_type>(change->get_attr_uint4(anchor_type_name)));
            adder->start(this, this);
         }
         catch(std::exception &)
         { event_complete::cpost(this, client, client_type::outcome_invalid_change, changes); }
      } // do_add_device


      void XmlNetworkMapChanger::do_move_branch(
         Csi::Xml::Element::value_type &change)
      {
         try
         {
            mover.bind(new BranchMover);
            mover->set_branch_root_name(change->get_attr_wstr(device_name_name));
            mover->set_anchor_name(change->get_attr_wstr(anchor_name));
            mover->set_anchor_code(
               static_cast<BranchMover::anchor_code_type>(change->get_attr_uint4(anchor_type_name)));
            mover->start(this, this);
         }
         catch(std::exception &)
         { event_complete::cpost(this, client, client_type::outcome_invalid_change, changes); }
      } // do_move_branch


      void XmlNetworkMapChanger::do_set_device_settings(
         Csi::Xml::Element::value_type &change)
      {
         try
         {
            if(!change->empty())
            {
               Cora::Device::DeviceSettingFactory factory;
               
               device_settings_setter.bind(new Device::SettingsSetter);
               device_settings_setter->set_device_name(change->get_attr_wstr(device_name_name));
               for(Csi::Xml::Element::iterator si = change->begin();
                   si != change->end();
                   ++si)
               {
                  Csi::Xml::Element::value_type &setting_xml = *si;
                  Device::SettingsSetter::setting_handle setting(
                     factory.make_setting(setting_xml->get_attr_uint4(setting_id_name)));
                  setting->read(setting_xml->get_cdata_str().c_str());
                  device_settings_setter->add_setting(setting);
               }
               device_settings_setter->start(this, this);
            }
            else
            {
               change->set_attr_bool(true, change_complete_name);
               ++current_change;
               do_next_change();
            }
            
         }
         catch(std::exception &)
         { event_complete::cpost(this, client, client_type::outcome_invalid_change, changes); }
      } // do_set_device_settings


      void XmlNetworkMapChanger::do_add_modem(
         Csi::Xml::Element::value_type &change)
      {
         try
         {
            modem_adder.bind(new ModemTypeAdder);
            modem_adder->set_type_name(change->get_attr_wstr(modem_name_name));
            modem_adder->set_reset(change->get_attr_str(modem_reset_name));
            modem_adder->set_init(change->get_attr_str(modem_init_name));
            modem_adder->start(this, this);
         }
         catch(std::exception &)
         { event_complete::cpost(this, client, client_type::outcome_invalid_change, changes); }
      } // do_add_modem


      void XmlNetworkMapChanger::do_change_modem(
         Csi::Xml::Element::value_type &change)
      {
         try
         {
            modem_changer.bind(new ModemTypeChanger);
            modem_changer->set_type_name(change->get_attr_wstr(modem_name_name));
            modem_changer->set_reset(change->get_attr_str(modem_reset_name));
            modem_changer->set_init(change->get_attr_str(modem_init_name));
            modem_changer->start(this, this);
         }
         catch(std::exception &)
         { event_complete::cpost(this, client, client_type::outcome_invalid_change, changes); }
      } // do_change_modem
   };
};

