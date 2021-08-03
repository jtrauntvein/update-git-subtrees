/* Cora.Tasks.Defs.h

   Copyright (C) 2012, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 27 April 2012
   Last Change: Wednesday 30 April 2014
   Last Commit: $Date: 2019-11-19 11:23:09 -0600 (Tue, 19 Nov 2019) $
   Last Changed by: $Author: amortenson $

*/

#ifndef Cora_Tasks_Defs_h
#define Cora_Tasks_Defs_h

#include "CsiTypeDefs.h"


namespace Cora
{
   namespace Tasks
   {
      ////////////////////////////////////////////////////////////
      // namespace Messages
      //
      // Defines the message identifiers for all messages exchanged in the
      // tasks interface.
      ////////////////////////////////////////////////////////////
      namespace Messages
      {
         enum Type
         {
            // tasks enumerate transaction
            tasks_enum_start_cmd = 1100,
            tasks_enum_not = 1101,
            tasks_enum_stop_cmd = 1102,
            tasks_enum_stopped_not = 1103,

            // tasks add task transaction
            add_task_cmd = 1104,
            add_task_ack = 1105,

            // tasks remove task transaction
            remove_task_cmd = 1106,
            remove_task_ack = 1107,

            // enumerate task settings transaction
            enum_settings_start_cmd = 1110,
            enum_settings_not = 1111,
            enum_settings_stop_cmd = 1112,
            enum_settings_stopped_not = 1113,

            // set task settings transction
            set_settings_cmd = 1114,
            set_settings_ack = 1115,

            // enable transaction
            enable_cmd = 1118,
            enable_ack = 1119,

            // trigger task
            trigger_task_cmd = 1121,
            trigger_task_ack = 1122,

            // announce access level
            announce_access_level = 1123
         };
      };


      ////////////////////////////////////////////////////////////
      // namespace Settings
      //
      // Defines all of the setting identifiers for a task.
      ////////////////////////////////////////////////////////////
      namespace Settings
      {
         enum setting_id_type
         {
            trigger_type = 1,
            task_name = 2,
            station_name = 3,
            task_to_follow = 4,
            schedule_base = 5,
            schedule_interval = 6,
            cron_minutes = 7,
            cron_hours = 8,
            cron_days = 9,
            cron_months = 10,
            cron_days_of_week = 11,
            poll_station_enabled = 12,
            station_to_poll = 13,
            ftp_send_files_enabled = 14,
            ftp_server_address = 15,
            ftp_user_name = 16,
            ftp_password = 17,
            ftp_use_sftp = 18,
            ftp_protocol = 42,
            ftp_queue_size = 19,
            ftp_remote_dir = 39,
            ftp_enable_epsv = 43,
            exec_program_enabled = 20,
            exec_program_path = 21,
            exec_run_dir = 22,
            exec_command_line = 23,
            exec_timeout = 37,
            exec_run_minimised = 38,
            status_last_time_triggered = 24,
            status_next_expected_trigger = 25,
            status_actions_pending = 26,
            status_poll_last_started = 27,
            status_poll_last_finished = 28,
            status_poll_last_outcome = 29,
            status_ftp_last_started = 30,
            status_ftp_last_finished = 31,
            status_ftp_last_outcome = 32,
            status_exec_last_started = 33,
            status_exec_last_finished = 34,
            status_exec_last_outcome = 35,
            status_exec_last_exit_code = 36,
            success_expression = 40,
            trigger_delay = 41

            // next task ID should be 44
         };


         ////////////////////////////////////////////////////////////
         // enum status_poll_outcome_type
         ////////////////////////////////////////////////////////////
         enum status_poll_outcome_type
         {
            status_poll_outcome_not_polled = 1,
            status_poll_outcome_succeeded = 2,
            status_poll_outcome_permission_denied = 3,
            status_poll_outcome_comms_failed = 4,
            status_poll_outcome_comms_disabled = 5,
            status_poll_outcome_invalid_table_defs = 6,
            status_poll_outcome_task_disabled = 7,
            status_poll_outcome_logger_locked = 8,
            status_poll_outcome_file_write_failed = 9,
            status_poll_outcome_invalid_device = 10
         };


         ////////////////////////////////////////////////////////////
         // enum status_exec_outcome_type
         //
         // Defines the possible outcomes for the status-exec-last-outcome setting. 
         ////////////////////////////////////////////////////////////
         enum status_exec_outcome_type
         {
            status_exec_outcome_not_run = 1,
            status_exec_outcome_run = 2,
            status_exec_outcome_run_failed = 3,
            status_exec_outcome_timed_out = 4
         };

         ////////////////////////////////////////////////////////////
         // enum ftp_protocol_type
         ////////////////////////////////////////////////////////////
         enum ftp_protocol_type
         {
            ftp_protocol_basic = 1,
            ftp_protocol_ssh = 2,
            ftp_protocol_tls = 3
         };


         ////////////////////////////////////////////////////////////
         // setting_id_to_str
         //
         // Translates a task setting identifier to a string.  I the identifier is not recognised,
         // an empty string will be returned.
         ////////////////////////////////////////////////////////////
         char const *setting_id_to_string(uint4 setting_id);

         ////////////////////////////////////////////////////////////
         // str_to_setting_id
         //
         // Translates the specified string into a setting identifier.  If the
         // string is not recognised, a std::invalid_argument exception will be thrown.
         ////////////////////////////////////////////////////////////
         setting_id_type str_to_setting_id(char const *setting_name);
      };


      namespace Triggers
      {
         enum trigger_type
         {
            trigger_after_scheduled = 1,
            trigger_successful_poll = 2,
            trigger_failed_poll = 3,
            trigger_any_poll = 4,
            trigger_poll_with_some_data = 5,
            trigger_callback_begins = 6,
            trigger_callback_ends = 7,
            trigger_retry_poll_failed = 8,
            trigger_switch_to_secondary = 9,
            trigger_one_way_data = 10,
            trigger_data_file_closed = 11,
            trigger_after_task = 12,
            trigger_on_schedule = 13,
            trigger_on_cron = 14,
            trigger_after_any_data_collected = 15,
            trigger_after_file_synch = 16,
            max_triggers_count
         };

         ////////////////////////////////////////////////////////////
         // trigger_type_to_str
         //
         // Translates a trigger type to a string. If the identifier is not recognised,
         // an empty string will be returned.
         ////////////////////////////////////////////////////////////
         char const *trigger_type_to_str(uint4 trigger_type_id);

         ////////////////////////////////////////////////////////////
         // str_to_trigger_type
         //
         // Translates the specified string into a trigger type. If the
         // string is not recognised, a std::invalid_argument exception will be thrown.
         ////////////////////////////////////////////////////////////
         trigger_type str_to_trigger_type(char const *s);
      };
   };
};


#endif

