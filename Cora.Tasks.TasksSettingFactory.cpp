/* Cora.Tasks.TasksSettingFactory.cpp

   Copyright (C) 2012, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 17 May 2012
   Last Change: Wednesday 30 April 2014
   Last Commit: $Date: 2014-04-30 14:44:25 -0600 (Wed, 30 Apr 2014) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Tasks.TasksSettingFactory.h"
#include "Cora.CommonSettingTypes.h"
#include "Cora.Tasks.Defs.h"


namespace Cora
{
   namespace Tasks
   {
      ////////////////////////////////////////////////////////////
      // class TriggerSetting
      ////////////////////////////////////////////////////////////
      class TriggerSetting: public SettingEnumeration
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructot
         ////////////////////////////////////////////////////////////
         TriggerSetting():
            SettingEnumeration(Settings::trigger_type)
         {
            supported_values[Triggers::trigger_after_scheduled] = "after-scheduled";
            supported_values[Triggers::trigger_successful_poll] = "sucessful-poll";
            supported_values[Triggers::trigger_failed_poll] = "failed-poll";
            supported_values[Triggers::trigger_any_poll] = "any-poll";
            supported_values[Triggers::trigger_poll_with_some_data] = "poll-with-some-data";
            supported_values[Triggers::trigger_callback_begins] = "callback-begins";
            supported_values[Triggers::trigger_callback_ends] = "callback-ends";
            supported_values[Triggers::trigger_retry_poll_failed] = "retry-poll-failed";
            supported_values[Triggers::trigger_switch_to_secondary] = "switch-to-secondary";
            supported_values[Triggers::trigger_one_way_data] = "one-way-data";
            supported_values[Triggers::trigger_data_file_closed] = "data-file-closed";
            supported_values[Triggers::trigger_after_task] = "after-task";
            supported_values[Triggers::trigger_on_schedule] = "on-schedule";
            supported_values[Triggers::trigger_on_cron] = "on-cron";
            supported_values[Triggers::trigger_after_any_data_collected] = "any-data-collected";
            supported_values[Triggers::trigger_after_file_synch] = "file-synch-complete"; 
         }
      };


      ////////////////////////////////////////////////////////////
      // class PollLastOutcomeSetting
      ////////////////////////////////////////////////////////////
      class PollLastOutcomeSetting: public SettingEnumeration
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         PollLastOutcomeSetting():
            SettingEnumeration(Settings::status_poll_last_outcome)
         {
            supported_values[Settings::status_poll_outcome_not_polled] = "not-polled";
            supported_values[Settings::status_poll_outcome_succeeded] = "success";
            supported_values[Settings::status_poll_outcome_permission_denied] = "failed-security";
            supported_values[Settings::status_poll_outcome_comms_failed] = "failed-comms";
            supported_values[Settings::status_poll_outcome_comms_disabled] = "failed-comms-disabled";
            supported_values[Settings::status_poll_outcome_invalid_table_defs] = "failed-invalid-table-defs";
            supported_values[Settings::status_poll_outcome_task_disabled] = "failed-disabled";
            supported_values[Settings::status_poll_outcome_logger_locked] = "failed-logger-locked";
            supported_values[Settings::status_poll_outcome_file_write_failed] = "failed-file-io";
            supported_values[Settings::status_poll_outcome_invalid_device] = "failed-invalid-device";
         }
      };


      ////////////////////////////////////////////////////////////
      // class StatusExecLastOutcomeSetting
      ////////////////////////////////////////////////////////////
      class StatusExecLastOutcomeSetting: public SettingEnumeration
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         StatusExecLastOutcomeSetting():
            SettingEnumeration(Settings::status_exec_last_outcome)
         {
            supported_values[Settings::status_exec_outcome_not_run] = "program-not-started";
            supported_values[Settings::status_exec_outcome_run] = "program-started";
            supported_values[Settings::status_exec_outcome_run_failed] = "no-program";
            supported_values[Settings::status_exec_outcome_timed_out] = "program-timed-out";
         }
      };


      ////////////////////////////////////////////////////////////
      // class FtpProtocolSetting
      ////////////////////////////////////////////////////////////
      class FtpProtocolSetting: public SettingEnumeration
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         FtpProtocolSetting():
            SettingEnumeration(Settings::ftp_protocol)
         {
            supported_values[Settings::ftp_protocol_basic] = "ftp-basic";
            supported_values[Settings::ftp_protocol_ssh] = "ftp-ssh";
            supported_values[Settings::ftp_protocol_tls] = "ftp-tls";
         }
      };
      
      
      ////////////////////////////////////////////////////////////
      // class TasksSettingFactory definitions
      ////////////////////////////////////////////////////////////
      Setting *TasksSettingFactory::make_setting(uint4 setting_id)
      {
         Setting *rtn(0);
         switch(setting_id)
         {
         case Settings::trigger_type:
            rtn = new TriggerSetting;
            break;
            
         case Settings::task_name:
         case Settings::station_name:
         case Settings::station_to_poll:
            rtn = new SettingStrUni(setting_id);
            break;

         case Settings::task_to_follow:
         case Settings::schedule_interval:
         case Settings::status_actions_pending:
         case Settings::exec_timeout:
         case Settings::ftp_queue_size:
         case Settings::trigger_delay:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::schedule_base:
         case Settings::status_last_time_triggered:
         case Settings::status_next_expected_trigger:
         case Settings::status_poll_last_started:
         case Settings::status_poll_last_finished:
         case Settings::status_ftp_last_started:
         case Settings::status_ftp_last_finished:
         case Settings::status_exec_last_started:
         case Settings::status_exec_last_finished:
            rtn = new TimeSetting(setting_id);
            break;

         case Settings::cron_minutes:
         case Settings::cron_hours:
         case Settings::cron_days:
         case Settings::cron_months:
         case Settings::cron_days_of_week:
            rtn = new SettingUInt4Set(setting_id);
            break;

         case Settings::poll_station_enabled:
         case Settings::ftp_send_files_enabled:
         case Settings::ftp_use_sftp:
         case Settings::exec_program_enabled:
         case Settings::exec_run_minimised:
         case Settings::ftp_enable_epsv:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::ftp_protocol:
            rtn = new FtpProtocolSetting;
            break;

         case Settings::ftp_password:
            rtn = new SettingAscPassword(setting_id);
            break;
            
         case Settings::ftp_server_address:
         case Settings::ftp_user_name:
         case Settings::exec_program_path:
         case Settings::exec_run_dir:
         case Settings::exec_command_line:
         case Settings::ftp_remote_dir:
         case Settings::success_expression:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Settings::status_poll_last_outcome:
            rtn = new PollLastOutcomeSetting;
            break;

         case Settings::status_ftp_last_outcome:
            rtn = new SettingInt4(setting_id); 
            break;

         case Settings::status_exec_last_outcome:
            rtn = new StatusExecLastOutcomeSetting;
            break;

         case Settings::status_exec_last_exit_code:
            rtn = new SettingInt4(setting_id);
            break;
         }
         return rtn;
      }
   };
};

