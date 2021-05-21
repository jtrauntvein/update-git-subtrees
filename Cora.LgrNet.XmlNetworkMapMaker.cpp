/* Cora.LgrNet.XmlNetworkMapMaker.cpp

   Copyright (C) 2007, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 13 June 2007
   Last Change: Monday 09 November 2009
   Last Commit: $Date: 2009-11-09 11:46:19 -0600 (Mon, 09 Nov 2009) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.XmlNetworkMapMaker.h"
#include "Cora.LgrNet.Defs.h"
#include "Csi.Html.List.h"
#include "Csi.Html.Empty.h"
#include "Csi.Html.Text.h"
#include "Csi.PolySharedPtr.h"
#include "Csi.Html.Table.h"
#include "Csi.Html.DefinitionList.h"
#include "Csi.StringLoader.h"


namespace Cora 
{
   namespace LgrNet
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // maker
            ////////////////////////////////////////////////////////////
            typedef XmlNetworkMapMaker maker_type;
            maker_type *maker;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef maker_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            typedef client_type::xml_network_map xml_network_map;
            xml_network_map xml_map;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               maker_type *maker,
               client_type *client,
               outcome_type outcome,
               xml_network_map xml_map = 0)
            {
               event_complete *event = new event_complete(
                  maker, client, outcome, xml_map);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               maker_type *maker_,
               client_type *client_,
               outcome_type outcome_,
               xml_network_map &xml_map_):
               Event(event_id, maker_),
               maker(maker_),
               client(client_),
               outcome(outcome_),
               xml_map(xml_map_)
            { }
         };


         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::LgrNet::XmlNetworkMapMaker::event_complete");


         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         enum my_string_ids
         {
            strid_setting_id = 1,
            strid_setting_value
         };
         Csi::LocalStringLoader::init_type my_string_initialisors[] =
         {
            { strid_setting_id, "Setting ID" },
            { strid_setting_value, "Value" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            "coratools",
            "1.0",
            "Cora::LgrNet::XmlNetworkMapMaker",
            my_string_initialisors);
      };
      
      
      ////////////////////////////////////////////////////////////
      // class XmlNetworkMapMaker definitions
      ////////////////////////////////////////////////////////////
      StrUni const XmlNetworkMapMaker::netmap_name(L"network-map");
      StrUni const XmlNetworkMapMaker::netmap_device_name(L"device");
      StrUni const XmlNetworkMapMaker::netmap_device_name_name(L"name");
      StrUni const XmlNetworkMapMaker::netmap_device_type_name(L"type");
      StrUni const XmlNetworkMapMaker::netmap_device_indent_name(L"indent");
      StrUni const XmlNetworkMapMaker::netmap_device_id_name(L"id");
      StrUni const XmlNetworkMapMaker::netmap_device_settings_name(L"settings");
      StrUni const XmlNetworkMapMaker::netmap_device_setting_name(L"setting");
      StrUni const XmlNetworkMapMaker::netmap_device_setting_id_name(L"id");
      StrUni const XmlNetworkMapMaker::netmap_lgrnet_settings_name(L"settings");
      StrUni const XmlNetworkMapMaker::netmap_lgrnet_setting_name(L"setting");
      StrUni const XmlNetworkMapMaker::netmap_lgrnet_setting_id_name(L"id");
      StrUni const XmlNetworkMapMaker::netmap_modem_types_name(L"custom-modem-types");
      StrUni const XmlNetworkMapMaker::netmap_modem_type_name(L"custom-modem");
      StrUni const XmlNetworkMapMaker::netmap_modem_init_name(L"init");
      StrUni const XmlNetworkMapMaker::netmap_modem_reset_name(L"reset");
      StrUni const XmlNetworkMapMaker::netmap_modem_type_name_name(L"name");
      StrUni const XmlNetworkMapMaker::netmap_device_collect_areas_name(L"collect-areas");
      StrUni const XmlNetworkMapMaker::netmap_device_collect_area_name(L"collect-area");
      StrUni const XmlNetworkMapMaker::netmap_device_collect_area_name_name(L"name");
      StrUni const XmlNetworkMapMaker::netmap_device_collect_area_persistence_name(L"persistence");
      
      
      XmlNetworkMapMaker::XmlNetworkMapMaker():
         client(0),
         state(state_standby),
         dump_collect_areas(false)
      { }


      XmlNetworkMapMaker::~XmlNetworkMapMaker()
      { finish(); }


      void XmlNetworkMapMaker::set_dump_collect_areas(bool val)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         dump_collect_areas = val;
      } // set_dump_collect_areas
      

      void XmlNetworkMapMaker::start(
         client_type *client_,
         router_handle &router)
      {
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("invalid client pointer");
         if(state != state_standby)
            throw exc_invalid_state();
         client = client_;
         state = state_delegate;
         ClientBase::start(router);
      } // start


      void XmlNetworkMapMaker::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("invalid client pointer");
         if(state != state_standby)
            throw exc_invalid_state();
         client = client_;
         state = state_delegate;
         ClientBase::start(other_component);
      } // start


      void XmlNetworkMapMaker::finish()
      {
         mapper.clear();
         client = 0;
         settings_queue.clear();
         device_settings_getter.clear();
         state = state_standby;
      } // finish


      void XmlNetworkMapMaker::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(state != state_standby)
            {
               finish();
               if(client_type::is_valid_instance(event->client))
                  event->client->on_complete(event->maker, event->outcome, event->xml_map);
            }
         }
      } // receive


      void XmlNetworkMapMaker::describe_network_map(
         Csi::Html::Tag &tag,
         Csi::Xml::Element &elem)
      {
         using namespace Csi;
         Html::Tag::value_type device_list;
         for(Xml::Element::iterator ei = elem.begin(); ei != elem.end(); ++ei)
         {
            Xml::Element::value_type &child = *ei;
            if(child->get_name() == netmap_device_name)
            {
               // make sure the device list is created
               if(device_list == 0)
                  device_list = tag.add_tag(new Html::List);
               
               // add a list item to hold the device name
               Html::Tag::value_type device_tag(device_list->add_tag(new Html::Empty));
               Csi::OStrAscStream device_name;

               device_name << child->get_attr_wstr(netmap_device_name_name) << " ("
                           << DeviceTypes::device_type_to_str(child->get_attr_uint4(netmap_device_type_name))
                           << ")";
               device_tag->add_tag(
                  new Html::Text(
                     device_name.str(), "b"));
               device_tag->add_tag(new Html::Text("<br>"));
               
               try
               {
                  // we need to format the settings for this device
                  Xml::Element::value_type settings_xml(
                     child->find_elem(netmap_device_settings_name));
                  PolySharedPtr<Html::Tag, Html::Table> settings_table(new Html::Table);
                  
                  device_tag->add_tag(settings_table.get_handle());
                  settings_table->add_heading(
                     new Html::Text(my_strings[strid_setting_id],"b"));
                  settings_table->add_heading(
                     new Html::Text(my_strings[strid_setting_value], "b"));
                  settings_table->add_attribute("border","1");
                  for(Xml::Element::iterator esi = settings_xml->begin();
                      esi != settings_xml->end();
                      ++esi)
                  {
                     Xml::Element::value_type &setting_xml = *esi;
                     settings_table->add_row();
                     settings_table->add_tag(
                        new Html::Text(
                           Device::Settings::setting_id_to_str(
                              setting_xml->get_attr_uint4(netmap_device_setting_id_name))));
                     settings_table->add_tag(
                        new Html::Text(
                           setting_xml->get_cdata_str(), "pre", false));
                  }

                  // if the device has any collect areas, we will need to describe these as well.
                  Xml::Element::iterator areas_it(child->find(netmap_device_collect_areas_name));
                  if(areas_it != child->end() && !(*areas_it)->empty())
                  {
                     PolySharedPtr<Html::Tag, Html::DefinitionList> areas_html(new Html::DefinitionList);
                     Xml::Element::value_type &areas_xml(*areas_it);

                     device_tag->add_tag(areas_html.get_handle());
                     for(Xml::Element::iterator cai = areas_xml->begin(); cai != areas_xml->end(); ++cai)
                     {
                        // we will create a table to hold the collect area settings and add this to
                        // the definitions list
                        Xml::Element::value_type &area_xml(*cai);
                        Xml::Element::value_type area_settings_xml(area_xml->find_elem(netmap_device_settings_name));
                        PolySharedPtr<Html::Tag, Html::Table> area_settings_table(new Html::Table);

                        area_settings_table->add_heading(
                           new Html::Text(my_strings[strid_setting_id], "b"));
                        area_settings_table->add_heading(
                           new Html::Text(my_strings[strid_setting_value], "b"));
                        area_settings_table->add_attribute("border", "1");
                        areas_html->add_definition(
                           new Html::Text(area_xml->get_attr_str(netmap_device_collect_area_name_name), "b"),
                           area_settings_table.get_handle());

                        // we can now add all of the settings for this collect area to the table
                        for(Xml::Element::iterator si = area_settings_xml->begin();
                            si != area_settings_xml->end();
                            ++si)
                        {
                           Xml::Element::value_type &setting_xml(*si);
                           area_settings_table->add_row();
                           area_settings_table->add_tag(
                              new Html::Text(
                                 Device::CollectArea::Settings::setting_id_to_str(
                                    setting_xml->get_attr_uint4(netmap_device_setting_id_name))));
                           area_settings_table->add_tag(
                              new Html::Text(setting_xml->get_cdata_str(), "pre", false));
                        }
                     }
                  }
                  
                  // we will add a horizontal rule to separate this device description from the next.
                  device_tag->add_tag(new Html::Tag("hr"));
               }
               catch(std::exception &)
               { }
               
               // we will recursively process any child devices to this device
               describe_network_map(*device_tag, *child);
            }
         }
      } // describe_network_map


      void XmlNetworkMapMaker::describe_failure(
         std::ostream &out,
         client_type::outcome_type failure)
      {
         switch(failure)
         {
         case client_type::outcome_invalid_logon:
            ClientBase::describe_failure(out,corabase_failure_logon);
            break;
            
         case client_type::outcome_session_broken:
            ClientBase::describe_failure(out,corabase_failure_session);
            break;
            
         case client_type::outcome_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_server_security_blocked:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;

         default:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // describe_failure


      void XmlNetworkMapMaker::on_complete(
         ModemTypesLister *lister, ModemTypesLister::client_type::outcome_type outcome)
      {
         // we need to describe the custom modem types in the XML map
         using namespace Csi::Xml;
         if(xml_map == 0)
            xml_map.bind(new Csi::Xml::Element(netmap_name));
         Element::value_type custom_modems(xml_map->add_element(netmap_modem_types_name));
         for(ModemTypesLister::const_iterator mi = lister->begin();
             mi != lister->end();
             ++mi)
         {
            if(mi->custom)
            {
               Element::value_type modem(custom_modems->add_element(netmap_modem_type_name));
               modem->set_attr_wstr(mi->type_name, netmap_modem_type_name_name);
               modem->set_attr_str(mi->reset, netmap_modem_reset_name);
               modem->set_attr_str(mi->init, netmap_modem_init_name);
            }
         }
         
         // we can now get the network map from the server
         state = state_get_network_settings;
         network_settings_getter.bind(new LgrNet::SettingsEnumerator);
         network_settings_getter->start(this, this); 
      } // on_complete

      
      void XmlNetworkMapMaker::on_failure(
         NetworkMapper *mapper,
         NetworkMapperClient::failure_type failure)
      {
         client_type::outcome_type outcome = client_type::outcome_unknown;
         switch(failure)
         {
         case NetworkMapperClient::failure_invalid_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case NetworkMapperClient::failure_session_broken:
            outcome = client_type::outcome_session_broken;
            break;
            
         case NetworkMapperClient::failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case NetworkMapperClient::failure_server_security:
            outcome = client_type::outcome_server_security_blocked;
            break;
         }
         event_complete::cpost(this,client,outcome);
      } // on_failure


      void XmlNetworkMapMaker::on_notify(
         NetworkMapper *mapper,
         uint4 network_map_version,
         uint4 agent_transaction_id,
         bool first_notification,
         uint4 device_count)
      {
         if(state == state_get_network_map)
         {
            if(xml_map == 0)
               xml_map.bind(new Csi::Xml::Element(netmap_name));
            if(device_count == 0)
               event_complete::cpost(
                  this, client, client_type::outcome_success, xml_map);
         }
      } // on_notify

      
      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate is_netmap_parent
         ////////////////////////////////////////////////////////////
         struct is_netmap_parent
         {
            uint4 indent;
            is_netmap_parent(uint4 indent_): indent(indent_) { }
            bool operator ()(Csi::Xml::Element::value_type &elem)
            {
               bool rtn = false;
               if(elem->get_name() == XmlNetworkMapMaker::netmap_device_name &&
                  elem->get_attr_uint4(XmlNetworkMapMaker::netmap_device_indent_name) < indent)
                  rtn = true;
               return rtn;
            }
         };
      };
      
      
      void XmlNetworkMapMaker::on_device(
         NetworkMapper *mapper,
         DevTypeCode device_type,
         uint4 device_object_id,
         StrUni const &name,
         uint4 level,
         bool last_device)
      {
         // make sure that the map is created and that the state is correct
         if(state != state_get_network_map && xml_map != 0)
            return;

         // we need to find the parent device (if any).  This parent will be one of the last devices
         // created that has an indentation level less than  this one.  If this device's indentation
         // level is zero, this implies that this device is a root level device.
         Csi::Xml::Element::value_type parent;
         settings_queue_type::reverse_iterator pi = std::find_if(
            settings_queue.rbegin(), settings_queue.rend(), is_netmap_parent(level));
         if(pi != settings_queue.rend())
            parent = *pi;

         // we can now create the XML object for this device
         Csi::Xml::Element::value_type device_xml(new Csi::Xml::Element(netmap_device_name));

         if(parent != 0)
            parent->add_element(device_xml);
         else
            xml_map->add_element(device_xml);
         device_xml->set_attr_wstr(name, netmap_device_name_name);
         device_xml->set_attr_uint4(device_type, netmap_device_type_name);
         device_xml->set_attr_uint4(level, netmap_device_indent_name);
         device_xml->set_attr_uint4(device_object_id, netmap_device_id_name);
         settings_queue.push_back(device_xml);

         // if this is the last device in the notification, we can now start getting device settings
         // for the devices.
         if(last_device)
            start_next_device_settings();
      } // on_device


      void XmlNetworkMapMaker::start_next_device_settings()
      {
         if(!settings_queue.empty())
         {
            settings_queue_type::value_type &device_xml = settings_queue.front();
            device_settings_getter.bind(new Cora::Device::SettingsEnumerator);
            device_settings_getter->set_device_name(device_xml->get_attr_wstr(netmap_device_name_name));
            device_settings_xml = device_xml->add_element(netmap_device_settings_name);
            state = state_get_device_settings;
            device_settings_getter->start(this,mapper.get_rep());
         }
         else
         {
            state = state_complete;
            event_complete::cpost(this,client,client_type::outcome_success,xml_map);
         }
      } // start_next_device_settings


      void XmlNetworkMapMaker::start_next_collect_area_settings()
      {
         if(!area_settings_queue.empty() && !settings_queue.empty())
         {
            settings_queue_type::value_type &area_xml(area_settings_queue.front());
            settings_queue_type::value_type &device_xml(settings_queue.front());
            area_settings.bind(new area_settings_type);
            area_settings->set_device_name(device_xml->get_attr_wstr(netmap_device_name_name));
            area_settings->set_collect_area_name(area_xml->get_attr_wstr(netmap_device_name_name));
            area_settings_xml = area_xml->add_element(netmap_device_settings_name);
            state = state_get_collect_area_settings;
            area_settings->start(this, mapper.get_rep());
         }
         else
         {
            if(!settings_queue.empty())
               settings_queue.pop_front();
            start_next_device_settings();
         }
      } // start_next_collect_area_settings


      void XmlNetworkMapMaker::on_started(
         Device::SettingsEnumerator *getter)
      {
         if(state == state_get_device_settings)
         {
            settings_queue_type::value_type &device_xml(settings_queue.front());
            uint4 device_type(device_xml->get_attr_uint4(netmap_device_type_name));
            if(dump_collect_areas && DeviceTypes::is_logger_type(device_type))
            {
               areas_xml = device_xml->add_element(netmap_device_collect_areas_name);
               areas_lister.bind(new areas_lister_type);
               areas_lister->set_device_name(device_xml->get_attr_wstr(netmap_device_name_name));
               state = state_get_collect_areas;
               areas_lister->start(this, mapper.get_rep());
            }
            else
            {
               settings_queue.pop_front();
               start_next_device_settings();
            }
         }
      } // on_started


      void XmlNetworkMapMaker::on_failure(
         LgrNet::SettingsEnumerator *getter,
         LgrNet::SettingsEnumeratorClient::failure_type failure)
      {
         event_complete::cpost(this, client, client_type::outcome_unknown);
      } // on_failure


      void XmlNetworkMapMaker::on_setting_changed(
         LgrNet::SettingsEnumerator *getter,
         setting_handle &setting)
      {
         // we need to ensure that the XML structure is allocated
         using namespace Csi::Xml;
         if(xml_map == 0)
            xml_map.bind(new Element(netmap_name));
         if(lgrnet_settings_xml == 0)
            lgrnet_settings_xml = xml_map->find_elem(netmap_lgrnet_settings_name, 0, true);

         // append a new setting structure
         Element::value_type setting_xml(lgrnet_settings_xml->add_element(netmap_lgrnet_setting_name));
         setting_xml->set_attr_uint4(setting->get_identifier(), netmap_lgrnet_setting_id_name);
         setting_val.str("");
         setting->format(setting_val);
         setting_xml->set_cdata_str(setting_val.str());
      } // on_setting_changed


      void XmlNetworkMapMaker::on_started(
         LgrNet::SettingsEnumerator *enumerator)
      {
         if(state == state_get_network_settings)
         {
            state = state_get_network_map;
            mapper.bind(new NetworkMapper);
            mapper->start(this,this);
         }
      } // on_started

      
      void XmlNetworkMapMaker::on_failure(
         Device::SettingsEnumerator *getter,
         Device::SettingsEnumeratorClient::failure_type)
      {
         event_complete::cpost(this, client, client_type::outcome_unknown);
      } // on_failure

      
      void XmlNetworkMapMaker::on_setting_changed(
         Device::SettingsEnumerator *getter,
         Csi::SharedPtr<Setting> &setting)
      {
         if(state == state_get_device_settings)
         {
            Csi::Xml::Element::value_type setting_xml(
               device_settings_xml->add_element(netmap_device_setting_name));
            
            setting_xml->set_attr_uint4(setting->get_identifier(),netmap_device_setting_id_name);
            setting_val.str("");
            setting->format(setting_val);
            setting_xml->set_cdata_str(setting_val.str());
         }
      } // on_setting_changed


      void XmlNetworkMapMaker::on_area_added(
         areas_lister_type *lister,
         StrUni const &area_name,
         persistence_type persistence,
         type_id_type type_id)
      {
         if(persistence == Device::CollectArea::Persistence::logger_feature ||
            persistence == Device::CollectArea::Persistence::table_def_feature)
         {
            Csi::Xml::Element::value_type area_xml(areas_xml->add_element(netmap_device_collect_area_name));
            area_xml->set_attr_wstr(area_name, netmap_device_collect_area_name_name);
            area_xml->set_attr_uint4(persistence, netmap_device_collect_area_persistence_name);
            area_settings_queue.push_back(area_xml);
         }
      } // on_area_added


      void XmlNetworkMapMaker::on_started(areas_lister_type *lister)
      {
         start_next_collect_area_settings();
      } // on_started


      void XmlNetworkMapMaker::on_failure(
         areas_lister_type *lister,
         areas_lister_type::client_type::failure_type failure)
      {
         event_complete::cpost(this, client, client_type::outcome_unknown);
      } // on_failure


      void XmlNetworkMapMaker::on_setting_changed(
         area_settings_type *lister, Csi::SharedPtr<Setting> &setting)
      {
         if(state == state_get_collect_area_settings)
         {
            Csi::Xml::Element::value_type setting_xml(
               area_settings_xml->add_element(netmap_device_setting_name));
            setting_xml->set_attr_uint4(setting->get_identifier(), netmap_device_setting_id_name);
            setting_val.str("");
            setting->format(setting_val);
            setting_xml->set_cdata_str(setting_val.str());
         }
      } // on_setting_changed


      void XmlNetworkMapMaker::on_started(area_settings_type *lister)
      {
         if(state == state_get_collect_area_settings)
         {
            area_settings_queue.pop_front();
            start_next_collect_area_settings();
         }
      } // on_started


      void XmlNetworkMapMaker::on_failure(
         area_settings_type *lister, area_settings_type::client_type::failure_type failure)
      {
         event_complete::cpost(this, client, client_type::outcome_unknown);
      } // on_failure

      
      void XmlNetworkMapMaker::on_corabase_ready()
      {
         state = state_get_modem_types;
         modems_lister.bind(new ModemTypesLister);
         modems_lister->start(this, this);
      } // on_corabase_ready

      
      void XmlNetworkMapMaker::on_corabase_failure(
         corabase_failure_type failure)
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
         event_complete::cpost(this,client,outcome);
      } // on_corabase_failure

      
      void XmlNetworkMapMaker::on_corabase_session_failure()
      { on_corabase_failure(ClientBase::corabase_failure_session); }
   };
};

