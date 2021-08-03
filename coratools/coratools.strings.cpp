/* coratools.strings.cpp

   Copyright (C) 2007, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 29 November 2007
   Last Change: Tuesday 15 December 2020
   Last Commit: $Date: 2020-12-17 12:40:37 -0600 (Thu, 17 Dec 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "coratools.strings.h"
#include "Csi.SignedJson.h"


namespace Csi
{
   namespace UtilsStrings
   {
      ////////////////////////////////////////////////////////////
      // my_strings
      ////////////////////////////////////////////////////////////
      Csi::LocalStringLoader::init_type my_init[] =
      {
         { strid_file_read_error,
           "File read error" },
         { strid_file_write_error,
           "File write error" },
         { strid_unable_to_create_dir,
           "Unable to create %1% directory",
           "%1% is the directory path" },
         { strid_not_a_dir,
           "%1% is not a directory" },
         { strid_temp_file_failed,
           "temporary file creation failed" },
         { strid_demo_remaining_days,
           "You have %1% day(s) left to try %2% %3%.\n"
           "Contact Campbell Scientific to purchase a registered copy.",
           "%1% is the number of days remaining\n"
           "%2% is the application name\n"
           "%3% is the version" },
         { strid_demo_expired,
           "Your trial version of %1% %2% has expired.\n"
           "Contact Campbell Scientific to purchase a registered copy.",
           "%1% is the application name\n"
           "%2% is the application version" },
         { strid_demo_not_installed,
           "%1% %2% has not been installed correctly.\n"
           "Reinstall or contact Campbell Scientific to purchase a registered copy.",
           "%1% is the application name\n"
           "%2% is the application version" },
         { strid_extract_xml_header_failed,
           "Unable to parse the XML header in the file" },
         { strid_append_invalid_environment_count,
           "invalid number of fields in the environment line" },
         { strid_append_invalid_file_type,
           "invalid file type specified" },
         { strid_append_station_names_dont_match,
           "the station names don't match" },
         { strid_append_table_names_dont_match,
           "the table names don't match" },
         { strid_append_number_columns_dont_match,
           "the number of columns doesn't match" },
         { strid_append_column_names_dont_match,
           "some column names don't match" },
         { strid_append_model_numbers_dont_match,
           "the model numbers don't match" },
         { strid_append_serial_numbers_dont_match,
           "the serial numbers don't match" },
         { strid_append_os_versions_dont_match,
           "the OS versions don't match" },
         { strid_append_program_names_dont_match,
           "the program names don't match" },
         { strid_append_program_sigs_dont_match,
           "the program signatures don't match" },
         { strid_append_units_count_dont_match,
           "the number of units strings doesn't match" },
         { strid_append_invalid_units_count,
           "the number of units strings doesn't match the columns count" },
         { strid_append_column_units_dont_match,
           "the units for field %1% don't match" },
         { strid_append_process_count_dont_match,
           "the number of process strings doesn't match" },
         { strid_append_invalid_process_count,
           "the number of process strings doesn't match the columns count" },
         { strid_append_column_process_dont_match,
           "the process strings for field %1% don't match" },
         { strid_append_data_types_count_dont_match,
           "the number of data types doesn't match" },
         { strid_append_data_types_dont_match,
           "one or more data types don't match" },
         { strid_append_invalid_target_header,
           "invalid target header" },
         { strid_append_formats_dont_match,
           "the existing format, %1%, is not the same as the expected format, %2%" },
         { strid_append_too_large,
           "the file is larger than %1% bytes" },
         { 0, 0 }
      };
      Csi::LocalStringLoader my_strings(
         CORASTRINGS_NAME,
         CORASTRINGS_VERSION,
         "Csi::UtilsStrings",
         my_init);
   };


   namespace SignedJsonStrings
   {
      Csi::LocalStringLoader::init_type my_init[] =
      {
         { Csi::signed_json_failure_malformed, "Malformed envelope" },
         { Csi::signed_json_failure_unsupported_algorithm, "Unsupported signature algorithm" },
         { Csi::signed_json_failure_invalid_signature, "Invalid signed envelope signature" },
         { 0, 0 }
      };
      Csi::LocalStringLoader my_strings(
         CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::SignedJsonStrings", my_init);
   };
   

   namespace ChallengeResponderStrings
   {
      Csi::LocalStringLoader::init_type my_init[] =
      {
         { strid_outcome_success, "authentication succeeded" },
         { strid_outcome_timed_out_on_challenge, "authentication timed out waiting for the challenge" },
         { strid_outcome_timed_out_on_response, "authentication timed out waiting for the response" },
         { strid_outcome_invalid_challenge, "an invalid challenge authentication was received" },
         { strid_outcome_invalid_response, "an invalid response authentication was received" },
         { strid_outcome_unknown, "unrecognized authentication error " },
         { 0, 0 }
      };
      Csi::LocalStringLoader my_strings(
         CORASTRINGS_NAME,
         CORASTRINGS_VERSION,
         "Csi::ChallengeResponderStrings",
         my_init);
   };


   namespace SoftwareLicenseStrings
   {
      Csi::LocalStringLoader::init_type my_strings_init[] = {
         { strid_renewal_outcome_failure_unknown, "unrecognised failure condition" },
         { strid_renewal_outcome_success, "license renewed" },
         { strid_renewal_outcome_failure_unknown_product, "the software product is not registered" },
         { strid_renewal_outcome_failure_disabled, "the software product license has been disabled" },
         { strid_renewal_outcome_failure_trial_expired, "the software product license trial period has expired" },
         { strid_renewal_outcome_failure_communication, "unable to communicate with the license server" },
         { strid_license_key_malformed, "invalid license key format" },
         { strid_license_key_bad_sig, "invalid license key signature" },
         { strid_register_addon_outcome_failure_unknown, "unrecognised failure condition" },
         { strid_register_addon_outcome_success, "sub-license was added" },
         { strid_register_addon_outcome_failure_no_parent_product, "the parent product license has not been registered" },
         { strid_register_addon_outcome_failure_child_already_added, "the child product has already been registered with another license" },
         { strid_register_addon_outcome_failure_no_child_product, "the child product model is not registered" },
         { strid_register_addon_outcome_failure_disabled, "the parent license has been disabled" },
         { strid_register_addon_outcome_failure_communication, "unable to communicate with the license server" },
         { strid_license_product, "Model" },
         { strid_license_version, "Version" },
         { strid_license_serial_no, "Serial Number" },
         { strid_license_activation_date, "Activation Date" },
         { strid_license_renewal_status, "Renewal Status" },
         { strid_license_certificate_is_current, "<font color='blue'>certificate is current.</font>" },
         { strid_license_expires_final, "<font color='red'>certificate must be renewed before %n.</font>" },
         { strid_license_renewal_not_required, "renewal is not required" },
         { strid_license_sub_licenses, "Registered Sub-Licenses" },
         { strid_check_version_outcome_failure_unknown, "unrecognised failure condition" },
         { strid_check_version_outcome_success, "versions retrieved from the license server" },
         { strid_check_version_outcome_failure_communication, "unable to communicate with the license server" },
         { strid_update_outcome_failure_unknown, "unrecognised update failure condition" },
         { strid_update_outcome_success, "update was downloaded" },
         { strid_update_outcome_failure_no_product, "no product update available" },
         { strid_update_outcome_failure_license_expired, "product license has expired" },
         { strid_update_outcome_failure_license_disabled, "product license has been disabled" },
         { strid_update_outcome_failure_needs_maintenance, "product license needs a maintenance agreement" },
         { strid_update_outcome_failure_maintenance_expired, "product license maintenance agreement has expired" },
         { strid_update_outcome_failure_aborted, "product update was aborted" },
         { strid_update_outcome_failure_communication, "unable to communicate with the license server" },
         { strid_changelog_outcome_failure_unknown, "unrecognised changelog failure condition" },
         { strid_changelog_outcome_success, "change log retrieved" },
         { strid_changelog_outcome_failure_no_product, "invalid model or version" },
         { strid_changelog_outcome_failure_communication, "unable to communicate with the license server" },
         { strid_changelog_version, "Version %1%" },
         { strid_changelog_build, "Build %1% (%2%)" },
         { strid_changelog_change, "(%1%) %2%" },
         { 0, 0 }
      };
      Csi::LocalStringLoader my_strings(
         CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::SoftwareLicense", my_strings_init);
   };


   namespace SocketAddressStrings
   {
      Csi::LocalStringLoader::init_type my_init[] =
      {
         { strid_validate_empty, "address is not set" },
         { strid_validate_ipv4_complete, "address is set to IPv4" },
         { strid_validate_ipv4_incomplete, "address incomplete for IPv4" },
         { strid_validate_ipv6_complete, "address is set to IPv6" },
         { strid_validate_ipv6_incomplete, "address incomplete for IPv6" },
         { strid_validate_error, "invalid address syntax" },
         { 0, 0 }
      };
      Csi::LocalStringLoader my_strings(
         CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::SocketAddress", my_init);
   };

   
   namespace DevConfig
   {
      namespace SettingsManagerStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_load_unknown_device_type,
              "The device type was not recognized.\n"
              "You may need to obtain a newer version of the Device Configuration Utility from\n"
              "Campbell Scientific." },
            { strid_load_unknown_catalog,
              "The major version reported by the device cannot be found in the device description.\n"
              "You may need to obtain a newer version of the Device Configuration Utility from\n"
              "Campbell Scientific." },
            { strid_load_comms, "Communication failed while loading settings" },
            { strid_load_locked, "The device is locked by another settings client" },
            { strid_load_unknown, "An unrecognized error occurred" },
            { strid_load_security, "Invalid security code to load settings" },
            { strid_load_invalid_device_desc, "Cannot read settings from the device description file" },
            { strid_commit_security, "Invalid security code to commit settings" },
            { strid_commit_comms, "Communications error occurred while committing settings" },
            { strid_commit_locked, "The device is locked by another settings client" },
            { strid_commit_setting_not_recognised,
              "Setting ID %1%, %2%, was not recognised by the device." },
            { strid_commit_setting_malformed,
              "Setting ID %1%, %2%, was reported as malformed or out of range." },
            { strid_commit_out_of_memory, "The device does not have enough memory to store a setting" },
            { strid_commit_system_error,
              "The device is unable to commit setting changes because of an internal system error" },
            { strid_commit_setting_read_only,
              "Setting ID %1%, %2%, has been reported as read only." },
            { strid_default_apply_message,
              "<html>\n"
              "<head>\n"
              "<title>Confirm Settings Apply</title>\n"
              "</head>\n"
              "<body>\n"
              "<p>We are about to apply setting changes to the device.  This will force the device to reset\n"
              "and can result in changes in the way that the device behaves.</p>\n"
              "<p>Are you sure that you want to apply these changes?</p>\n"
              "</body>\n"
              "</html>\n" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Csi::DevConfig::SettingsManager",
            my_init);
      };


      namespace FileSenderStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_outcome_success, "success" },
            { strid_outcome_invalid_file_name, "invalid file name specified" },
            { strid_outcome_link_failed, "the link failed" },
            { strid_outcome_timed_out, "timed out waiting for a response" },
            { strid_outcome_invalid_fragment_len, "a fragment was too long or had an invalid length" },
            { strid_outcome_corrupt, "the file content appears to be corrupt" },
            { strid_outcome_unknown, "an unrecognized error occured" },
            { strid_outcome_incompatible, "the file content is incompatible wih the device" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::DevConfig::FileSender", my_init);
      };


      namespace SummaryConverterBaseStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_no_source_device_type, "Unable to locate the source device type" },
            { strid_no_conversion_supported,
              "There is no conversion defined between major version %1% and major version %2% for this device.",
              "%1% is the source major version and %2% is the destination major version" },
            { strid_rf401_incompatible_sdc_address, "The summary is using an incompatible SDC address for this OS" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::DevConfig::SummaryConverterBase", my_init);
      };


      namespace SettingStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_too_many_values,
              "Too many values specified for the %1% setting." },
            { strid_invalid_char_prefix,
              "An invalid character, \'%1%\', was found while searching for the prefix for %2%." },
            { strid_prefix_not_found,
              "Expected the prefix, \"%1%\", before %2%" },
            { strid_postfix_not_found,
              "Expected the postfix, \"%1%\", following the \"%2%\" component." },
            { strid_invalid_component_name,
              "Invalid component name" },
            { strid_cannot_copy_unrelated,
              "Can't copy unrelated settings" },
            { strid_protected_value, "value is protected" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::DevConfig::Setting", my_init);
      };


      namespace ConfigSummaryStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_load_out_of_range, "The setting, %1%, has a value that is out of range" },
            { strid_load_failed, "Failed to read a setting: %1%" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::DevConfig::ConfigSummary", my_init);
      };


      namespace SettingPollerStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_outcome_unknown, "unrecognised setting poll failure" },
            { strid_outcome_success, "settings poll succeeded" },
            { strid_outcome_link_failed, "the link failed while polling settings" },
            { strid_outcome_timed_out, "commands timed out while polling settings" },
            { strid_outcome_partials_not_supported, "settings poll protocol error" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::DevConfig::SettingPoller", my_init);
      };

      namespace SettingComp
      {
         namespace TlsKeyStrings
         {
            Csi::LocalStringLoader::init_type my_init[] =
            {
               { strid_invalid_pem_format, "the PEM file has missing footer" },
               { strid_missing_encryption_spec, "the PEM file is missing the encryption specification" },
               { strid_invalid_encryption_spec, "the encryption specified is not supported by this device" },
               { strid_invalid_pem_type, "an RSA private key PEM file not specified" },
               { 0, 0 }
            };
            Csi::LocalStringLoader my_strings(
               CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::DevConfig::SettingComp::TlsKey", my_init);
         };


         namespace TlsCertificateStrings
         {
            Csi::LocalStringLoader::init_type my_init[] =
            {
               { strid_begin_cert_without_end, "a certificate was begun but not finished" },
               { strid_invalid_pem_format, "wrong type of PEM file" },
               { strid_not_pem_encoded, "certificates must be PEM encoded" },
               { 0, 0 }
            };
            Csi::LocalStringLoader my_strings(
               CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::DevConfig::SettingComp::TlsCertificate", my_init);
         };
      };
   };


   namespace OsLoader
   {
      namespace OsLoaderBaseStrings
      {
         LocalStringLoader::init_type my_initialisors[] =
         {
            { strid_low_level_error,
              "OS Download Low Level Error: ",
              "Error message when the low level interface fails" },
            { 0, 0 }
         };
         LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::OsLoader::OsLoaderBase", my_initialisors);
      };

      
      namespace SimpleSRecordStrings
      {
         LocalStringLoader::init_type my_string_initialisors[] =
         {
            { strid_invalid_open_state,
              "Loader object is in an invalid state to be opened" },
            { strid_unable_to_open_file,
              "Unable to open the OS image file" },
            { strid_invalid_srecord,
              "Invalid S-Record on line %1%",
              "%1% is the line number" },
            { strid_no_valid_srecords,
              "No valid s-records in the OS image file" },
            { strid_invalid_start_state,
              "Cannot start the OS send because it has already ben started" },
            { strid_cancelled,
              "The send operation was cancelled" },
            { strid_no_srecord_ack,
              "No response received from the last fragment sent." },
            { strid_srecord_ack,
              "Sent %1% of %2% fragments",
              "%1% is the number of records sent, %2% is the total number to send"},
            { strid_retrying,
              "Retrying fragment %1%: %2%",
              "%1% is the fragment number, %2% is the number of times its been retried" },
            { strid_too_many_retries,
              "Aborted due to too many retries on fragment %1%.",
              "%1% is the fragment number" },
            { strid_complete,
              "%1% has been sent." },
            { 0, 0 }
         };
         LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::OsLoader::SimpleSRecord", my_string_initialisors);
      };


      namespace LargeSRecordLoaderStrings
      {
         Csi::LocalStringLoader::init_type my_initialisors[] =
         {
            { strid_invalid_open_state,
              "LargeSRecordLoader object is in invalid state to be opened" },
            { strid_unable_to_open_file,
              "Unable to open the file" },
            { strid_invalid_srecord,
              "Invalid s-record in line " },
            { strid_no_valid_srecords,
              "No valid s-records in the file" },
            { strid_invalid_termination,
              "The last s-record does not terminate the file" },
            { strid_no_device_type,
              "Unable to locate the device type" },
            { strid_invalid_start_state,
              "LargeSRecordLoader is in an invalid state to be started" },
            { strid_waiting_for_type,
              "Waiting for up to 60 seconds to receive the type code from a %1%\n\n"
              "If the device is powered on, reboot it by toggling its power." },
            { strid_usb_waiting_for_type,
              "Waiting for up to 60 seconds to receive the type code from a %1%\n\n"
              "Do not disconnect the USB cable as this will break the USB link." },
            { strid_cancelled,
              "Cancelled by the user" },
            { strid_already_ended,
              "Loader has already ended" },
            { strid_no_device_type_received,
              "Timed out waiting for the device type echo" },
            { strid_no_srecord_ack,
              "Timed out while waiting for an s-record ack" },
            { strid_srecord_ack, "Record %1% acknowledged" },
            { strid_sent,
              "<p>The OS, %1%, has been sent.</p>\n"
              "<p>Its signature is %2%</p>" },
            { strid_eot_received,
              "EOT received before the last record was sent" },
            { strid_nak_received, "NAK received for record %1%, retry = %2%" },
            { strid_too_many_retries,
              "Too many retries for record " },
            { strid_wont_boot_warning,
              "The whole operating system could not be sent.  The device "
              "will probably not boot after this.  In order to recover, "
              "the operating system must be re-sent." },
            { strid_wait_for_flash_erase, "Waiting for the device to erase flash" },
            { strid_no_ack_after_flash_erase,
              "No ACK was received from the device to indicate that the flash erase\n"
              "was completed." },
            { strid_sending_reset,
              "Transmitting a devconfig reset command to the device\n" },
            { strid_xon_not_received, "XON not received after the device sent XON.\n" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::OsLoader::LargeSRecordLoader", my_initialisors);
      };


      namespace CsosLoaderStrings
      {
         Csi::LocalStringLoader::init_type const my_initialisors[] =
         {
            { strid_invalid_loader_state,
              "Loader is in an invalid state" },
            { strid_invalid_obj_length,
              "Invalid OBJ file length" },
            { strid_invalid_segment_sig,
              "Invalid segment signature at offset" },
            { strid_waiting_for_dev_code1,
              "Waiting for device code from a " },
            { strid_waiting_for_dev_code2,
              "Turn off the power on the device and then turn it back on." },
            { strid_invalid_send_state,
              "State is inappropriate for sending" },
            { strid_send_cancelled,
              "Send was cancelled by the user" },
            { strid_already_cancelled,
              "Already stopped" },
            { strid_expected_synch_failed,
              "Failed to receive the expected synch" },
            { strid_no_device_response,
              "The device failed to respond" },
            { strid_retrying_fragment1,
              "retrying fragment " },
            { strid_retrying_fragment2,
              " after a timeout." },
            { strid_fragment_sig_failed,
              "Failed to receive the fragment signature" },
            { strid_finished1,
              "<p><b>The OS has been sent.</b></p>"
              "<p>The file signature is " },
            { strid_finished2,
              "</p><p>The datalogger will now reboot and test its memory.\n"
              "This test must not be interrupted.  Establishing\n"
              "communication with the\n" },
            { strid_finished3,
              " before this test is complete can lead to unpredictable results.</p><br>"
              "<p>The time required for the datalogger to reboot can\n"
              "be determined as follows:</p>"
              "<table border=\"1\">\n"
              "  <tr>\n"
              "    <td><b>Datalogger</b></td>\n"
              "    <td><b>Reboot Time</b></td>\n"
              "  </tr>\n"
              "  <tr><td>CR10X</td> <td>35 seconds</td></tr>\n"
              "  <tr><td>CR10X-2M</td><td>8 minutes</td></tr>\n"
              "  <tr><td>CR510</td><td>35 seconds</td></tr>\n"
              "  <tr><td>CR510-2M</td><td>8 minutes</td></tr>\n"
              "  <tr><td>CR23X</td><td>8 minutes</td></tr>\n"
              "  <tr><td>CR23x-4M</td><td>16 minutes</td></tr>\n"
              "</table>" },
            { strid_sending1,
              "Sending fragment " },
            { strid_sending2,
              " of " },
            { strid_last_fragment_sent,
              "The last fragment has been sent" },
            { strid_invalid_sig_received,
              "Invalid signature received for packet " },
            { strid_wont_boot_warning,
              "The whole operating system could not be sent.  The device "
              "will probably not boot after this.  In order to recover, "
              "the operating system must be re-sent." },
            { strid_no_sig_received,
              "Timed out waiting for a signature from the last packet." },
            { 0, 0 }
         }; 
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::OsLoader::CsosLoader", my_initialisors);
      };


      namespace DevconfigLoaderStrings
      {
         Csi::LocalStringLoader::init_type my_string_inits[] =
         {
            { strid_fragment_sent, "Sent %1% of %2% bytes" },
            { strid_send_complete,
              "The operating system has been sent.\n"
              "It may take up to %1% seconds in order for the device to reboot.\n"
              "Do not power down the device during this interval." },
            { strid_send_failed, "Send failed: %1%" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::OsLoader::DevconfigLoader", my_string_inits);
      };


      namespace CstermLoaderStrings
      {
         Csi::LocalStringLoader::init_type my_initialisors[] =
         {
            { strid_invalid_start_state,
              "CstermLoader is in an invalid state to be started",
              "Text associated with the exception thrown if the application attempts "
              "to start the OS loader when its has already been started."},
            { strid_unable_to_open_input,
              "Unable to open the input file" },
            { strid_waiting_for_synch,
              "Waiting for the synch byte.  Make sure the device is powered" },
            { strid_send_cancelled,
              "The transmission was cancelled" },
            { strid_no_term_prompt,
              "Failed to get the terminal prompt" },
            { strid_no_boot_prompt,
              "Failed to receive the boot prompt" },
            { strid_no_contact,
              "Failed to contact the device" },
            { strid_no_synch,
              "No synch response received from the device" },
            { strid_no_response_after_flash_erase,
              "No response received after the erase flash sequence" },
            { strid_no_response_after_reset,
              "No response received after the device reset" },
            { strid_no_response_after_last,
              "No response received for the last fragment." },
            { strid_entering_boot_mode,
              "Received the terminal prompt.  Attempting to enter boot mode..." },
            { strid_getting_synch,
              "Got the boot prompt.  Getting the synch byte" },
            { strid_received_synch,
              "Received the synch byte" },
            { strid_waiting_for_flash_erase,
              "Waiting for flash erase" },
            { strid_invalid_file_size,
              "Invalid input file size" },
            { strid_waiting_for_reset,
              "Waiting the the device reset" },
            { strid_starting_send,
              "Flash is erased, starting the send sequence" },
            { strid_sending,
              "Sending fragment at offset " },
            { strid_sent1,
              "<p>The operating system file, " },
            { strid_sent2,
              ", has been sent</p>\n" },
            { strid_sent3,
              "<p>Its signature is " },
            { strid_invalid_sig_received,
              "Invalid fragment signature received." },  
            { strid_no_devconfig_response,
              "No response received from the reset command" },
            { strid_devconfig_security_wrong,
              "The security code provided is wrong" },
            { strid_already_cancelled,
              "Cancellation has already begun" },
            { strid_current_block_too_big,
              "One of the blocks in the OS image is marked larger than the available space",
              "strid_current_block_too_big" },
            { strid_invalid_boot_sig,
              "The boot segment signature is invalid",
              "strid_invalid_boot_sig" },
            { strid_invalid_os_sig,
              "The signature of the operating system is invalid",
              "strid_invalid_os_sig" },
            { strid_invalid_filter_sig,
              "The signature of one of the filter module images is invalid",
              "strid_invalid_filter_sig" },
            { strid_unrecognised_device_type,
              "Unable to recognize the device type code given in the image",
              "strid_unrecognised_device_type" },
            { strid_waiting_after,
              "Waiting for the device to write critical flash sections",
              "strid_waiting_after" },
            { strid_no_boot_response,
              "No response from the device after reboot.  You might need to cycle "
              "power on the device in order for it to reboot properly.",
              "strid_no_boot_response" },
            { strid_wont_boot_warning,
              "The whole operating system  could not be sent.  The device "
              "will probably not boot after this.  In order "
              "to recover, the operating system must be re-sent." },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::OsLoader::CstermLoader", my_initialisors);
      };
   };


   namespace Graphics
   {
      Csi::LocalStringLoader::init_type my_init[] =
      {
         { strid_restore, "Restore" },
            { 0, 0 }
      };
      Csi::LocalStringLoader my_strings(
         CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::Graphics::Graph", my_init);
   };
   

   namespace Win32
   {
      namespace SerialPortBaseStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_get_comm_state_failed, "failed to get the comm state" },
            { strid_set_comm_state_failed, "failed to set the comm state" },
            { strid_overlapped_write_failed, "overlapped write failed" },
            { strid_write_failed, "write failed" },
            { strid_serial_open_failed, "serial open failed" },
            { strid_write_timed_out, "write timed out" },
            { strid_read_failed, "read failed" },
            { strid_wait_comm_event_failed, "wait comm event failed" },
            { strid_overlapped_read_failed, "overlapped read failed" },
            { strid_wait_failed, "wait failed" },
            { strid_get_comm_timeouts_failed, "get comm timeouts failed" },
            { strid_set_comm_timeouts_failed, "set comm timeouts failed" },
            { strid_get_modem_status_failed, "get modem status failed" },
            { strid_set_comm_mask_failed, "set comm mask failed" },
            { strid_clear_comm_errors_failed, "clear comm errors failed" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Csi::Win32::SerialPortBase", my_init);
      };
   };
};


namespace Cora
{
   namespace ClientBaseStrings
   {
      ////////////////////////////////////////////////////////////
      // my_strings
      ////////////////////////////////////////////////////////////
      Csi::LocalStringLoader::init_type my_string_init[] =
      {
         { strid_failure_unknown,
           "An unrecognized failure has occurred" },
         { strid_failure_logon,
           "Invalid logon information supplied to the server" },
         { strid_failure_session,
           "Communication with the server has been lost" },
         { strid_failure_security,
           "The LoggerNet account does not have enough access" },
         { strid_failure_unsupported,
           "Attempted to use an unsupported transaction" },
         { strid_failure_invalid_access,
           "Attempted to log in using an invalid access token" },
         { strid_failure_access_expired,
           "The access token has expired" },
         { 0, 0 }
      };
      Csi::LocalStringLoader my_strings(
         CORASTRINGS_NAME,
         CORASTRINGS_VERSION,
         "Cora::ClientBase",
         my_string_init);
   };


   namespace SettingStrings
   {
      Csi::LocalStringLoader::init_type my_strings_init[] = {
         { strid_outcome_no_attempt_made, "no attempt made" },
         { strid_outcome_set, "set" },
         { strid_outcome_read_only, "setting is read-only" },
         { strid_outcome_locked, "setting is locked" },
         { strid_outcome_invalid_value, "invalid setting value" },
         { strid_outcome_unsupported, "setting is not supported" },
         { strid_outcome_network_locked, "network is locked" },
         { 0, 0 }
      };
      Csi::LocalStringLoader my_strings(
         CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Setting", my_strings_init);
   };

   
   namespace Device
   {
      ////////////////////////////////////////////////////////////
      // common_strings
      ////////////////////////////////////////////////////////////
      Csi::LocalStringLoader::init_type common_init[] =
      {
         { common_success, "success" },
         { common_comm_failed, "communication failed" },
         { common_comm_disabled, "communication is disabled" },
         { common_logger_security_blocked, "blocked by datalogger security" },
         { common_table_defs_invalid, "the server has invalid table definitions for the datalogger" },
         { common_aborted, "aborted because of user action" },
         { common_logger_locked, "the datalogger is locked for another operation" },
         { common_no_tables, "the server has no tables for this datalogger" },
         { common_file_io_failed, "file I/O failed" },
         { common_invalid_collect_area_name, "an invalid collect area name was specified" },
         { common_device_shut_down, "the device was shut down" },
         { common_invalid_file_name, "an invalid file name was specified" },
         { common_network_locked, "network is locked" },
         { 0, 0 }
      };
      Csi::LocalStringLoader common_strings(
         CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Device::CommonStrings", common_init);

      
      namespace DeviceBaseStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         Csi::LocalStringLoader::init_type my_string_initialisors[] =
         {
            { strid_invalid_device_name, "Specified device is not defined" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Cora::Device::DeviceBase",
            my_string_initialisors);
      };


      namespace SettingsOverriderStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         Csi::LocalStringLoader::init_type my_string_initialisors[] =
         {
            { strid_another_in_progress,
              "Another override transaction is already active for the device" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Cora::Device::SettingsOverrider",
            my_string_initialisors);
      };


      namespace TableResetterStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_table_reset, "The table was reset" },
            { strid_communication_failed, "communication failed" },
            { strid_communication_disabled, "communication is disabled" },
            { strid_invalid_table_name, "the specified table does not exist" },
            { strid_logger_security_blocked, "invalid datalogger security code" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Cora::Device::TableResetter",
            my_init);
      };


      namespace ProgramFileSenderStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_success, "succeeded" },
            { strid_other_in_progress,
              "another transaction is already in progress for this station" },
            { strid_invalid_program_name,
              "the program name specified is invalid" },
            { strid_communication_failed,
              "communication with the datalogger failed" },
            { strid_logger_compile_error,
              "the datalogger failed to compile the program" },
            { strid_cannot_open_file,
              "the specified file cannot be opened" },
            { strid_network_locked,
              "the network is locked by another client" },
            { strid_aborted,
              "the send attempt was aborted" },
            { strid_table_defs_failed,
              "failed to get the datalogger table definitions" },
            { strid_server_resource_error,
              "the server does not have enough resources to buffer the program file" },
            { strid_logger_security_failed,
              "the security code specified does not give rights to send the file" },
            { strid_logger_buffer_full,
              "there is not enough space on the logger to store or buffer the file" },
            { strid_logger_file_inaccessible,
              "the datalogger file is inaccessible" },
            { strid_logger_root_dir_full, "the datalogger root directory is full" },
            { strid_logger_incompatible, "the file is incompatible with the datalogger" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Cora::Device::ProgramFileSender",
            my_init);
      };


      namespace FilesSynchroniserStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_status_getting_dir, "reading the datalogger directory" },
            { strid_status_file_already_retrieved, "\"%1%\" - already retrieved" },
            { strid_status_starting_retrieve, "\"%1%\" - retrieving" },
            { strid_status_retrieve_failed, "\"%1%\" - failed to retrieve (the file has probably been deleted)" },
            { strid_status_file_skipped, "\"%1%\" - skipping (too many files)" },
            { strid_status_file_retrieved, "\"%1%\" - retrieved" },
            { strid_outcome_success, "synch files complete" },
            { strid_outcome_comm_failed, "communication failed while synchronising files" },
            { strid_outcome_comm_disabled, "communication is disabled" },
            { strid_outcome_invalid_logger_security, "the server has the wrong security code for this logger" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Cora::Device::FilesSynchroniser",
            my_init);
      };


      namespace LinkTimeResetterStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_outcome_success, "link up time reset for the device" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Cora::Device::LinkTimeResetter",
            my_init); 
      };


      namespace ClassicASenderStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_outcome_success, "success" },
            { strid_outcome_comm_failed, "communication failed" },
            { strid_outcome_comm_disabled, "communication is disabled" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Cora::Device::ClassicASender",
            my_init);
      };


      namespace FileControllerStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_outcome_success, "success" },
            { strid_outcome_logger_communication_disabled, "datalogger communication is disabled" },
            { strid_outcome_logger_communication_failed, "datalogger communication failed" },
            { strid_outcome_logger_security_blocked, "blocked by datalogger security or the datalogger is busy with the file or file system" },
            { strid_outcome_insufficient_logger_resources, "insufficient datalogger resources" },
            { strid_outcome_invalid_file_name, "invalid file name" },
            { strid_outcome_unsupported_command, "unsupported file control command" },
            { strid_outcome_logger_locked, "loggernet datalogger locked" },
            { strid_outcome_network_locked, "loggernet network map is locked" },
            { strid_outcome_logger_root_dir_full, "the datalogger root directory is full" },
            { strid_outcome_logger_file_busy, "the file cannot be deleted because the datalogger program has it open" },
            { strid_outcome_logger_drive_busy, "the device cannot be formatted because the datalogger has one or more files open" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Device::FileController", my_init);
      };


      namespace FileSenderStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_outcome_success, "success" },
            { strid_outcome_communication_disabled, "datalogger communication is disabled" },
            { strid_outcome_missing_file_name, "missing the destination file name" },
            { strid_outcome_invalid_file_name, "invalid file name" },
            { strid_outcome_logger_resource_error, "datalogger resource error" },
            { strid_outcome_logger_compile_error, "datalogger compile failure" },
            { strid_outcome_communication_failed, "datalogger communication failed" },
            { strid_outcome_logger_permission_denied, "datalogger permission denied" },
            { strid_outcome_network_locked, "the network map is locked" },
            { strid_outcome_logger_root_dir_full, "the datalogger root directory is full" },
            { strid_outcome_logger_incompatible, "the file is incompatible with the datalogger" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Device::FileSender", my_init);
      };


      namespace CommResourceManagerStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         Csi::LocalStringLoader::init_type my_strings_init[] =
         {
            { strid_device_shut_down, "the device has been shut down" },
            { strid_communication_disabled, "communication has been disabled" },
            { strid_unsupported, "this transaction is not supported for the device" },
            { strid_unreachable, "the device cannot be reached" },
            { strid_max_time_online, "maximum time on-line has been exceeded" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Device::CommResourceManager", my_strings_init);
      };


      namespace LoggerQueryStrings
      {
         Csi::LocalStringLoader::init_type my_strings_init[] =
         {
            { strid_logger_security_blocked, "wrong datalogger security code" },
            { strid_communication_failure, "communication with the datalogger failed" },
            { strid_communication_disabled, "communication with the datalogger is disabled" },
            { strid_invalid_table_name, "the specified table name does not exist" },
            { strid_invalid_table_definition, "the logger table definitions have changed" },
            { strid_insufficient_resources, "there are not enough server resources for the query" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Device::LoggerQuery", my_strings_init);
      };


      namespace LoggerQueryExStrings
      {
         Csi::LocalStringLoader::init_type my_strings_init[] =
         {
            { strid_stopped_outcome_failure_ack_timed_out,
              "timed out waiting for client record acknowledgements" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Device::LoggerQueryEx", my_strings_init);
      };


      namespace LoggerQueryFileStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_outcome_success, "success" },
            { strid_outcome_file_open_failed, "file open failed" },
            { strid_outcome_query_interrupted, "the query was interrupted" },
            { strid_outcome_cannot_append, "cannot append to the file" },
            { strid_outcome_file_io_failed, "cannot write to the file" },
            { strid_status_temporary_table_created, "temporary table created" },
            { strid_status_some_data_collected, "some data collected" },
            { strid_status_all_data_collected, "all data collected" },
            { strid_status_file_mark_created, "file mark created" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Device::LoggerQueryFile", my_init);
      };


      namespace ClockSetterStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         Csi::LocalStringLoader::init_type my_strings_init[] =
         {
            { strid_clock_checked, "station clock checked" },
            { strid_clock_set, "station clock set" },
            { strid_communication_failed, "communication failed" },
            { strid_communication_disabled, "communication disabled" },
            { strid_logger_security_blocked, "blocked by station security" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Device::ClockSetter", my_strings_init);
      };


      namespace VariableSetterStrings
      {
         Csi::LocalStringLoader::init_type my_strings_init[] =
         {
            { strid_outcome_succeeded, "The variable was set" },
            { strid_outcome_column_read_only, "The variable is read-only" },
            { strid_outcome_invalid_table_name, "Invalid table name specified" },
            { strid_outcome_invalid_column_name, "Invalid column name specified" },
            { strid_outcome_invalid_subscript, "Invalid array subscript specified" },
            { strid_outcome_invalid_data_type, "Invalid column  data type or conversion error" },
            { strid_outcome_communication_failed, "Communication with the datalogger failed" },
            { strid_outcome_communication_disabled, "Communication with the datalogger is disabled" },
            { strid_outcome_logger_security_blocked, "Blocked by datalogger security" },
            { strid_outcome_invalid_table_defs, "The LoggerNet server has invalid table definitions" },
            { strid_outcome_invalid_device_name, "An invalid device name was specified" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Device::VariableSetter", my_strings_init);
      };


      namespace VariableGetterStrings
      {
         Csi::LocalStringLoader::init_type my_strings_init[] =
         {
            { strid_invalid_table_name, "invalid table name" },
            { strid_invalid_column_name, "invalid field name" },
            { strid_invalid_subscript, "invalid array subscript" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Device::VariableGetterStrings", my_strings_init);
      };
      
      
      namespace ProgramStatsGetterStrings
      {
         Csi::LocalStringLoader::init_type my_strings_init[] =
         {
            { strid_no_program_running, "no program running" },
            { strid_program_running, "program running" },
            { strid_program_compile_error, "compile error" },
            { strid_program_stopped, "program stopped" },
            { strid_program_status,
              "Running Program: %1%\n"
              "Run on Power Up Program: %2%\n"
              "Program State: %3%\n"
              "Compile Time: %4%\n"
              "OS Version: %5%\n"
              "====================\n"
              "%6%" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Cora::Device::ProgramStatsGetter",
            my_strings_init);
      };


      namespace TableDefsRefresherStrings
      {
         Csi::LocalStringLoader::init_type my_strings_init[] =
         {
            { strid_outcome_invalid_table_name, "invalid table name" },
            { strid_outcome_in_progress, "already retrieving table definitions" },
            { strid_outcome_network_locked, "the network is locked" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Devcice::TableDefsRefresher", my_strings_init);
      };
   };


   namespace Broker
   {
      namespace BrokerBaseStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_invalid_id, "The specified data broker does not exist" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Cora::Broker::BrokerBase",
            my_init); 
      };


      namespace DataAdvisorStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_invalid_table_name, "The specified table does not exist" },
            { strid_invalid_start_option, "An invalid start option was specified" },
            { strid_invalid_order_option, "An invalid order option was specified" },
            { strid_table_deleted, "The table was deleted" },
            { strid_station_shut_down, "The data broker was shut down" },
            { strid_invalid_column_name, "An invalid column name was specified" },
            { strid_invalid_array_address, "An invalid column array address was specified" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Cora::Broker::DataAdvisor",
            my_init);
      };


      ////////////////////////////////////////////////////////////
      // namespace FormattedDataAdvisorStrings
      ////////////////////////////////////////////////////////////
      namespace FormattedDataAdvisorStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_invalid_start_option, "invalid start option" },
            { strid_invalid_order_option, "invalid order option" },
            { strid_invalid_format_option, "invalid format option" },
            { strid_table_deleted, "the table was deleted" },
            { strid_station_shut_down, "the station was shut down" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Broker::FormattedDataAdvisor", my_init);
      };


      namespace DataQueryStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_success, "success" },
            { strid_invalid_table_name, "invalid table name" },
            { strid_invalid_range, "invalid query range" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Broker::DataQuery", my_init);
      };
   };


   namespace LgrNet
   {
      namespace AccessTokenGetterStrings
      {
         Csi::LocalStringLoader::init_type my_strings_init[] = {
            { strid_success, "success" },
            { strid_failure_invalid_refresh, "invalid refresh token specified" },
            { strid_failure_refresh_expired, "refresh token has expired" },
            { strid_failure_no_account, "no account available to generate the access token" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::LgrNet::AccessTokenGetter", my_strings_init);
      };

      
      namespace SettingsSetterStrings
      {
         Csi::LocalStringLoader::init_type my_strings_init[] = {
            { strid_outcome_success, "success" },
            { strid_outcome_some_errors, "errors with some settings" },
            { strid_outcome_locked, "the network is locked" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::LgrNet::SettingsSetter", my_strings_init);
      };

      
      namespace NetworkLockerStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_failure_already_locked,
              "The LoggerNet server has already been locked by '%1%' under the '%2%' application.",
              "%1% is the other user name, %2% is the other application" },
            { strid_failure_already_locked_no_user,
              "The LoggerNet server has already been locked under the '%1%' application." },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::LgrNet::NetworkLocker", my_init);
      };


      namespace DeviceRenamerStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_outcome_success, "success" },
            { strid_outcome_failure_invalid_device_name, "the device name or the new device name is invalid" },
            { strid_outcome_failure_device_online, "rename failed because the device is on-line" },
            { strid_outcome_failure_locked, "the network map is locked by another client" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::LgrNet::DeviceRenamer", my_init);
      };

      
      namespace BranchMoverStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_device_not_found, "invalid target device specified" },
            { strid_anchor_not_found, "invalid anchor device specified" },
            { strid_unattachable, "the target device cannot be attached as specified" },
            { strid_pending_transaction, "the network is busy" },
            { strid_network_locked, "the network has been locked by another client" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::LgrNet::BranchMover", my_init);
      };

      
      namespace LogsClearerStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_outcome_success, "Success" },
            { strid_outcome_clear_failed, "One or more log files could not be deleted" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Cora::LgrNet::LogsClearer",
            my_init);
      };


      namespace LogsZipperStrings
      {
         Csi::LocalStringLoader::init_type my_init[] =
         {
            { strid_outcome_success, "success" },
            { strid_outcome_create_failed, "ZIP file creation failed" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION,
            "Cora::LgrNet::LogsZipper",
            my_init);
      };


      namespace FileRetrieverStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         Csi::LocalStringLoader::init_type my_strings_init[] =
         {
            { strid_outcome_success, "The file was retrieved." },
            { strid_invalid_file_name, "The file does not exist for the server." },
            { strid_server_file_open_failed, "Failed to open the server file." },
            { strid_local_file_open_failed, "Failed to open the local file." },
            { strid_invalid_server_file_name, "No remote file name was specified." },
            { strid_invalid_local_file_name, "No local file name was specified." },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::LgrNet::FileRetriever", my_strings_init);
      };


      namespace XmlNetworkMapChangerStrings
      {
         Csi::LocalStringLoader::init_type my_string_initialisors[] =
         {
            { strid_add_device_description,
              "<font color=\"green\">Add device, %1%, of type %2% %3% %4%</font>",
              "%1% is the device name, %2% is the device type, %3% is the attachment mode,\n"
              "and %4% is the anchor name" },
            { strid_anchor_as_child, "as child to" },
            { strid_anchor_before, "before" },
            { strid_anchor_after, "after" },
            { strid_move_branch_description,
              "<font color=\"red\">Move device, %1%, %2% %3%</font>",
              "%1% is the device name, %2% is the anchor mode, %3% is the anchor name" },
            { strid_set_device_settings_description,
              "<font color=\"blue\">Change settings for device %1%</font>",
              "%1% is the device name" },
            { strid_setting_id, "Identifier" },
            { strid_setting_value, "Value" },
            { strid_invalid_change,
              "Failed to parse a change entry" },
            { strid_failure_unrecognised_change,
              "An invalid change element, '%1%', was specified.",
              "%1% is the name of the change element" },
            { strid_failure_invalid_device_name,
              "An invalid device name, '%1%', (duplicate or containing invalid characters) was specified",
              "%1% is the invalid name" },
            { strid_failure_unattachable,
              "The device, '%1%', cannot be attached %2%.",
              "%1% is the device name, %2% is the attachment mode." },
            { strid_failure_unsupported_device_type,
              "The device type specified for '%2%', %1% is not supported by the server.",
              "%1% is the device type, %2% is the device name" },
            { strid_failure_invalid_setting,
              "An invalid setting has been specified for device '%1%'.",
              "%1% is the device name" },
            { strid_set_lgrnet_settings_description,
              "<font color=\"blue\">Change server settings</font>" },
            { strid_add_modem_description,
              "<font color=\"green\">Add custom modem, \"%1%\", with reset, \"%2%\", and init, \"%3%\".</font>" },
            { strid_change_modem_description,
              "<font color=\"blue\">Change custom modem, \"%1%\", with reset, \"%2%\", and init, \"%3%\".</font>" },
            { strid_failure_invalid_modem_name,
              "A modem type name was invalid." },
            { strid_failure_modem_read_only,
              "Attempted to change the modem strings for a default modem." },
            { strid_failure_invalid_modem_strings,
              "The reset or init strings for a modem contained invalid characters." },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME,
            CORASTRINGS_VERSION, 
            "Cora::LgrNet::XmlNetworkMapChanger",
            my_string_initialisors);
      };


      namespace DeviceAdderStrings
      {
         Csi::LocalStringLoader::init_type const my_strings_init[] =
         {
            { strid_outcome_invalid_device_name, "an invalid device name was specified" },
            { strid_outcome_unattachable, "the new device could not be added at the specified location" },
            { strid_outcome_unsupported_device_type, "the specified device type is not supported by the server" },
            { strid_outcome_network_locked, "the network map is locked by another client" },
            { strid_outcome_too_many_stations, "the loggernet licensed stations limit would be exceeded" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::LgrNet::DeviceAdder", my_strings_init);
      };


      namespace DeviceDeleterStrings
      {
         Csi::LocalStringLoader::init_type const my_strings_init[] =
         {
            { strid_outcome_invalid_device_name, "an invalid device name was specified" }, 
            { strid_outcome_device_online, "cannot delete because one or more devices in the branch are on-line" },
            { strid_outcome_network_locked, "the network map is locked by another client" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::LgrNet::DeviceDeleter", my_strings_init);
      };


      namespace SnapshotRestorerStrings
      {
         Csi::LocalStringLoader::init_type const my_strings_init[] =
         {
            { strid_outcome_success, "success" },
            { strid_outcome_partial_success, "some, but not all, files restored" },
            { strid_outcome_invalid_file_name, "failed to open backup file" },
            { strid_outcome_invalid_snapshot_version, "unsupported backup file version" },
            { strid_outcome_corrupt_snapshot, "backup file is corrupt" },
            { strid_outcome_other_transactions, "the server is busy" },
            { strid_outcome_network_locked, "the network is locked" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::LgrNet::SnapshotRestorer", my_strings_init);
      };

      namespace BackupCreatorStrings
      {
         Csi::LocalStringLoader::init_type const my_strings_init[] =
         {
            {strid_outcome_invalid_file_name, "invalid file name"},
            {strid_outcome_no_resources, "no resources"},
            {0,0}
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::LgrNet::BackupCreator", my_strings_init);
      };

      namespace LogAdvisorStrings
      {
         Csi::LocalStringLoader::init_type const my_strings_init[] =
         {
            {strid_server_cancelled, "cancelled by server"},
            {0,0}
         };
         Csi::LocalStringLoader my_strings(CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::LgrNet::LogAdvisor", my_strings_init);
      };

      namespace ReplicationLoginStrings
      {
         Csi::LocalStringLoader::init_type const my_strings_init[] =
         {
            { strid_outcome_failure_cloud_comms, "unable to connect to the cloud service" },
            { strid_outcome_failure_cloud, "unable to get an access token from the cloud service" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::LgrNet::ReplicationLogin", my_strings_init);
      };
   };


   namespace DataSources
   {
      namespace SinkBaseStrings
      {
         Csi::LocalStringLoader::init_type const my_strings_init[] =
         {
            { strid_failure_unknown, "unrecognized failure occurred" },
            { strid_failure_invalid_source, "invalid source name" },
            { strid_failure_connection_failed, "the source connection failed" },
            { strid_failure_invalid_logon, "source logon failed" },
            { strid_failure_invalid_station_name, "invalid station name" },
            { strid_failure_invalid_table_name, "invalid table name" },
            { strid_failure_server_security, "blocked by server security" },
            { strid_failure_invalid_start_option, "invalid start option" },
            { strid_failure_invalid_order_option, "invalid order option" },
            { strid_failure_table_deleted, "the table was deleted" },
            { strid_failure_station_shut_down, "the station was shut down" },
            { strid_failure_unsupported, "unsupported operation" },
            { strid_failure_invalid_column_name, "invalid column name" },
            { strid_failure_invalid_array_address, "invalid array address" },
            { 0, 0} 
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::DataSources::SinkBase", my_strings_init);
      };
   };


   namespace Sec2
   {
      namespace AccountChangerStrings
      {
         Csi::LocalStringLoader::init_type const my_strings_init[] =
         {
            { strid_outcome_success, "success" },
            { strid_outcome_failure_locked, "the network is locked" },
            { strid_outcome_failure_invalid_account_name, "invalid account name" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Sec2::AccountChanger", my_strings_init);
      };


      namespace EnablerStrings
      {
         Csi::LocalStringLoader::init_type my_strings_init[] =
         {
            { strid_outcome_success, "success" },
            { strid_outcome_no_root_account, "no admin account is defined" },
            { strid_outcome_locked, "the network is locked" },
            { strid_outcome_not_admin, "admin privileges are required" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Sec2::Enabler", my_strings_init);
      };


      namespace AccountDeleterStrings
      {
         Csi::LocalStringLoader::init_type my_strings_init[] = {
            { strid_outcome_success, "success" },
            { strid_outcome_locked, "the network is locked" },
            { strid_outcome_invalid_account_name, "invalid account name" },
            { strid_outcome_account_in_use, "account is in use" },
            { 0, 0 }
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Sec2::Deleter", my_strings_init);
      };
   };

   
   namespace Tasks
   {
      namespace TaskAdderStrings
      {
         Csi::LocalStringLoader::init_type const my_strings_init[] =
         {
            { strid_outcome_success, "success" },
            { strid_outcome_server_locked, "the network map is locked by another client" },
            { strid_outcome_invalid_task_id, "invalid task identifier" },
            { strid_outcome_setting_set, "setting was changed" },
            { strid_outcome_invalid_setting_id, "unsupported setting ID" },
            { strid_outcome_invalid_setting_value, "invalid setting value" },
            { strid_outcome_setting_read_only, "the setting is read-only" },
            { strid_outcome_task_found, "the task was found" },
            { 0, 0 },
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, CORASTRINGS_VERSION, "Cora::Tasks::TaskAdder", my_strings_init);
      };
   };


   namespace PbRouter
   {
      namespace PbRouterBaseStrings
      {
         Csi::LocalStringLoader::init_type my_strings_init[] =
         {
            { strid_invalid_id, "The specified pakbus router does not exist" },
            { 0, 0 },
         };
         Csi::LocalStringLoader my_strings(
            CORASTRINGS_NAME, 
            CORASTRINGS_VERSION, 
            "Cora::PbRouter::PbRouterBase", 
            my_strings_init);
      }
   }
};

