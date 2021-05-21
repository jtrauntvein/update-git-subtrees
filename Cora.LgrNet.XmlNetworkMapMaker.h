/* Cora.LgrNet.XmlNetworkMapMaker.h

   Copyright (C) 2007, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 13 June 2007
   Last Change: Saturday 07 November 2009
   Last Commit: $Date: 2009-11-09 11:46:19 -0600 (Mon, 09 Nov 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_LgrNet_XmlNetworkMapMaker_h
#define Cora_LgrNet_XmlNetworkMapMaker_h

#include "Cora.LgrNet.NetworkMapper.h"
#include "Cora.LgrNet.SettingsEnumerator.h"
#include "Cora.LgrNet.ModemTypesLister.h"
#include "Cora.Device.SettingsEnumerator.h"
#include "Cora.Device.CollectAreasEnumerator.h"
#include "Cora.Device.CollectArea.SettingsEnumerator.h"
#include "Csi.Xml.Element.h"
#include "Csi.StrAscStream.h"
#include "Csi.Html.Tag.h"


namespace Cora
{
   namespace LgrNet
   {
      // @group class forward declarations
      class XmlNetworkMapMaker;
      // @endgroup


      ////////////////////////////////////////////////////////////
      // class XmlNetworkMapMakerClient
      //
      // Defines the call-back interface for the XmlNetworkMapMaker component. 
      ////////////////////////////////////////////////////////////
      class XmlNetworkMapMakerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the component has completed its structure or an error
         // has occurred.  The xml_map parameter will specify the resulting XML
         // structure but will only be filled in if the value of outcome is
         // equal to outcome_success.
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_session_broken = 3,
            outcome_unsupported = 4,
            outcome_server_security_blocked = 5
         };
         typedef Csi::Xml::Element::value_type xml_network_map;
         virtual void on_complete(
            XmlNetworkMapMaker *maker,
            outcome_type outcome,
            xml_network_map &xml_map) = 0;
      };

      
      ////////////////////////////////////////////////////////////
      // class XmlNetworkMapMaker
      //
      // Defines a component that can be used to produce an XML netwokr-map
      // structure that describes the structure of the LgrNet server's network
      // map.
      //
      // In order to use this component, an application must provide an object
      // derived from class XmlNetworkMapMakerClient (or the client_type
      // typedef defined in this class).  The application should then create an
      // object of this class and invoke one of the two start() methods.  When
      // the structure is built, the application will receive notification
      // through its overload of the XmlNetworkMapMakerClient::on_complete()
      // method.   This same method will be used to report any errors that
      // might arise during the process. 
      ////////////////////////////////////////////////////////////
      class XmlNetworkMapMaker:
         public ClientBase,
         public Csi::EventReceiver,
         public ModemTypesListerClient,
         public NetworkMapperClient,
         public Device::SettingsEnumeratorClient,
         public LgrNet::SettingsEnumeratorClient,
         public Device::CollectAreasEnumeratorClient,
         public Device::CollectArea::SettingsEnumeratorClient 
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         XmlNetworkMapMaker();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~XmlNetworkMapMaker();

         ////////////////////////////////////////////////////////////
         // get_dump_collect_areas
         ////////////////////////////////////////////////////////////
         bool get_dump_collect_areas() const
         { return dump_collect_areas; }

         ////////////////////////////////////////////////////////////
         // set_dump_collect_areas
         ////////////////////////////////////////////////////////////
         void set_dump_collect_areas(bool val);
         
         ////////////////////////////////////////////////////////////
         // start (new connection)
         ////////////////////////////////////////////////////////////
         typedef XmlNetworkMapMakerClient client_type;
         void start(
            client_type *client_,
            ClientBase::router_handle &router_);

         ////////////////////////////////////////////////////////////
         // start (from running component)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         // @group Name for elements and attributes in the network-map format
         static StrUni const netmap_name;
         static StrUni const netmap_lgrnet_settings_name;
         static StrUni const netmap_lgrnet_setting_name;
         static StrUni const netmap_lgrnet_setting_id_name;
         static StrUni const netmap_modem_types_name;
         static StrUni const netmap_modem_type_name;
         static StrUni const netmap_modem_type_name_name;
         static StrUni const netmap_modem_init_name;
         static StrUni const netmap_modem_reset_name;
         static StrUni const netmap_device_name;
         static StrUni const netmap_device_name_name;
         static StrUni const netmap_device_type_name;
         static StrUni const netmap_device_indent_name;
         static StrUni const netmap_device_id_name;
         static StrUni const netmap_device_settings_name;
         static StrUni const netmap_device_setting_name;
         static StrUni const netmap_device_setting_id_name;
         static StrUni const netmap_device_collect_areas_name;
         static StrUni const netmap_device_collect_area_name;
         static StrUni const netmap_device_collect_area_persistence_name;
         static StrUni const netmap_device_collect_area_name_name;
         // @endgroup

         ////////////////////////////////////////////////////////////
         // describe_network_map
         ////////////////////////////////////////////////////////////
         static void describe_network_map(
            Csi::Html::Tag &tag,
            Csi::Xml::Element &map);

         ////////////////////////////////////////////////////////////
         // predicate device_has_name_and_type
         //
         // Evaluates whether a device in a network-map structure has the same
         // name and type as that specified in the constructor.  This predicate
         // will be applied recursively on all device elements. 
         ////////////////////////////////////////////////////////////
         struct device_has_name_and_type
         {
            StrAsc const &name;
            uint4 type;
            device_has_name_and_type(StrAsc const &name_, uint4 type_):
               name(name_),
               type(type_)
            { }
            bool operator ()(Csi::Xml::Element::value_type &elem)
            {
               bool rtn = 
                  elem->get_name() == netmap_device_name &&
                  elem->get_attr_str(netmap_device_name_name) == name &&
                  elem->get_attr_uint4(netmap_device_type_name) == type;
               return rtn;
            }
         };

         ////////////////////////////////////////////////////////////
         // describe_failure
         ////////////////////////////////////////////////////////////
         static void describe_failure(
            std::ostream &out,
            client_type::outcome_type failure);
         
      protected: 
         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            NetworkMapper *mapper,
            NetworkMapperClient::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_notify
         ////////////////////////////////////////////////////////////
         virtual void on_notify(
            NetworkMapper *mapper,
            uint4 network_map_version,
            uint4 agent_transaction_id,
            bool first_notification,
            uint4 device_count);

         ////////////////////////////////////////////////////////////
         // on_device
         ////////////////////////////////////////////////////////////
         virtual void on_device(
            NetworkMapper *mapper,
            DevTypeCode device_type,
            uint4 device_object_id,
            StrUni const &name,
            uint4 level,
            bool last_device);

         ////////////////////////////////////////////////////////////
         // start_next_device_settings
         ////////////////////////////////////////////////////////////
         void start_next_device_settings();

         ////////////////////////////////////////////////////////////
         // start_next_collect_area_settings
         ////////////////////////////////////////////////////////////
         void start_next_collect_area_settings();

         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            Device::SettingsEnumerator *getter);

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            Device::SettingsEnumerator *getter,
            Device::SettingsEnumeratorClient::failure_type);

         ////////////////////////////////////////////////////////////
         // on_setting_changed
         ////////////////////////////////////////////////////////////
         virtual void on_setting_changed(
            Device::SettingsEnumerator *getter,
            Csi::SharedPtr<Setting> &setting);

         ////////////////////////////////////////////////////////////
         // on_failure(lgrnet settings enumerator)
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            LgrNet::SettingsEnumerator *enumerator,
            LgrNet::SettingsEnumerator::client_type::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_setting_changed
         ////////////////////////////////////////////////////////////
         virtual void on_setting_changed(
            LgrNet::SettingsEnumerator *enumerator,
            setting_handle &setting);

         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            LgrNet::SettingsEnumerator *enumerator);

         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         virtual void on_complete(
            ModemTypesLister *lister, ModemTypesLister::client_type::outcome_type outcome);

         ////////////////////////////////////////////////////////////
         // on_area_added
         ////////////////////////////////////////////////////////////
         typedef Device::CollectAreasEnumerator areas_lister_type;
         virtual void on_area_added(
            areas_lister_type *lister,
            StrUni const &area_name,
            persistence_type persistence,
            type_id_type type_id);

         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(areas_lister_type *lister);

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            areas_lister_type *lister,
            areas_lister_type::client_type::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_setting_changed
         ////////////////////////////////////////////////////////////
         typedef Device::CollectArea::SettingsEnumerator area_settings_type;
         virtual void on_setting_changed(
            area_settings_type *lister,
            Csi::SharedPtr<Setting> &setting);

         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(area_settings_type *lister);

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            area_settings_type *lister,
            area_settings_type::client_type::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure();

      protected:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_get_modem_types,
            state_get_network_settings,
            state_get_network_map,
            state_get_device_settings,
            state_get_collect_areas,
            state_get_collect_area_settings,
            state_complete
         } state;

         ////////////////////////////////////////////////////////////
         // modems_lister
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<ModemTypesLister> modems_lister;

         ////////////////////////////////////////////////////////////
         // network_settings_getter
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<LgrNet::SettingsEnumerator> network_settings_getter;
         
         ////////////////////////////////////////////////////////////
         // mapper
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<NetworkMapper> mapper;

         ////////////////////////////////////////////////////////////
         // xml_map
         ////////////////////////////////////////////////////////////
         Csi::Xml::Element::value_type xml_map;

         ////////////////////////////////////////////////////////////
         // lgrnet_settings_xml
         ////////////////////////////////////////////////////////////
         Csi::Xml::Element::value_type lgrnet_settings_xml;

         ////////////////////////////////////////////////////////////
         // settings_queue
         //
         // Holds XML elements that describe the devices for which we need
         // settings.  When this queue is empty, we will know that the
         // generation is complete.
         ////////////////////////////////////////////////////////////
         typedef std::list<Csi::Xml::Element::value_type> settings_queue_type;
         settings_queue_type settings_queue;

         ////////////////////////////////////////////////////////////
         // device_settings_getter
         //
         // Used to poll the settings for a device.
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<Device::SettingsEnumerator> device_settings_getter;

         ////////////////////////////////////////////////////////////
         // device_settings_xml
         //
         // Reference to the last device settings XML structure allocated.
         ////////////////////////////////////////////////////////////
         Csi::Xml::Element::value_type device_settings_xml;

         ////////////////////////////////////////////////////////////
         // areas_lister
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<areas_lister_type> areas_lister;
         
         ////////////////////////////////////////////////////////////
         // area_settings_queue
         //
         // A queue of collect areas for the current device for which we need
         // to retrieve collect area settings.
         ////////////////////////////////////////////////////////////
         settings_queue_type area_settings_queue;

         ////////////////////////////////////////////////////////////
         // area_settings
         //
         // The component that is used to retrieve settings for the current
         // collect area.
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<area_settings_type> area_settings;

         ////////////////////////////////////////////////////////////
         // area_settings_xml
         //
         // Reference to the last collect area settings XML structure that was
         // allocated. 
         ////////////////////////////////////////////////////////////
         Csi::Xml::Element::value_type areas_xml;
         Csi::Xml::Element::value_type area_settings_xml;

         ////////////////////////////////////////////////////////////
         // setting_val
         //
         // Used to format the current setting value.  This is kept as a
         // variable to prevent unneeded heap allocation.
         ////////////////////////////////////////////////////////////
         Csi::OStrAscStream setting_val;

         ////////////////////////////////////////////////////////////
         // dump_collect_areas
         ////////////////////////////////////////////////////////////
         bool dump_collect_areas;
      };
   };
};


#endif
