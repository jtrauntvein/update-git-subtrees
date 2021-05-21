/* Csi.DevConfig.DeviceDesc.h

   Copyright (C) 2003, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 10 December 2003
   Last Change: Friday 06 October 2017
   Last Commit: $Date: 2018-03-15 16:43:21 -0600 (Thu, 15 Mar 2018) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_DeviceDesc_h
#define Csi_DevConfig_DeviceDesc_h

#include "Csi.DevConfig.SettingCatalog.h"


namespace Csi
{
   // @group: class forward declarations
   namespace Xml
   {
      class Element;
   };
   // @endgroup

   
   namespace DevConfig
   {
      /**
       * Defines a component that organises the information read from a device description file.
       */
      class DeviceDesc
      {
         /**
          * Specifies the collection of setting catalogues read from the device description.
          */
      public:
         typedef SharedPtr<SettingCatalog> value_type;
         typedef std::list<value_type> catalogs_type;
      private:
         catalogs_type catalogs;
         
         /**
          * Specifies the directory from which the device description was read.
          */
         StrAsc library_directory;
         
         /**
          * Specifies the reported model number for this device.
          */
         StrAsc model_no;

         /**
          * Specifies the model number that should be presented to the user.
          */
         StrAsc present_model_no;
         
         /**
          * Specifies the devconfig device type code for this device.
          */
         uint2 device_type_code;
         
         /**
          * Specifies the path of the file that contains user instructions for how to connect to
          * this  device.
          */
         StrAsc connect_instructions_file_name;

         /**
          * Specifies the path to the file that contains the user instructions for connecting to
          * this device without the use of TCP/IP.
          */
         StrAsc connect_instructions_noip_file_name;

         /**
          * Specifies the suggested baud rate for this device type.
          */
         uint4 config_baud_rate;

         /**
          * Set to true if the value of config_baud_rate should be used and no other value allowed.
          */
         bool force_baud_rate;

         /**
          * Set to true if this device description should be excluded from the devconfig device
          * types list.
          */
         bool exclude;

         /**
          * Specifies the protocol that should be used to configure the device.
          */
      public:
         enum config_protocol_type
         {
            config_devconfig = 1,
            config_pakbus = 2,
            config_tx3xx = 3,
            config_at_commands = 4,
            config_none
         };
      private:
         config_protocol_type config_protocol;

         /**
          * Specifies the path to the file that maps setting identifiers to AT commands when the at
          * commands protocol should be used.
          */
         StrAsc at_command_map;

         /**
          * Specifies the location of the user instructions to send an operating system.  This
          * should be specified relative to the library directory path.
          */
         StrAsc os_instructions_url;

         /**
          * Specifies the baud rate that should be used to send an operating system.
          */
         uint4 os_baud_rate;

         /**
          * Specifies the protocol that should be used to send an operating system to the device.
          */
      public:
         enum os_protocol_type
         {
            os_protocol_none = 0,
            os_protocol_csterm = 1,
            os_protocol_csos = 2,
            os_protocol_srecord = 3,
            os_protocol_simplified_srecord = 4,
            os_protocol_devconfig = 5,
            os_protocol_srecord_usb = 7,
            os_protocol_sr50a = 8
         };
      private:
         os_protocol_type os_protocol;

         /**
          * Specifies the extension of operating system image files for this device.
          */
         StrAsc os_file_extension;

         /**
          * Specifies a message that will be shown to the user after an operating system has been
          * successfully sent.
          */
         StrAsc os_admonishment;

         /**
          * Set to true if the device description had a section that configures the srecord
          * protocol.
          */
         bool has_srecord_config;

         /**
          * Specifies the model number that will be expected to be found in the srecord image format
          * for this device type.
          */
         StrAsc srecord_model_no;

         /**
          * Specifies the address at which the model  number will be expected to be found in the
          * srecord image format for this device.
          */
         uint4 srecord_model_address;

         /**
          * Specifies the address where the signature of the os image will be expected to be
          * written for this device type.
          */
         uint4 srecord_os_sig_address;

         /**
          * Set to true if the srecord format image should be confirmed by reading the file to find
          * the model number for this device type.  If set to false, the os loader will simply use
          * the value of srecord_model_no to initiate the OS download for the device.  This is
          * needed for some device types, like the CS120 whose processor architecture makes it
          * difficult to read the model number is a standard way.
          */
         bool srecord_should_confirm;

         /**
          * Specifies the interval, in milliseconds, that the application should poll for new
          * setting values for settings that are marked as auto-refresh.  A value of zero (the
          * default) will disable any auto refresh from taking place.
          */
         uint4 refresh_interval;

         /**
          * Specifis the number of retries that should be made when starting a session with this
          * device type.  It will default to 375 if not specified.
          */
         uint4 session_start_retries;

         /**
          * Specifies the time out interval in milli-seconds for attempts to start a session with
          * the device.  This value will default to 40 milliseconds of not specified.
          */
         uint4 session_start_timeout;

         /**
          * Lists all of the files that can be used to describe off-line
          * settings for the device. This was formally a single file optionally
          * specified as an attribute in the device description element.  With
          * the advent of the CR6 having various daughter board configurations,
          * there is now a need to be able to specify multiple default
          * summaries.
          *
          * The first member of each pair is the name of the file and the
          * second member for each pair is a description string for that
          * summary.
          */
      public:
         typedef std::pair<StrAsc, StrAsc> default_summary_type;
         typedef std::deque<default_summary_type> summaries_type;
      private:
         summaries_type default_summaries;

         /**
          * Set to true (the default value) if this device properly supports the factory defaults
          * operation.
          */
         bool supports_defaults;

         /**
          * Set to true if this device type can be configured using the TCP protocol.
          */
         bool network_config;

         /**
          * Specifies the UDP port that should be used to broadcast UDP discovery packets for this
          * device type.
          */
         uint2 discovery_udp_port;

         /**
          * Set to true if an operating system can be sent to the device using TCP/IP.
          */
         bool network_os;

         /**
          * Set to true if the device supports TLS encryption.
          */
         bool supports_encryption;

         /**
          * Specifies the category string for this device type.
          */
         StrAsc category;

         /**
          * Specifies the name of the file that should serve as an icon for this device.
          */
         StrAsc icon_file_name;

         /**
          * Set to true if this device supports PakBus encryption.
          */
         bool supports_pakbus_encryption;

         /**
          * Set to true if this device supports offline defaults.
          */
         bool offline_defaults;

         /**
          * Specifies a description for this device type.
          */
         StrAsc description;

         /**
          * Sety to true if the settings editor panel should be shown for this device type.
          */
         bool supports_settings_editor;

         /**
          * Specifies the title that should be used for the PakBus/TCP password.
          */
         StrAsc network_config_password_title;

         /**
          * Specifies the collection of USB driver files used for this device.
          */
         typedef std::deque<StrAsc> usb_drivers_type;
         usb_drivers_type usb_drivers;

         /**
          * Specifies the name of the factory defaults button that should be used for this device
          * type.
          */
         StrUni defaults_button_name;

         /**
          * Set to true (the default) if this device support the terminal.
          */
         bool supports_terminal;

         /**
          * Specifies the message shown to the user before settings are applied.
          */
         StrUni apply_message;

         /**
          * Specifies the name of the device description file.
          */
         StrAsc device_description_file_name;

         /**
          * Set to true if this device should only be shown when the application is run in factory
          * mode.
          */
         bool factory_only;

         /**
          * Specifies the state of reading the settings for this device type.
          */
         enum init_state_type
         {
            init_read_nothing,
            init_read_header,
            init_read_everything
         } init_state;
         
      public:
         /**
          * Construct with a file name.
          *
          * @param file_name_ Specifies the device description file name for this device type.
          *
          * @param library_directory_ Specifies the directory when the devconfig library is located.
          */
         DeviceDesc(
            StrAsc const &file_name_,
            StrAsc const &library_directory_);

         /**
          * Construct with a byte array that represents the device description image.
          *
          * @param dd_image Specifies the byte array that contains the device description image.
          *
          * @param dd_image_len Specifies the length of the byte array.
          *
          * @param library_directory_ Specifies the path to the devconfig library directory.
          */
         DeviceDesc(
            void const *dd_image,
            uint4 dd_image_len,
            StrAsc const &library_directory_);
         
         /**
          * @return Returns the model number of this device.  If use_present is set to true, the
          * user model number should be returned instead.
          */
         StrAsc const &get_model_no(bool use_present = false) const
         {
            if(use_present && present_model_no.length() > 0)
               return present_model_no;
            else
               return model_no;
         }
         
         /**
          * @return Returns the device type code for this device.
          */
         uint2 get_device_type() const
         { return device_type_code; }
         
         /**
          * @return Returns the path to the user connect instructions.
          */
         StrAsc const &get_connect_instructions_file_name() const
         { return connect_instructions_file_name; }

         /**
          * @return the path to the user instructions to connect without TCP/IP.
          */
         StrAsc const &get_connect_instructions_noip_file_name() const
         { return connect_instructions_noip_file_name; }
         
         /**
          * @return Returns the preferred baud rate for connecting to the device.  A value of zero
          * will be returned if there is no preferred baud rate.
          */
         uint4 get_config_baud_rate() const
         { return config_baud_rate; }

         /**
          * @return Returns true if there is a specific baud rate that must be used for connecting
          * to the device.
          */
         bool get_force_baud_rate() const
         { return force_baud_rate && config_baud_rate != 0; }

         /**
          * @return Returns the protocol that should be used to configure the device.
          */
         config_protocol_type get_config_protocol() const
         { return config_protocol; }

         /**
          * @return Returns the path to the XML file that should be used to map AT commands to
          * settings for the AT protocol.
          */
         StrAsc const &get_at_command_map() const
         { return at_command_map; }

         /**
          * @return Returns true if devconfig should exclude this device type from its device list.
          */
         bool get_exclude() const
         { return exclude; }

         /**
          * @return Returns the interval, in milliseconds, at which the application should
          * automatically poll for settings that are marked as auto-refresh.  If this value is zero,
          * no automatic polling will take place.
          */
         uint4 get_refresh_interval() const
         { return refresh_interval; }

         /**
          * @return Returns true if the device should only be visible when run in factory mode.
          */
         bool get_factory_only() const
         { return factory_only; }
         
         //@group catalogs container access

         /**
          * @return Returns the iterator to the first catalogue for this device type.
          */
         typedef catalogs_type::iterator iterator;
         iterator begin()
         {
            check_loaded();
            return catalogs.begin();
         }

         /**
          * @return Returns the end of sequence iterator for the catalogues collection.
          */
         iterator end()
         {
            check_loaded();
            return catalogs.end();
         }

         /**
          * @return Returns the number of catalogues for this device type.
          */
         catalogs_type::size_type size() const
         {
            return catalogs.size();
         }
         
         /**
          * @return Returns true if there are no catalogues sotred for this device type.
          */
         bool empty() const
         {
            return catalogs.empty();
         }

         /**
          * @return Returns a reference to the first setting catalogue.
          */
         value_type const &front() const
         {
            return catalogs.front();
         }

         /**
          * @return Returns a reference to the last setting catalogue.
          */
         value_type const &back() const
         {
            return catalogs.back();
         }
         
         //@endgroup

         /**
          * @return Returns an iterator to the catalogue that matches the specified major version
          * number.  end() will be returned if there is no such catalogue.
          */
         iterator find_catalog(byte major_version);
         
         /**
          * @return Returns the protocol that should be used to send an OS to this device.
          */
         os_protocol_type get_os_protocol() const
         { return os_protocol; }

         /**
          * @return Returns the baud rate that should be used to send an operating system to this
          * device.   A value of zero will indicate that there is no preferred baud rate.
          */
         uint4 get_os_baud_rate() const
         { return os_baud_rate; }

         /**
          * @return Returns the path of the file that contains user instructions to send an
          * operating system to the device.  This path will be relative to the library directory.
          */
         StrAsc const &get_os_instructions_url() const
         { return os_instructions_url; }

         /**
          * @return Returns the file extension pattern for operating system file images.
          */
         StrAsc const &get_os_file_extension() const
         { return os_file_extension; }

         /**
          * @return Returns the text that should be shown to the user after an operating system has
          * been sent.
          */
         StrAsc const &get_os_admonishment() const
         { return os_admonishment; }

         /**
          * @return Returns true if the device description contained a configuration for configuring
          * the srecord protocol os sender.
          */
         bool get_has_srecord_config() const
         { return has_srecord_config; }

         /**
          * @return Returns true if the srecord protocol sender should read the model number from
          * the OS image to confirm it before sending the image to the device.
          */
         bool get_srecord_should_confirm() const
         { return srecord_should_confirm; }

         /**
          * @return Returns the model number that should be sent to the device when initiating the
          * srecord protocol for sending an OS.
          */
         StrAsc const &get_srecord_model_no() const
         { return srecord_model_no; }

         /**
          * @return Returns the address where the model number is expected to reside in the OS image
          * for this device type.
          */
         uint4 get_srecord_model_address() const
         { return srecord_model_address; }

         /**
          * @return Returns the address in an srecord image file where the operating system file
          * signature should be written.
          */
         uint4 get_srecord_os_sig_address() const
         { return srecord_os_sig_address; }

         /**
          * @return Returns the number of retry attempts should be made when first attempting to
          * establish a session with this device.
          */
         uint4 get_session_start_retries() const
         { return session_start_retries; }

         /**
          * @return Specifies the time interval between retry attempts when first attempting to
          * start a session with the device.
         */
         uint4 get_session_start_timeout() const
         { return session_start_timeout; }

         /**
          * @return Returns the first file name in the default summaries list.
          * This will be an empty string if there are no summaries provided.
          */
         StrAsc get_default_summary() const
         {
            StrAsc rtn;
            if(!default_summaries.empty())
               rtn = default_summaries.front().first;
            return rtn;
         }

         /**
          * @return Returns a const iterator to the first item in the default
          * summaries list.
          */
         typedef summaries_type::const_iterator summary_const_iterator;
         summary_const_iterator summaries_begin() const
         { return default_summaries.begin(); }

         /**
          * @return Returns the end iterator for the set of default sumamries
          * for this device.
          */
         summary_const_iterator summaries_end() const
         { return default_summaries.end(); }

         /**
          * @return Returns true if there are no default summaries for this
          * device.
          */
         bool summaries_empty() const
         { return default_summaries.empty(); }

         /**
          * @return Returns the number of default summaries are given for this
          * device.
          */
         typedef summaries_type::size_type summaries_size_type;
         summaries_size_type summaries_size() const
         { return default_summaries.size(); }

         /**
          * @return Returns true if the factory defaults operation is properly supported by this
          * device type.
          */
         bool get_supports_factory_defaults() const
         { return supports_defaults; }

         /**
          * @return Specifies the name that should be assigned to the factory defaults button for
          * this device.
          */
         StrUni const &get_factory_defaults_button_name() const
         { return defaults_button_name; }

         /**
          * @return Returns true if the device can be configured using a TCP/IP connection.
          */
         bool get_network_config() const
         { return network_config; }

         /**
          * @return Returns the title that should be used for the control that specified the TCP
          * password for the device.
          */
         StrAsc const &get_network_config_password_title() const
         { return network_config_password_title; }

         /**
          * @return Returns the UDP port that should be used to broadcast UDP discovery messages for
          * this device type.
          */
         uint2 get_discovery_udp_port() const
         { return discovery_udp_port; }

         /**
          * @return Returns true if an operating system can be sent to the device using TCP/IP.
          */
         bool get_network_os() const
         { return network_os; }

         /**
          * @return Returns true if the device supports TLS encryption.
          */
         bool get_supports_encryption() const
         { return supports_encryption; }

         /**
          * @return Returns a string that identifies the category for this device type.
          */
         StrAsc const &get_category() const
         { return category; }

         /**
          * @return Returns a string that identifies the path to an image file that can be used to
          * identify this device.
          */
         StrAsc const &get_icon_file_name() const
         { return icon_file_name; }

         /**
          * @return Returns true of the device supports PakBus encryption.
          */
         bool get_supports_pakbus_encryption() const
         { return supports_pakbus_encryption; }

         /**
          * @return Returns true if this device supports off-line defaults.
          */
         bool get_offline_defaults() const
         { return offline_defaults; }

         /**
          * @return Returns the description for this device type.
          */
         StrAsc const &get_description() const
         {
            if(description.length() > 0)
               return description;
            else
               return model_no;
         }

         /**
          * @return Returns true if the settings editor panel should be shown for this device type.
          */
         bool get_supports_settings_editor() const
         { return supports_settings_editor; }

         /**
          * Executes dpinst to install any device driver files for this device.
          */
         void install_usb_drivers();

         /**
          * Evaluates the program name and command line arguments that should be used to install an
          * operating system for this device.
          *
          * @param program_name Specifies the path to the program.
          *
          * @param command_line Reference to the command line.
          */
         void get_install_usb_items(StrAsc &program_name, StrAsc &command_line);

         /**
          * @return Returns true if the terminal emulator should be shown for this device type.
          */
         bool get_supports_terminal() const
         { return supports_terminal; }

         /**
          * @return Returns a message that should be shown to the user in order to confirm whether
          * settings should be saved.
          */
         StrUni const &get_apply_message() const
         { return apply_message; }

         /**
          * @return Returns the directory where the devconfig library is located.
          */
         StrAsc const &get_library_directory() const
         { return library_directory; }

         /**
          * Checks to see if the device description is completely loaded and, if not, parses the
          * entire file.
          *
          * @throws Throws an exception derived from std::exception if there is an error in parsing
          * the file.
          */
         void check_loaded();
         
      private:
         /**
          * Reads the device description information from the specified XML structure.
          */
         void read_xml(Xml::Element &root);

         /**
          * Reads the header of the device description.
          */
         void read_header(std::istream &input);
      };
   };
};


#endif
