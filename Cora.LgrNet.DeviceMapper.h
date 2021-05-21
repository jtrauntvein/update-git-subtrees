/* Cora.LgrNet.DeviceMapper.h

   Copyright (C) 2002, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 23 August 2002
   Last Change: Tuesday 22 January 2019
   Last Commit: $Date: 2019-01-22 15:04:50 -0600 (Tue, 22 Jan 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_DeviceMapper_h
#define Cora_LgrNet_DeviceMapper_h


#include "Cora.ClientBase.h"
#include "Cora.LgrNet.Defs.h"
#include "Csi.InstanceValidator.h"
#include <list>
#include <map>


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class DeviceMapper;
      namespace DeviceMapperHelpers
      {
         class Device;
      };
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class DeviceMapperClient
      ////////////////////////////////////////////////////////////
      class DeviceMapperClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_change_start
         //
         // Called when a new network map notification has been received from the server.  This will
         // always precede other on_device_added, on_device_renamed, on_device_deleted, or
         // on_device_parent_changed notifications.
         ////////////////////////////////////////////////////////////
         virtual void on_change_start(
            DeviceMapper *mapper)
         { }

         ////////////////////////////////////////////////////////////
         // on_change_complete
         //
         // Called when a new network map notification has been completely processed.  This method
         // will be called after on_change_start(), on_device_added(), on_device_renamed(),
         // on_device_deleted(), or on_device_parent_changed() notifications. 
         ////////////////////////////////////////////////////////////
         virtual void on_change_complete(
            DeviceMapper *mapper)
         { }

         ////////////////////////////////////////////////////////////
         // on_device_added
         //
         // Called when a new device has been added to the server's network map.
         ////////////////////////////////////////////////////////////
         typedef DeviceMapperHelpers::Device device_type;
         typedef Csi::SharedPtr<device_type> device_handle;
         virtual void on_device_added(
            DeviceMapper *mapper,
            device_handle &device)
         { }

         ////////////////////////////////////////////////////////////
         // on_device_deleted
         //
         // Called when a device has been deleted from the server's network map.
         ////////////////////////////////////////////////////////////
         virtual void on_device_deleted(
            DeviceMapper *mapper,
            device_handle &device)
         { }

         ////////////////////////////////////////////////////////////
         // on_device_renamed
         //
         // Called when the device has been given a new name.
         ////////////////////////////////////////////////////////////
         virtual void on_device_renamed(
            DeviceMapper *mapper,
            device_handle &device,
            StrUni const &old_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_device_parent_change
         ////////////////////////////////////////////////////////////
         virtual void on_device_parent_change(
            DeviceMapper *mapper,
            device_handle &device,
            uint4 old_parent_id)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_logon = 1,
            failure_session_broken = 2,
            failure_unsupported = 3,
            failure_server_security = 4,
         };
         virtual void on_failure(
            DeviceMapper *mapper,
            failure_type failure) = 0;

         /**
          * Called to report that the component received notification that a snapshot has been
          * restored.
          *
          * @param sender Specifies the component sending this notification.
          */
         virtual void on_snapshot_restored(DeviceMapper *sender)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class DeviceMapper
      //
      // This class defines a component that maintains an internal model of the server's network map
      // and provides event notifications to the application when changes occur to that model.  In
      // order to use this component, an application must provide an object derived from class
      // DeviceMapperClient.  It must then create an instance of this component (or a class derived
      // from it) and, after optionally setting the appropriate attributes, call one of the two
      // versions of start().  Once the connection to the server has been established and the first
      // network map notification received, various client methods will be invoked giving the events
      // that a change has taken place, device added, and the change processing is complete.  Any
      // time after that the same types of notifications can take place.  During this time, the
      // application can browse through the device list by accessing the begin(), end(), and other
      // related methods.
      //
      // If, at any time, the connection to the server is lost or other failure occurs, the client's
      // on_failure() notification will be called and the component returned to a standby state.  
      ////////////////////////////////////////////////////////////
      class DeviceMapper:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // device_map
         //
         // The list of devices in the order that they were received from the server in the last
         // network map notification.
         ////////////////////////////////////////////////////////////
      public:
         typedef Csi::SharedPtr<DeviceMapperHelpers::Device> device_handle;
         typedef std::list<device_handle> device_map_type;
      private:
         device_map_type device_map;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         DeviceMapper();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~DeviceMapper();

         ////////////////////////////////////////////////////////////
         // start (new connection)
         ////////////////////////////////////////////////////////////
         typedef DeviceMapperClient client_type;
         void start(
            client_type *client,
            router_handle &router);

         ////////////////////////////////////////////////////////////
         // start (reuse connection)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(std::ostream &out, client_type::failure_type failure);

         ////////////////////////////////////////////////////////////
         // begin
         //
         // Returns the first iterator in the device map. 
         ////////////////////////////////////////////////////////////
         typedef device_map_type::iterator iterator;
         typedef device_map_type::const_iterator const_iterator;
         iterator begin() { return device_map.begin(); }
         const_iterator begin() const { return device_map.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         //
         // Returns the end iterator of the device map
         ////////////////////////////////////////////////////////////
         iterator end() { return device_map.end(); }
         const_iterator end() const { return device_map.end(); }
         
         ////////////////////////////////////////////////////////////
         // rbegin
         ////////////////////////////////////////////////////////////
         typedef device_map_type::reverse_iterator reverse_iterator;
         typedef device_map_type::const_reverse_iterator const_reverse_iterator;
         reverse_iterator rbegin() { return device_map.rbegin(); }
         const_reverse_iterator rbegin() const { return device_map.rbegin(); }
         
         ////////////////////////////////////////////////////////////
         // rend
         ////////////////////////////////////////////////////////////
         reverse_iterator rend() { return device_map.rend(); }
         const_reverse_iterator rend() const { return device_map.rend(); }
         
         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         size_t size() const { return device_map.size(); }
         
         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const { return device_map.empty(); }

         ////////////////////////////////////////////////////////////
         // find_device
         //
         // Searches for the device specified by the object_id parameter.  Will return the device in
         // the first parameter if found and will also return true.  Otherwise, the first parameter
         // will be set to a null pointer and the return value will be false.
         ////////////////////////////////////////////////////////////
         bool find_device(device_handle &device, uint4 object_id);

         ////////////////////////////////////////////////////////////
         // find_device
         //
         // Performs the same task as the first version of find_device() only it looks up the device
         // by name rather than object id.
         ////////////////////////////////////////////////////////////
         bool find_device(device_handle &device, StrUni const &name);
         
      protected:
         ////////////////////////////////////////////////////////////
         // make_device
         //
         // Creates a device object.  This method can be overloaded by a derived class to allow for
         // application defined device types. 
         ////////////////////////////////////////////////////////////
         virtual device_handle make_device(
            uint4 object_id,
            DeviceTypes::device_type type_code);
         
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

         /**
          * Overloads the base class version to report the notification to the client.
          */
         virtual void on_snapshot_restored();

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);
         
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
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
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // master_list
         //
         // The master list of devices ordered by their object identifiers.  
         ////////////////////////////////////////////////////////////
         typedef std::map<uint4, device_handle> master_list_type;
         master_list_type master_list;
      };
      

      namespace DeviceMapperHelpers
      {
         ////////////////////////////////////////////////////////////
         // class Device
         //
         // Describes the fundamental characteristics of a device object.  This class can be
         // extended by the application and provided to the mapper component by the application if
         // the application extends the component type and overloads the component class'
         // make_device() method. 
         ////////////////////////////////////////////////////////////
         class Device
         {
         protected:
            ////////////////////////////////////////////////////////////
            // object_id
            //
            // Uniquely identifies the device in the scope of the network map. Even if the device is
            // renamed, this identifier will stay the same.  It is declared as const here to
            // reinforce the idea that it should be immutable.
            ////////////////////////////////////////////////////////////
            uint4 const object_id;

            ////////////////////////////////////////////////////////////
            // type_code
            //
            // Identifies the type of device in the network map.  There is no means in the server
            // interface to change the type of a device once it is created short of deleting the
            // device and creating a new one in its place with a different type.  This member is
            // declared as const to reinforce this idea of immutability.
            ////////////////////////////////////////////////////////////
            DeviceTypes::device_type const type_code;

            ////////////////////////////////////////////////////////////
            // name
            //
            // Relates the current name of the device in the network map.
            ////////////////////////////////////////////////////////////
            StrUni name;

            ////////////////////////////////////////////////////////////
            // indentation_level
            //
            // Identifies the device's current level of indentation in the network map.  A device at
            // the root level will have a level of zero.  
            ////////////////////////////////////////////////////////////
            uint4 indentation_level;

            ////////////////////////////////////////////////////////////
            // parent_id
            //
            // The object id of the current parent to this device.
            ////////////////////////////////////////////////////////////
            uint4 parent_id;

            ////////////////////////////////////////////////////////////
            // children
            //
            // The list of identifiers for device that are immediate children to this device. 
            ////////////////////////////////////////////////////////////
         public:
            typedef std::list<uint4> children_type;
         protected:
            children_type children;

            ////////////////////////////////////////////////////////////
            // map_version
            //
            // The map version for this device.  This is used by the component to determine if the
            // device should be deleted after receiving a notification.
            ////////////////////////////////////////////////////////////
            uint4 map_version;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            //
            // This constructor requires the two immutable characteristics of a device those being
            // the object identifier and device type.
            ////////////////////////////////////////////////////////////
            Device(
               uint4 object_id_,
               DeviceTypes::device_type type_code_):
               object_id(object_id_),
               type_code(type_code_),
               parent_id(0),
               indentation_level(0),
               map_version(0)
            { }

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~Device()
            { }

            ////////////////////////////////////////////////////////////
            // get_object_id
            ////////////////////////////////////////////////////////////
            uint4 get_object_id() const { return object_id; }

            ////////////////////////////////////////////////////////////
            // get_type_code
            ////////////////////////////////////////////////////////////
            DeviceTypes::device_type get_type_code() const { return type_code; }

            ////////////////////////////////////////////////////////////
            // get_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_name() const { return name; }

            ////////////////////////////////////////////////////////////
            // set_name
            ////////////////////////////////////////////////////////////
            virtual void set_name(StrUni const &name_)
            { name = name_; }
            
            ////////////////////////////////////////////////////////////
            // get_indentation_level
            ////////////////////////////////////////////////////////////
            uint4 get_indentation_level() const { return indentation_level; }

            ////////////////////////////////////////////////////////////
            // set_identation_level
            ////////////////////////////////////////////////////////////
            virtual void set_indentation_level(uint4 indentation_level_)
            { indentation_level = indentation_level_; }
            
            ////////////////////////////////////////////////////////////
            // get_parent_id
            ////////////////////////////////////////////////////////////
            uint4 get_parent_id() const { return parent_id; }

            ////////////////////////////////////////////////////////////
            // set_parent_id
            ////////////////////////////////////////////////////////////
            virtual void set_parent_id(uint4 parent_id_) { parent_id = parent_id_; }

            ////////////////////////////////////////////////////////////
            // children_begin
            //
            // Returns a const iterator for the children list
            ////////////////////////////////////////////////////////////
            typedef children_type::const_iterator children_iterator;
            children_iterator children_begin() const { return children.begin(); }

            ////////////////////////////////////////////////////////////
            // children_end
            ////////////////////////////////////////////////////////////
            children_iterator children_end() const { return children.end(); }

            ////////////////////////////////////////////////////////////
            // children_size
            ////////////////////////////////////////////////////////////
            size_t children_size() const { return children.size(); }

            ////////////////////////////////////////////////////////////
            // clear_children
            ////////////////////////////////////////////////////////////
            virtual void clear_children() { children.clear(); }

            ////////////////////////////////////////////////////////////
            // add_child
            ////////////////////////////////////////////////////////////
            virtual void add_child(uint4 child_id) { children.push_back(child_id); }

            ////////////////////////////////////////////////////////////
            // get_map_version
            ////////////////////////////////////////////////////////////
            uint4 get_map_version() const { return map_version; }

            ////////////////////////////////////////////////////////////
            // set_map_version
            ////////////////////////////////////////////////////////////
            void set_map_version(uint4 map_version_) { map_version = map_version_; }
         };
      };
   };
};


#endif
