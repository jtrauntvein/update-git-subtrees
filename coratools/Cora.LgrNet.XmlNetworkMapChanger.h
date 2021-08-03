/* Cora.LgrNet.XmlNetworkMapChanger.h

   Copyright (C) 2007, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 19 June 2007
   Last Change: Monday 27 January 2020
   Last Commit: $Date: 2020-01-27 11:26:11 -0600 (Mon, 27 Jan 2020) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_LgrNet_XmlNetworkMapChanger_h
#define Cora_LgrNet_XmlNetworkMapChanger_h

#include "Cora.LgrNet.DeviceAdder.h"
#include "Cora.LgrNet.BranchMover.h"
#include "Cora.LgrNet.SettingsSetter.h"
#include "Cora.Device.SettingsSetter.h"
#include "Cora.LgrNet.ModemTypeAdder.h"
#include "Cora.LgrNet.ModemTypeChanger.h"
#include "Csi.Xml.Element.h"
#include "Csi.Html.Tag.h"


namespace Cora
{
   namespace LgrNet
   {
      // @group class forward declarations
      class XmlNetworkMapChanger;
      // @endgroup


      /**
       * Defines the interface that must be implemented by the appliction in order to use the
       * XmlNetworkMapChanger component.
       */
      class XmlNetworkMapChangerClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when all of the changes in the list have been made or after one of the changes
          * have failed.
          *
          * @param sender Specifies the component that posted this event.
          *
          * @param outcome Specifies the overall outcome of the changes.
          *
          * @param changes_list Specifies the list of changes and the outcome of each in an XML
          * structure.
          */
         enum outcome_type
         {
            outcome_unknown = 0, // unrecognised failure
            outcome_success = 1, // all changes applied
            outcome_invalid_logon = 2, // server logon failed
            outcome_session_broken = 3, // server session lost
            outcome_unsupported = 4,    // attempted to use an unsupported transaction
            outcome_server_security_blocked = 5, // blocked by server security
            outcome_unrecognised_change = 6,     // change type not recognised
            outcome_invalid_change = 7,          // failed to extract attributes from a change element
            outcome_invalid_device_name = 8,     // an invalid device name was specified
            outcome_unattachable = 9,            // could not create/move a device at the specified
                                                 // point
            outcome_unsupported_device_type = 10, // an add-device record specified a device type
                                                  // that is not supported by the server
            outcome_network_locked = 11,          // the network is locked by
                                                  // another client
            outcome_invalid_setting = 12,         // the application specified an invalid setting
            outcome_invalid_modem_name = 13,
            outcome_modem_read_only = 14,
            outcome_invalid_modem_strings = 15,
            outcome_too_many_stations = 16
         };
         virtual void on_complete(
            XmlNetworkMapChanger *sender, outcome_type outcome, Csi::Xml::Element &changes_list) = 0;

         /**
          * Called when one of the changes has been successfully completed.
          *
          * Called when one of hte pending changes has been completed.
          *
          * @param sender Specifies the component that has called this event
          *
          * @param change Specifies an XML description of the change.
          */
         virtual void on_change_complete(
            XmlNetworkMapChanger *changer, Csi::Xml::Element &change)
         { }
      };


      /**
       * Defines a component that makes mass changes to the server's network map, settings, device
       * settings, & etc. based upon an application provided map-changes XML structure.  In order to
       * use this component, the application must provide an object that is derived from class
       * XmlNetworkMapChangerClient. It must then create an instance of this component, set the
       * changes structure, and then invoke one of the two versions of start().  The changes
       * structure can be created by a call to the static reconcile_network_maps() method.  This
       * method accepts two versions of the same type of XML structure that describes the network
       * map and will generate the list of changes required to transform the first into the second.
       *
       * As each change is completed, the client's on_change_complete() method will be called.  When
       * all of the changes have been completed or when one of the changes have failed. the client's
       * on_complete() method will be called.
       */
      class XmlNetworkMapChanger:
         public ClientBase,
         public DeviceAdderClient,
         public BranchMoverClient,
         public LgrNet::SettingsSetterClient,
         public Device::SettingsSetterClient,
         public ModemTypeAdderClient,
         public ModemTypeChangerClient,
         public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the internal state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Specifies the application delegate.
          */
         XmlNetworkMapChangerClient *client;

         /**
          * Specifies the list of changes in an XML structure.
          */
         Csi::SharedPtr<Csi::Xml::Element> changes;

      public:
         /**
          * Constructor
          */
         XmlNetworkMapChanger():
            state(state_standby),
            client(0)
         { }

         /**
          * Destructor
          */
         virtual ~XmlNetworkMapChanger()
         { finish(); }


         static StrUni const changes_name;
         static StrUni const add_device_name;
         static StrUni const move_branch_name;
         static StrUni const set_device_settings_name;
         static StrUni const device_name_name;
         static StrUni const device_type_name;
         static StrUni const anchor_name;
         static StrUni const anchor_type_name;
         static StrUni const change_complete_name;
         static StrUni const setting_id_name;
         static StrUni const setting_name;
         static StrUni const set_lgrnet_settings_name;
         static StrUni const add_modem_name;
         static StrUni const modem_name_name;
         static StrUni const modem_reset_name;
         static StrUni const modem_init_name;
         static StrUni const change_modem_name;

         
         /**
          * @return Returns the XML structure that describes the changes.
          */
         typedef Csi::SharedPtr<Csi::Xml::Element> changes_type;
         changes_type &get_changes()
         { return changes; }

         /**
          * @param changes_ Specifies the XML structure that describes the changes.
          */
         void set_changes(changes_type changes_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            changes = changes_;
         }

         /**
          * Implements an algorithm to produce the set of changes that would be needed in order for
          * the network-map XML structure given in the target map to be a subset of the network
          * described by the network map structure given in the source map.  If this reconciliation
          * is not possible, a std::exception derived object will be thrown.  The return value of
          * this method can be used to set the changes property.
          */
         static changes_type reconcile_network_maps(
            Csi::Xml::Element &source_map,
            Csi::Xml::Element &target_map);

         /**
          * Generates an HTML report on the changes structure provided.
          *
          * @param tag Specifies the HTML element to which the report structure will be added.
          *
          * @param changes Specifies the changes to format.
          */
         static void describe_changes(Csi::Html::Tag &tag, changes_type &changes);

         /**
          * Formats a description of the failure to the specified stream.
          *
          * @param out Specifies the stream to which the failure will be formatted.
          *
          * @param outcome Specifies the outcome code.
          *
          * @param changes Specifies the changes that were to have happened and their outcomes.
          */
         typedef XmlNetworkMapChangerClient client_type;
         static void describe_failure(
            std::ostream &out, client_type::outcome_type outcome, Csi::Xml::Element &changes);

         /**
          * Starts the component.
          *
          * @param client_ Specifies the application delegate reference.
          *
          * @param router Specifies a newly created messaging router that has not yet been
          * connected.
          *
          * @param other_component Specifies a component that already has a connection that can be
          * shared.
          */
         void start(client_type *client_, router_handle router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            if(changes == 0)
               throw std::invalid_argument("no changes specified");
            client = client_;
            state = state_delegate;
            ClientBase::start(router);
         } 
         void start(
            client_type *client_,
            ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            if(changes == 0)
               throw std::invalid_argument("no changes specified");
            client = client_;
            state = state_delegate;
            ClientBase::start(other_component);
         }

         /**
          * Overloads the base class version to release all resources.
          */
         virtual void finish();

         /**
          * Overloads the base class version to handle the completion event for loggernet settings.
          */
         virtual void on_complete(
            LgrNet::SettingsSetter *setter,
            LgrNet::SettingsSetterClient::outcome_type outcome);

         /**
          * Overloads the base class version to handle the completion event for adding a device.
          */
         virtual void on_complete(
            DeviceAdder *adder,
            DeviceAdderClient::outcome_type outcome);

         /**
          * Overloads the base class version to handle the completion event for moving a device.
          */
         virtual void on_complete(
            BranchMover *mover,
            BranchMoverClient::outcome_type outcome);

         /**
          * Overloads the base class version to handle the completion event for changing device
          * settings.
          */
         virtual void on_complete(
            Device::SettingsSetter *setter,
            Device::SettingsSetterClient::outcome_type outcome);

         /**
          * Overloads the base class version to handle the completion event for adding a modem type.
          */
         virtual void on_complete(
            ModemTypeAdder *adder,
            ModemTypeAdder::client_type::outcome_type outcome);

         /**
          * Overloads the base class version to handle the completion event for changing a modem
          * type.
          */
         virtual void on_complete(
            ModemTypeChanger *changer,
            ModemTypeChanger::client_type::outcome_type outcome);

      protected:
         /**
          * Overloads the base class version to handle the event when the session is ready.
          */
         virtual void on_corabase_ready();

         /**
          * Overloads the base class version to handle a failure detected by the base class.
          */
         virtual void on_corabase_failure(corabase_failure_type failure);

         /**
          * Overloads the base class version to handle a failure of the connection to the server.
          */
         virtual void on_corabase_session_failure();

         /**
          * Overloads the base class version to handle asynch events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         /**
          * Starts the next pending change.
          */
         void do_next_change();

         /**
          * Starts the operation to set loggernet settingss.
          */
         void do_set_lgrnet_settings(Csi::Xml::Element::value_type &change);

         /**
          * Starts the operation to add a new device.
          */
         void do_add_device(Csi::Xml::Element::value_type &change);

         /**
          * Starts the operation to move a branch.
          */
         void do_move_branch(
            Csi::Xml::Element::value_type &change);

         /**
          * Starts the operation to set a device settings.
          */
         void do_set_device_settings(Csi::Xml::Element::value_type &change);

         /**
          * Starts an operation to add a new modem type.
          */
         void do_add_modem(Csi::Xml::Element::value_type &change);

         /**
          * Starts an operation to change a modem type.
          */
         void do_change_modem(Csi::Xml::Element::value_type &change);
         
      private:
         /**
          * Specifies the component that sets loggernet settings.
          */
         Csi::SharedPtr<LgrNet::SettingsSetter> lgrnet_settings_setter;
         
         /**
          * Specifies the component that adds a new device.
          */
         Csi::SharedPtr<DeviceAdder> adder;

         /**
          * Specifies the component that moves a branch.
          */
         Csi::SharedPtr<BranchMover> mover; 

         /**
          * Specifies the component that sets device settings.
          */
         Csi::SharedPtr<Device::SettingsSetter> device_settings_setter;

         /**
          * Specifies the component that adds a modem type.
          */
         Csi::SharedPtr<ModemTypeAdder> modem_adder;

         /**
          * Specifies the component that changes a modem type.
          */
         Csi::SharedPtr<ModemTypeChanger> modem_changer;

         /**
          * Specifies the current change that is pending.
          */
         Csi::Xml::Element::iterator current_change;
      };
   };
};


#endif
