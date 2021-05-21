/* coratools.strings.h

   Copyright (C) 2007, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 29 November 2007
   Last Change: Friday 21 May 2021
   Last Commit: $Date: 2020-12-17 12:40:37 -0600 (Thu, 17 Dec 2020) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef coratools_strings_h
#define coratools_strings_h

#include "Csi.StringLoader.h"
#include "LoggerNetBuild.h"

#define CORASTRINGS_NAME    "coratools"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // namespace UtilsStrings
   //
   // Definitions for the strings used in the Csi.Utils module.
   //////////////////////////////////////////////////////////// 
   namespace UtilsStrings
   {
      ////////////////////////////////////////////////////////////
      // my_strings
      ////////////////////////////////////////////////////////////
      enum my_string_ids
      {
         strid_file_read_error = 1,
         strid_file_write_error,
         strid_demo_remaining_days,
         strid_unable_to_create_dir,
         strid_not_a_dir,
         strid_temp_file_failed,
         strid_demo_expired,
         strid_demo_not_installed,
         strid_extract_xml_header_failed,
         strid_append_invalid_environment_count,
         strid_append_invalid_file_type,
         strid_append_station_names_dont_match,
         strid_append_table_names_dont_match,
         strid_append_number_columns_dont_match,
         strid_append_column_names_dont_match,
         strid_append_model_numbers_dont_match,
         strid_append_serial_numbers_dont_match,
         strid_append_os_versions_dont_match,
         strid_append_program_names_dont_match,
         strid_append_program_sigs_dont_match,
         strid_append_invalid_units_count,
         strid_append_column_units_dont_match,
         strid_append_process_count_dont_match,
         strid_append_invalid_process_count,
         strid_append_units_count_dont_match,
         strid_append_column_process_dont_match,
         strid_append_data_types_count_dont_match,
         strid_append_data_types_dont_match,
         strid_append_invalid_target_header,
         strid_append_formats_dont_match,
         strid_append_too_large
      };
      extern Csi::LocalStringLoader my_strings;
   };


   ////////////////////////////////////////////////////////////
   // namespace ChallengeResponderStrings
   ////////////////////////////////////////////////////////////
   namespace ChallengeResponderStrings
   {
      enum my_ids
      {
         strid_outcome_success = 1,
         strid_outcome_timed_out_on_challenge,
         strid_outcome_timed_out_on_response,
         strid_outcome_invalid_challenge,
         strid_outcome_invalid_response,
         strid_outcome_unknown
      };
      extern Csi::LocalStringLoader my_strings;
   };

   namespace SignedJsonStrings
   {
      extern Csi::LocalStringLoader my_strings;
   };


   namespace SocketAddressStrings
   {
      enum my_ids
      {
         strid_validate_empty = 1,
         strid_validate_ipv4_complete,
         strid_validate_ipv6_complete,
         strid_validate_ipv4_incomplete,
         strid_validate_ipv6_incomplete,
         strid_validate_error
      };
      extern Csi::LocalStringLoader my_strings;
   };

   
   namespace SoftwareLicenseStrings
   {
      enum my_string_ids
      {
         strid_renewal_outcome_failure_unknown = 1,
         strid_renewal_outcome_success,
         strid_renewal_outcome_failure_unknown_product,
         strid_renewal_outcome_failure_disabled,
         strid_renewal_outcome_failure_communication,
         strid_license_key_malformed,
         strid_license_key_bad_sig,
         strid_register_addon_outcome_failure_unknown,
         strid_register_addon_outcome_success,
         strid_register_addon_outcome_failure_no_parent_product,
         strid_register_addon_outcome_failure_child_already_added,
         strid_register_addon_outcome_failure_no_child_product,
         strid_register_addon_outcome_failure_disabled,
         strid_register_addon_outcome_failure_communication,
         strid_license_product,
         strid_license_version,
         strid_license_serial_no,
         strid_license_activation_date,
         strid_license_renewal_status,
         strid_license_certificate_is_current,
         strid_license_expires_final,
         strid_license_renewal_not_required,
         strid_license_sub_licenses,
         strid_check_version_outcome_failure_unknown,
         strid_check_version_outcome_success,
         strid_check_version_outcome_failure_communication,
         strid_update_outcome_failure_unknown,
         strid_update_outcome_success,
         strid_update_outcome_failure_no_product,
         strid_update_outcome_failure_license_expired,
         strid_update_outcome_failure_license_disabled,
         strid_update_outcome_failure_needs_maintenance,
         strid_update_outcome_failure_maintenance_expired,
         strid_update_outcome_failure_aborted,
         strid_update_outcome_failure_communication,
         strid_changelog_outcome_failure_unknown,
         strid_changelog_outcome_success,
         strid_changelog_outcome_failure_no_product,
         strid_changelog_outcome_failure_communication,
         strid_changelog_version,
         strid_changelog_build,
         strid_changelog_change,
         strid_renewal_outcome_failure_trial_expired
      };
      extern Csi::LocalStringLoader my_strings;
   };
   

   namespace DevConfig
   {
      namespace SettingsManagerStrings
      {
         enum my_ids
         {
            strid_load_unknown_device_type = 1,
            strid_load_unknown_catalog,
            strid_load_comms,
            strid_load_security,
            strid_load_locked,
            strid_load_unknown,
            strid_commit_security,
            strid_commit_comms,
            strid_commit_locked,
            strid_commit_setting_not_recognised,
            strid_commit_setting_malformed,
            strid_commit_reserved,
            strid_commit_out_of_memory,
            strid_commit_system_error,
            strid_commit_setting_read_only,
            strid_default_apply_message,
            strid_load_invalid_device_desc
         };
         extern LocalStringLoader my_strings;
      };


      namespace FileSenderStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1,
            strid_outcome_invalid_file_name,
            strid_outcome_link_failed,
            strid_outcome_timed_out,
            strid_outcome_invalid_fragment_len,
            strid_outcome_unknown,
            strid_outcome_corrupt,
            strid_outcome_incompatible
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace SummaryConverterBaseStrings
      {
         enum my_string_ids
         {
            strid_no_source_device_type = 1,
            strid_no_conversion_supported,
            strid_rf401_incompatible_sdc_address
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace SettingStrings
      {
         enum my_string_ids
         {
            strid_too_many_values = 1,
            strid_invalid_char_prefix,
            strid_prefix_not_found,
            strid_postfix_not_found,
            strid_invalid_component_name,
            strid_cannot_copy_unrelated,
            strid_protected_value
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace ConfigSummaryStrings
      {
         enum my_string_ids
         {
            strid_load_out_of_range = 1,
            strid_load_failed
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace SettingPollerStrings
      {
         enum my_string_ids
         {
            strid_outcome_unknown = 1,
            strid_outcome_success,
            strid_outcome_link_failed,
            strid_outcome_timed_out,
            strid_outcome_partials_not_supported
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace SettingComp
      {
         namespace TlsKeyStrings
         {
            ////////////////////////////////////////////////////////////
            // my_strings
            ////////////////////////////////////////////////////////////
            enum my_string_ids
            {
               strid_invalid_pem_format = 1,
               strid_missing_encryption_spec,
               strid_invalid_encryption_spec,
               strid_invalid_pem_type
            };
            extern Csi::LocalStringLoader my_strings;
         };


         namespace TlsCertificateStrings
         {
            ////////////////////////////////////////////////////////////
            // my_strings
            ////////////////////////////////////////////////////////////
            enum my_string_ids
            {
               strid_begin_cert_without_end = 1,
               strid_invalid_pem_format,
               strid_not_pem_encoded
            };
            extern Csi::LocalStringLoader my_strings;
         };
      }; 
   };


   namespace Win32
   {
      namespace SerialPortBaseStrings
      {
         enum my_string_ids
         {
            strid_get_comm_state_failed = 1,
            strid_set_comm_state_failed,
            strid_overlapped_write_failed,
            strid_write_failed,
            strid_serial_open_failed,
            strid_write_timed_out,
            strid_read_failed,
            strid_wait_comm_event_failed,
            strid_overlapped_read_failed,
            strid_wait_failed,
            strid_get_comm_timeouts_failed,
            strid_set_comm_timeouts_failed,
            strid_get_modem_status_failed,
            strid_set_comm_mask_failed,
            strid_clear_comm_errors_failed
         };
         extern Csi::LocalStringLoader my_strings;
      };
   };


   namespace OsLoader
   {
      namespace OsLoaderBaseStrings
      {
         enum my_string_ids
         {
            strid_low_level_error = 1
         };
         extern Csi::LocalStringLoader my_strings;
      };

      
      namespace SimpleSRecordStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         //////////////////////////////////////////////////////////// 
         enum my_string_ids
         {
            strid_invalid_open_state = 1,
            strid_unable_to_open_file,
            strid_invalid_srecord,
            strid_no_valid_srecords,
            strid_invalid_start_state,
            strid_cancelled,
            strid_no_srecord_ack,
            strid_srecord_ack,
            strid_retrying,
            strid_too_many_retries,
            strid_complete
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace LargeSRecordLoaderStrings
      {
        enum my_string_ids
         {
            strid_none = 0,
            strid_invalid_open_state,
            strid_unable_to_open_file,
            strid_invalid_srecord,
            strid_no_valid_srecords,
            strid_invalid_termination,
            strid_no_device_type,
            strid_invalid_start_state,
            strid_waiting_for_type,
            strid_cancelled,
            strid_already_ended,
            strid_no_device_type_received,
            strid_no_srecord_ack,
            strid_srecord_ack,
            strid_sent,
            strid_eot_received,
            strid_nak_received,
            strid_too_many_retries,
            strid_wont_boot_warning,
            strid_wait_for_flash_erase,
            strid_no_ack_after_flash_erase,
            strid_sending_reset,
            strid_usb_waiting_for_type,
            strid_xon_not_received
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace CsosLoaderStrings
      {
         enum string_id_types
         {
            strid_none = 0,
            strid_invalid_loader_state = 1,
            strid_invalid_obj_length,
            strid_invalid_segment_sig,
            strid_waiting_for_dev_code1,
            strid_waiting_for_dev_code2,
            strid_invalid_send_state,
            strid_send_cancelled,
            strid_already_cancelled,
            strid_expected_synch_failed,
            strid_no_device_response,
            strid_retrying_fragment1,
            strid_retrying_fragment2,
            strid_fragment_sig_failed,
            strid_finished1,
            strid_finished2,
            strid_finished3,
            strid_sending1,
            strid_sending2,
            strid_last_fragment_sent,
            strid_invalid_sig_received,
            strid_wont_boot_warning,
            strid_no_sig_received
         }; 
         extern Csi::LocalStringLoader my_strings;
      };


      namespace DevconfigLoaderStrings
      {
         enum string_ids
         {
            strid_fragment_sent = 1,
            strid_send_complete,
            strid_send_failed
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace CstermLoaderStrings
      {
         enum my_string_ids
         {
            strid_none = 0,
            strid_invalid_start_state,
            strid_unable_to_open_input,
            strid_waiting_for_synch,
            strid_send_cancelled,
            strid_no_term_prompt,
            strid_no_boot_prompt,
            strid_no_contact,
            strid_no_synch,
            strid_no_response_after_reset,
            strid_no_response_after_last,
            strid_entering_boot_mode,
            strid_getting_synch,
            strid_received_synch,
            strid_waiting_for_flash_erase,
            strid_no_response_after_flash_erase,
            strid_invalid_file_size,
            strid_waiting_for_reset,
            strid_starting_send,
            strid_sending,
            strid_sent1,
            strid_sent2,
            strid_sent3,
            strid_invalid_sig_received,
            strid_no_devconfig_response,
            strid_devconfig_security_wrong,
            strid_already_cancelled,
            strid_current_block_too_big,
            strid_invalid_boot_sig,
            strid_invalid_os_sig,
            strid_invalid_filter_sig,
            strid_unrecognised_device_type,
            strid_waiting_after,
            strid_no_boot_response,
            strid_wont_boot_warning
         };
         extern Csi::LocalStringLoader my_strings;
      };
   };


   namespace Graphics
   {
      /**
       * Defines the strings associated with the graphing component.
       */
      enum my_string_ids
      {
         strid_restore = 1
      };
      extern Csi::LocalStringLoader my_strings;
   };
};


namespace Cora
{
   namespace ClientBaseStrings
   {
      ////////////////////////////////////////////////////////////
      // my_strings
      ////////////////////////////////////////////////////////////
      enum my_string_ids
      {
         strid_failure_unknown = 1,
         strid_failure_logon,
         strid_failure_session,
         strid_failure_unsupported,
         strid_failure_security,
         strid_failure_invalid_access,
         strid_failure_access_expired
      };
      extern Csi::LocalStringLoader my_strings;
   };


   namespace SettingStrings
   {
      enum my_string_ids
      {
         strid_outcome_no_attempt_made = 1,
         strid_outcome_set,
         strid_outcome_read_only,
         strid_outcome_locked,
         strid_outcome_invalid_value,
         strid_outcome_unsupported,
         strid_outcome_network_locked
      };
      extern Csi::LocalStringLoader my_strings;
   };


   namespace Device
   {
      ////////////////////////////////////////////////////////////
      // common_strings
      ////////////////////////////////////////////////////////////
      enum common_string_ids
      {
         common_success = 1,
         common_comm_failed,
         common_comm_disabled,
         common_logger_security_blocked,
         common_table_defs_invalid,
         common_aborted,
         common_logger_locked,
         common_no_tables,
         common_file_io_failed,
         common_invalid_collect_area_name,
         common_device_shut_down,
         common_invalid_file_name,
         common_network_locked
      };
      extern Csi::LocalStringLoader common_strings;

      
      namespace DeviceBaseStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         enum my_string_ids
         {
            strid_invalid_device_name = 1
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace SettingsOverriderStrings
      {
         enum my_string_ids
         {
            strid_another_in_progress = 1
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace TableResetterStrings
      {
         enum my_string_ids
         {
            strid_table_reset = 1,
            strid_communication_failed,
            strid_communication_disabled,
            strid_invalid_table_name,
            strid_logger_security_blocked
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace ProgramFileSenderStrings
      {
         ////////////////////////////////////////////////////////////
         // enum my_string_ids
         ////////////////////////////////////////////////////////////
         enum my_string_ids
         {
            strid_success = 1,
            strid_other_in_progress,
            strid_invalid_program_name,
            strid_communication_failed,
            strid_logger_compile_error,
            strid_cannot_open_file,
            strid_network_locked,
            strid_aborted,
            strid_table_defs_failed ,
            strid_server_resource_error,
            strid_logger_security_failed,
            strid_logger_buffer_full,
            strid_logger_file_inaccessible,
            strid_logger_root_dir_full,
            strid_logger_incompatible
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace FilesSynchroniserStrings
      {
         enum my_string_ids
         {
            strid_status_getting_dir = 1,
            strid_status_file_already_retrieved,
            strid_status_starting_retrieve,
            strid_status_retrieve_failed,
            strid_status_file_skipped,
            strid_status_file_retrieved,
            strid_outcome_success,
            strid_outcome_comm_failed,
            strid_outcome_comm_disabled,
            strid_outcome_invalid_logger_security
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace LinkTimeResetterStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace ClassicASenderStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1,
            strid_outcome_comm_failed,
            strid_outcome_comm_disabled
         };
         extern Csi::LocalStringLoader my_strings; 
      };


      namespace FileControllerStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1,
            strid_outcome_logger_communication_disabled,
            strid_outcome_logger_communication_failed,
            strid_outcome_logger_security_blocked,
            strid_outcome_insufficient_logger_resources,
            strid_outcome_invalid_file_name,
            strid_outcome_unsupported_command,
            strid_outcome_logger_locked,
            strid_outcome_network_locked,
            strid_outcome_logger_root_dir_full,
            strid_outcome_logger_file_busy,
            strid_outcome_logger_drive_busy
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace FileSenderStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1,
            strid_outcome_communication_disabled,
            strid_outcome_missing_file_name,
            strid_outcome_invalid_file_name,
            strid_outcome_logger_resource_error,
            strid_outcome_logger_compile_error,
            strid_outcome_communication_failed,
            strid_outcome_logger_permission_denied,
            strid_outcome_network_locked,
            strid_outcome_logger_root_dir_full,
            strid_outcome_logger_incompatible
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace CommResourceManagerStrings
      {
         ////////////////////////////////////////////////////////////
         // enum my_string_ids
         ////////////////////////////////////////////////////////////
         enum my_string_ids
         {
            strid_device_shut_down = 1,
            strid_communication_disabled,
            strid_unsupported,
            strid_unreachable,
            strid_max_time_online
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace LoggerQueryStrings
      {
         enum my_string_ids
         {
            strid_logger_security_blocked = 1,
            strid_communication_failure,
            strid_communication_disabled,
            strid_invalid_table_name,
            strid_invalid_table_definition,
            strid_insufficient_resources
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace LoggerQueryExStrings
      {
         enum my_string_ids
         {
            strid_stopped_outcome_failure_ack_timed_out = 1
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace LoggerQueryFileStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1,
            strid_outcome_file_open_failed,
            strid_outcome_query_interrupted,
            strid_outcome_cannot_append,
            strid_outcome_file_io_failed,
            strid_status_temporary_table_created,
            strid_status_some_data_collected,
            strid_status_all_data_collected,
            strid_status_file_mark_created
         };
         extern Csi::LocalStringLoader my_strings;
      };


      ////////////////////////////////////////////////////////////
      // namespace ClockSetterStrings
      ////////////////////////////////////////////////////////////
      namespace ClockSetterStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         enum my_string_ids
         {
            strid_clock_checked = 1,
            strid_clock_set,
            strid_communication_failed,
            strid_communication_disabled,
            strid_logger_security_blocked
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace VariableSetterStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         enum my_string_ids
         {
            strid_outcome_succeeded = 1,
            strid_outcome_column_read_only,
            strid_outcome_invalid_table_name,
            strid_outcome_invalid_column_name,
            strid_outcome_invalid_subscript,
            strid_outcome_invalid_data_type,
            strid_outcome_communication_failed,
            strid_outcome_communication_disabled,
            strid_outcome_logger_security_blocked,
            strid_outcome_invalid_table_defs,
            strid_outcome_invalid_device_name
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace VariableGetterStrings
      {
         enum my_string_ids
         {
            strid_invalid_table_name = 1,
            strid_invalid_column_name,
            strid_invalid_subscript
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace ProgramStatsGetterStrings
      {
         enum my_string_ids
         {
            strid_no_program_running = 1,
            strid_program_running,
            strid_program_compile_error,
            strid_program_stopped,
            strid_program_status
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace TableDefsRefresherStrings
      {
         enum my_string_ids
         {
            strid_outcome_invalid_table_name = 1,
            strid_outcome_in_progress,
            strid_outcome_network_locked
         };
         extern Csi::LocalStringLoader my_strings;
      };
   };

   
   namespace Broker
   {
      namespace BrokerBaseStrings
      {
         enum my_ids
         {
            strid_invalid_id = 1
         };
         extern Csi::LocalStringLoader my_strings;
      };
      
      
      namespace DataAdvisorStrings
      {
         enum my_ids
         {
            strid_invalid_table_name = 1,
            strid_invalid_start_option,
            strid_invalid_order_option,
            strid_table_deleted,
            strid_station_shut_down,
            strid_invalid_column_name,
            strid_invalid_array_address
         };
         extern Csi::LocalStringLoader my_strings;
      };


      ////////////////////////////////////////////////////////////
      // namespace FormattedDataAdvisorStrings
      ////////////////////////////////////////////////////////////
      namespace FormattedDataAdvisorStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         enum  my_ids
         {
            strid_invalid_start_option = 1,
            strid_invalid_order_option,
            strid_invalid_format_option,
            strid_table_deleted,
            strid_station_shut_down
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace DataQueryStrings
      {
         enum my_string_ids
         {
            strid_success = 1,
            strid_invalid_table_name,
            strid_invalid_range
         };
         extern Csi::LocalStringLoader my_strings;
      };
   };


   namespace LgrNet
   {
      namespace AccessTokenGetterStrings
      {
         enum my_string_ids
         {
            strid_success = 1,
            strid_failure_invalid_refresh,
            strid_failure_refresh_expired,
            strid_failure_no_account
         };
         extern Csi::LocalStringLoader my_strings;
      };

      
      namespace SettingsSetterStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1,
            strid_outcome_some_errors,
            strid_outcome_locked
         };
         extern Csi::LocalStringLoader my_strings;
      };

      
      namespace NetworkLockerStrings
      {
         enum my_string_ids
         {
            strid_failure_already_locked = 1,
            strid_failure_already_locked_no_user
         };
         extern Csi::LocalStringLoader my_strings;
      };


      /**
       * Defines the strings for the device renamer component.
       */
      namespace DeviceRenamerStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1,
            strid_outcome_failure_invalid_device_name,
            strid_outcome_failure_device_online,
            strid_outcome_failure_locked
         };
         extern Csi::LocalStringLoader my_strings;
      };

      
      ////////////////////////////////////////////////////////////
      // BranchMoverStrings
      ////////////////////////////////////////////////////////////
      namespace BranchMoverStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         enum my_string_ids
         {
            strid_device_not_found = 1,
            strid_anchor_not_found,
            strid_unattachable,
            strid_pending_transaction,
            strid_network_locked
         };
         extern Csi::LocalStringLoader my_strings;
      };

      
      ////////////////////////////////////////////////////////////
      // namespace LogsClearerStrings
      ////////////////////////////////////////////////////////////
      namespace LogsClearerStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         enum my_ids
         {
            strid_outcome_success = 1,
            strid_outcome_clear_failed
         };
         extern Csi::LocalStringLoader my_strings;
      };


      ////////////////////////////////////////////////////////////
      // namespace LogsZipperStrings
      ////////////////////////////////////////////////////////////
      namespace LogsZipperStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         enum my_ids
         {
            strid_outcome_success = 1,
            strid_outcome_create_failed
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace FileRetrieverStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         enum my_ids
         {
            strid_outcome_success = 1,
            strid_invalid_file_name,
            strid_server_file_open_failed ,
            strid_local_file_open_failed,
            strid_invalid_server_file_name,
            strid_invalid_local_file_name
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace XmlNetworkMapChangerStrings
      {
         ////////////////////////////////////////////////////////////
         // my_strings
         ////////////////////////////////////////////////////////////
         enum my_string_ids
         {
            strid_add_device_description = 1,
            strid_anchor_as_child,
            strid_anchor_before,
            strid_anchor_after,
            strid_move_branch_description,
            strid_set_device_settings_description,
            strid_setting_id,
            strid_setting_value,
            strid_failure_network_locked,
            strid_failure_unrecognised_change,
            strid_failure_invalid_device_name,
            strid_failure_unattachable,
            strid_failure_unsupported_device_type,
            strid_failure_invalid_setting,
            strid_invalid_change,
            strid_set_lgrnet_settings_description,
            strid_add_modem_description,
            strid_change_modem_description,
            strid_failure_invalid_modem_name,
            strid_failure_modem_read_only,
            strid_failure_invalid_modem_strings
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace DeviceAdderStrings
      {
         enum my_string_ids
         {
            strid_outcome_invalid_device_name = 1,
            strid_outcome_unattachable,
            strid_outcome_unsupported_device_type,
            strid_outcome_network_locked,
            strid_outcome_too_many_stations
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace DeviceDeleterStrings
      {
         enum my_string_ids
         {
            strid_outcome_invalid_device_name = 1,
            strid_outcome_device_online,
            strid_outcome_network_locked
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace SnapshotRestorerStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1,
            strid_outcome_partial_success,
            strid_outcome_invalid_file_name,
            strid_outcome_invalid_snapshot_version,
            strid_outcome_corrupt_snapshot,
            strid_outcome_other_transactions,
            strid_outcome_network_locked
         };
         extern Csi::LocalStringLoader my_strings;
      };

      namespace BackupCreatorStrings
      {
         enum mu_string_ids
         {
            strid_outcome_invalid_file_name = 1,
            strid_outcome_no_resources
         };
         extern Csi::LocalStringLoader my_strings;
      };

      namespace LogAdvisorStrings
      {
         enum my_string_ids
         {
            strid_server_cancelled = 1
         };
         extern Csi::LocalStringLoader my_strings;
      };

      namespace ReplicationLoginStrings
      {
         enum my_string_ids
         {
            strid_outcome_failure_cloud_comms = 1,
            strid_outcome_failure_cloud
         };
         extern Csi::LocalStringLoader my_strings;
      };
   };
   

   namespace DataSources
   {
      namespace SinkBaseStrings
      {
         enum my_string_ids
         {
            strid_failure_unknown = 1,
            strid_failure_invalid_source,
            strid_failure_connection_failed,
            strid_failure_invalid_logon,
            strid_failure_invalid_station_name,
            strid_failure_invalid_table_name,
            strid_failure_server_security,
            strid_failure_invalid_start_option,
            strid_failure_invalid_order_option,
            strid_failure_table_deleted,
            strid_failure_station_shut_down,
            strid_failure_unsupported,
            strid_failure_invalid_column_name,
            strid_failure_invalid_array_address
         };
         extern Csi::LocalStringLoader my_strings;
      };
   };


   namespace Sec2
   {
      namespace AccountChangerStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1,
            strid_outcome_failure_locked,
            strid_outcome_failure_invalid_account_name
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace EnablerStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1,
            strid_outcome_no_root_account,
            strid_outcome_locked,
            strid_outcome_not_admin
         };
         extern Csi::LocalStringLoader my_strings;
      };


      namespace AccountDeleterStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1,
            strid_outcome_locked,
            strid_outcome_invalid_account_name,
            strid_outcome_account_in_use
         };
         extern Csi::LocalStringLoader my_strings;
      };
   };

   
   namespace Tasks
   {
      namespace TaskAdderStrings
      {
         enum my_string_ids
         {
            strid_outcome_success = 1,
            strid_outcome_server_locked,
            strid_outcome_invalid_task_id,
            strid_outcome_task_found,
            strid_outcome_setting_set,
            strid_outcome_invalid_setting_id,
            strid_outcome_invalid_setting_value,
            strid_outcome_setting_read_only
         };
         extern Csi::LocalStringLoader my_strings;
      };
   };

   namespace PbRouter
   {
      namespace PbRouterBaseStrings
      {
         enum my_string_ids
         {
            strid_invalid_id = 1
         };
         extern Csi::LocalStringLoader my_strings;
      };
   };
};


#endif
