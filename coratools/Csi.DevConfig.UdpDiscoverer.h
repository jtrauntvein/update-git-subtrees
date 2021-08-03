/* Csi.DevConfig.UdpDiscoverer.h

   Copyright (C) 2015, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 19 May 2015
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_UdpDiscoverer_h
#define Csi_DevConfig_UdpDiscoverer_h
#include "Csi.SocketAddress.h"
#include "Csi.DevConfig.Message.h"
#include "Csi.DevConfig.LibraryManager.h"
#include "Csi.SocketUdpServer.h"
#include "OneShot.h"


namespace Csi
{
   namespace DevConfig
   {
      class UdpDiscoverer;

      
      namespace UdpDiscovererHelpers
      {
         /**
          * Defines an object that provides the details of a device that has
          * been discovered.
          */
         class Device
         {
         public:
            /**
             * Constructor
             */
            Device(SocketAddress const &ip_address_):
               ip_address(ip_address_),
               device_type(0),
               major_version(0),
               minor_version(0),
               config_port(0),
               has_mac_address(false),
               has_station_name(false),
               has_serial_no(false),
               has_os_version(false),
               has_encrypted(false)
            { }

            /**
             * @return Returns the IP address for the device.
             */
            SocketAddress const &get_ip_address() const
            { return ip_address; }

            /**
             * @return Returns the IP address formatted as a string.
             */
            StrAsc format_ip_address() const
            { return ip_address.get_address(); }

            /**
             * @return Returns the model number for the device.
             */
            StrAsc const &get_model_no() const
            { return model_no; }

            /**
             * @param value Specifies the value of the model number.
             */
            void set_model_no(StrAsc const &value)
            { model_no = value; }

            /**
             * @return Returns the devconfig device type.
             */
            uint2 get_device_type() const
            { return device_type; }

            /**
             * @param value Specifies the devconfig device type code for the device.
             */
            void set_device_type(uint2 value)
            { device_type = value; }

            /**
             * @return Returns the devconfig major version.
             */
            byte get_major_version() const
            { return major_version; }

            /**
             * @param value Specifies the major version.
             */
            void set_major_version(byte value)
            { major_version = value; }

            /**
             * @return Returns the devconfig minor version.
             */
            byte get_minor_version() const
            { return minor_version; }

            /**
             * @param value Specifies the value of the minor version.
             */
            void set_minor_version(byte value)
            { minor_version  = value; }

            /**
             * @return Returns the TCP port used to reach the device.
             */
            uint2 get_config_port() const
            { return config_port; }

            /**
             * @param value Specifies the value for the config port.
             */
            void set_config_port(uint2 value)
            { config_port = value; }

            /**
             * @return Returns the MAC address for the device.
             */
            StrAsc const &get_mac_address() const
            { return mac_address; }

            /**
             * @param value Specifies the value of the mac address.
             */
            void set_mac_address(StrAsc const &value)
            {
               mac_address = value;
               has_mac_address = true;
            }

            /**
             * @return Returns true if the device reported a MAC address.
             */
            bool get_has_mac_address() const
            { return has_mac_address; }

            /**
             * @return Returns the station name reported by the device.
             */
            StrAsc const &get_station_name() const
            { return station_name; }

            /**
             * @param value Specifies the value of the station name.
             */
            void set_station_name(StrAsc const &value)
            {
               station_name = value;
               has_station_name = true;
            }

            /**
             * @return Returns true if the station name was reported by the
             * device.
             */
            bool get_has_station_name() const
            { return has_station_name; }

            /**
             * @return Returns the serial number reported by the device.
             */
            StrAsc const &get_serial_no() const
            { return serial_no; }

            /**
             * @param value Specifies the value for the serial number.
             */
            void set_serial_no(StrAsc const &value)
            {
               serial_no = value;
               has_serial_no = true;
            }

            /**
             * @return Returns true if the serial number was reported by the
             * device.
             */
            bool get_has_serial_no() const
            { return has_serial_no; }

            /**
             * @return Returns the OS version reported by the device.
             */
            StrAsc const &get_os_version() const
            { return os_version; }

            /**
             * @param value Specifies the value of the OS version.
             */
            void set_os_version(StrAsc const &value)
            {
               os_version = value;
               has_os_version = true;
            }

            /**
             * @return Returns true if the OS version was reported by the
             * device.
             */
            bool get_has_os_version() const
            { return has_os_version; }

            /**
             * @return Returns true if the device expects encryption.
             */
            bool get_encrypted() const
            { return encrypted; }

            /**
             * @param value Specifies the new value for the encrypted flag.
             */
            void set_encrypted(bool value)
            {
               encrypted = value;
               has_encrypted = true;
            }

            /**
             * @return Returns true if the device reported whether it expects
             * encryption.
             */
            bool get_has_encrypted() const
            { return has_encrypted; }

            /**
             * @return Returns the PakBus address.
             */
            uint2 get_pakbus_address() const
            { return pakbus_address; }

            /**
             * @param value Specifies the value for the PakBus address.
             */
            void set_pakbus_address(uint2 value)
            {
               pakbus_address = value;
               has_pakbus_address = true;
            }

            /**
             * @return Returns true if the PakBus address was received.
             */
            bool get_has_pakbus_address() const
            { return has_pakbus_address; }

            /**
             * @return Returns the value for the PakBus/TCP service port.
             */
            uint2 get_pakbus_tcp_port() const
            { return pakbus_tcp_port; }

            /**
             * @param value Specifies the value for the PakBus/TCP port.
             */
            void set_pakbus_tcp_port(uint2 value)
            {
               pakbus_tcp_port = value;
               has_pakbus_tcp_port = true;
            }

            /**
             * @return Returns true if the PakBus/TCP port was received.
             */
            bool get_has_pakbus_tcp_port() const
            { return has_pakbus_tcp_port; }
            
            /**
             * Reads the content for this device from the specified message.
             *
             * @return Returns true if the message was read successfully.
             *
             * @param message Specifies the message to read.
             *
             * @param library Specifies the library manager.
             */
            bool read(Message &message, LibraryManager *library);
            
         private:
            /**
             * Specifies the address of the discovered device.
             */
            SocketAddress ip_address;

            /**
             * Specifies the model number for the discovered device.
             */
            StrAsc model_no;

            /**
             * Specifies the devconfig device type code for the device.
             */
            uint2 device_type;

            /**
             * Specifies the devfconfig major version for the device.
             */
            byte major_version;

            /**
             * Specifies the devconfig minor version for the device.
             */
            byte minor_version;

            /**
             * Specifies the TCP port used to configure the device.
             */
            uint2 config_port;

            /**
             * Specifies the MAC address of the device.
             */
            bool has_mac_address;
            StrAsc mac_address;

            /**
             * Specifies the OS version for the device.
             */
            bool has_os_version;
            StrAsc os_version;
            
            /**
             * Specifies the station name for the device.
             */
            bool has_station_name;
            StrAsc station_name;

            /**
             * Specifies whether the device is configured to expect PakBus
             * encryption.
             */
            bool has_encrypted;
            bool encrypted;

            /**
             * Specifies the serial number reported by the device.
             */
            StrAsc serial_no;
            bool has_serial_no;

            /**
             * Specifies the PakBus address.
             */
            uint2 pakbus_address;
            bool has_pakbus_address;

            /**
             * Specifies the PakBus/TCP port.
             */
            uint2 pakbus_tcp_port;
            bool has_pakbus_tcp_port;
         };


         /**
          * Defines an object that listens on a UDP socket for incoming datagrams.
          */
         class Listener: public SocketUdpServer
         {
         public:
            /**
             * Constructor
             *
             * @param owner_ Specifies the object that is driving discovery.
             *
             * @param address_ Specifies the address of the interface on which we should listen.
             *
             * @param library_ Specifies a reference to the devconfig library manager.
             */
            typedef SharedPtr<LibraryManager> library_handle;
            Listener(
               UdpDiscoverer *owner_, SocketAddress const &address_, library_handle &library);

            /**
             * Destructor
             */
            virtual ~Listener();

            /**
             * Overloaded to handle an incoming datagram.
             */
            virtual void on_datagram(Packet &packet, SocketAddress const &sender);

            /**
             * Overloaded to handle a socket error.
             */
            virtual void on_socket_error(int error_code);

            /**
             * Sends a UDP broadcast on the socket.
             */
            void transmit();

         public:
            /**
             * Specifies the owner of this listener.
             */
            UdpDiscoverer *owner;

            /**
             * Specifies the address of the interface on which we will listen.
             */
            SocketAddress address;

            /**
             * Specifies the address to which we will broadcast.
             */
            SocketAddress broadcast_address;

            /**
             * Specifies the library manager.
             */
            library_handle library;

            /**
             * Specifies the last transaction number that we sent.
             */
            byte last_tran_no;
         };
      };


      /**
       * Defines the interface that the application class must implement in order to use the UDP
       * discoverer.
       */
      class UdpDiscovererClient: public InstanceValidator
      {
      public:
         /**
          * Called to report that listening has failed on the specified interface address.
          *
          * @param discoverer Specifies the component reporting this event.
          *
          * @param address Specifies the address for the interface that failed.
          *
          * @param message Specifies the error message associated with the failure.
          */
         virtual void on_listen_fail(
            UdpDiscoverer *discoverer,
            SocketAddress const &address,
            StrAsc const &message)
         { }

         /**
          * Called to report that a new device has been discovered.
          *
          * @param discoverer Specifies the component reporting this event.
          *
          * @param device Specifies the device that was discovered.
          */
         typedef UdpDiscovererHelpers::Device device_type;
         typedef SharedPtr<device_type> device_handle;
         virtual void on_device(
            UdpDiscoverer *discoverer,
            device_handle const &device) = 0;
      };


      /**
       * Defines a component that can be used to discover CSI devices on the local area network
       * using UDP broadcasts.  In order to use this component, the application must provide an
       * object that inherits from class UdpDiscovererClient.  It should then create an instance of
       * this class, optionally specify the filter device type and one shot timer, and call
       * start().  As the component accepts responses to the broadcast and discovers new devices, it
       * will notify the application using the client's on_device() method.  The application can
       * stop this process at any time by calling finish() or by deleting this component.
       */
      class UdpDiscoverer: public OneShotClient
      {
      public:
         /**
          * Constructor.
          */
         UdpDiscoverer();

         /**
          * Destructor
          */
         virtual ~UdpDiscoverer()
         { finish(); }

         /**
          * @param value Specifies the device type filter.  In order to receive only notifications
          * for devices of a certain type, the application can specify the devconfig device type
          * code.  If set to a value of 0xffff (the default), any device type will be reported.
          *
          * @throw MsgExcept Throws a MsgExcept object if the component has already been started.
          */
         void set_device_mask(uint2 value);

         /**
          * @return Returns the mask used to match received devices.
          */
         uint2 get_device_mask() const
         { return device_mask; }

         /**
          * @param timer_ Specifies the timer that will be used by this component to time the
          * transmissions of broadcasts.  If this value is set to null when start() is called, a new
          * timer will be created.
          *
          * @throw MsgExcept Throws a MsgExcept object if the component has alredy been started.
          */
         typedef SharedPtr<OneShot> timer_handle;
         void set_timer(timer_handle timer_);

         /**
          * @return Returns the timer used by this component.
          */
         timer_handle get_timer() const
         { return timer; }

         /**
          * @param value Specifies the UDP port on which the service will work.
          *
          * @throw MsgExcept Throws a MsgExcept object if the component is already started.
          */
         void set_discover_port(uint2 value);

         /**
          * @return Returns the UDP port used for discovery broadcasts.
          */
         uint2 get_discover_port() const
         { return discover_port; }

         /**
          * @param library_ Specifies the devconfig library manager.
          *
          * @throw MsgExcept Throws a MsgExcept if the component is already started.
          */
         typedef SharedPtr<LibraryManager> library_handle;
         void set_library(library_handle library_);

         /**
          * @return Returns the devconfig library manager.
          */
         library_handle get_library() const
         { return library; }

         /**
          * @param value Specifies the interval in milliseconds between broadcasts on
          * the network.  The effective interval used by the component will be the maximum value of
          * 5000 and this interval.
          */
         void set_broadcast_interval(uint4 value)
         { broadcast_interval = value; }

         /**
          * @return Returns the interval in milliseconds between network broadcasts.
          */
         uint4 get_broadcast_interval() const
         { return broadcast_interval; }

         /**
          * Called to create the UDP sockets and to start the broadcasts.
          *
          * @param client_ Specifies the application object that will receive notifications.
          *
          * @throw MsgExcept Throws a MsgExcept object if the component has already been started.
          */
         typedef UdpDiscovererClient client_type;
         void start(client_type *client_);

         /**
          * Called to shut down all UDP sockets and return this component to a standby state.
          */
         void finish();

         /**
          * Overloads the base class version to handle the broadcast timer.
          */
         virtual void onOneShotFired(uint4 id);

         // @group: methods that allow this component to act as a container of devices.

         /**
          * @return Returns the begin iterator for the devices container.
          */
         typedef UdpDiscovererHelpers::Device device_type;
         typedef SharedPtr<device_type> device_handle;
         typedef std::map<SocketAddress, device_handle> devices_type;
         typedef devices_type::value_type value_type;
         typedef devices_type::iterator iterator;
         typedef devices_type::const_iterator const_iterator;
         iterator begin()
         { return devices.begin(); }
         const_iterator begin() const
         { return devices.begin(); }

         /**
          * @return Returns the end iterator for the devices container.
          */
         iterator end()
         { return devices.end(); }
         const_iterator end() const
         { return devices.end(); }

         /**
          * @return Returns the number of devices discovered.
          */
         typedef devices_type::size_type size_type;
         size_type size() const
         { return devices.size(); }

         /**
          * @return Returns true if there are no devices that have been discovered.
          */
         bool empty() const
         { return devices.empty(); }

         /**
          * @return Returns the iterator for the device at the specified address or end() if not
          * present.
          *
          * @param address Specifies the address to fond.
          */
         iterator find(SocketAddress const &address)
         { return devices.find(address); }
         const_iterator find(SocketAddress const &address) const
         { return devices.find(address); }

         /**
          * Removes all devices that have been discovered.  If this component is still started,
          * the device list will be built up again.
          */
         void clear()
         { devices.clear(); }
         
         // @endgroup:

         /**
          * @return Returns the number of pendinfg listeners.
          */
         size_t get_listeners_count() const
         { return listeners.size(); }
         
      private:
         /**
          * Handles the incoming notification of a device report.
          */
         void on_device_report(device_handle &device);

         /**
          * Handles the notification of a listener failure.
          */
         typedef UdpDiscovererHelpers::Listener listener_type;
         void on_listener_fail(listener_type *listener, StrAsc const &message);
         
      private:
         /**
          * Specifies the list of listener objects.
          */
         friend class UdpDiscovererHelpers::Listener;
         typedef SharedPtr<listener_type> listener_handle;
         typedef std::deque<listener_handle> listeners_type;
         listeners_type listeners;

         /**
          * Specifies the list of devices that have already been discovered.
          */
         devices_type devices;

         /**
          * Specifies the object that will receive notifications.
          */
         client_type *client;

         /**
          * Specifies the device type that the application wants reported.
          */
         uint2 device_mask;

         /**
          * Specifies the timer used to transmit broadcasts.
          */
         timer_handle timer;

         /**
          * Specifies the port on which we will listen.
          */
         uint2 discover_port;

         /**
          * Specifies the devconfig library manager.
          */
         library_handle library;

         /**
          * Specifies the timer ID that will identify broadcast events.
          */
         uint4 broadcast_id;

         /**
          * Specifies the interval in milliseconds at which this component will send discovery
          * broadcasts.
          */
         uint4 broadcast_interval;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_started
         } state;
      };
   };
};


#endif

