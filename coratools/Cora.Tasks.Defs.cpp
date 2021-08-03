/* Cora.Tasks.Defs.cpp

   Copyright (C) 2012, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 27 April 2012
   Last Change: Thursday 16 January 2020
   Last Commit: $Date: 2020-01-27 11:26:11 -0600 (Mon, 27 Jan 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Tasks.Defs.h"
#include "StrAsc.h"
#include <stdlib.h>


namespace Cora
{
   namespace Tasks
   {
      namespace Settings
      {
         namespace
         {
            struct setting_map_type
            {
               uint4 const id;
               char const *name;
            } setting_maps[] =
            {
               { trigger_type, "trigger-type" },
               { trigger_type, "triggerType" },
               { task_name, "task-name" },
               { task_name, "taskName" },
               { station_name, "station-name" },
               { station_name, "stationName" },
               { task_to_follow, "task-to-follow" },
               { task_to_follow, "taskToFollow" },
               { schedule_base, "schedule-base" },
               { schedule_base, "scheduleBase" },
               { schedule_interval, "schedule_interval" },
               { schedule_interval, "scheduleInterval" },
               { cron_minutes, "cron-minutes" },
               { cron_minutes, "cronMinutes" },
               { cron_hours, "cron-hours" },
               { cron_hours, "cronHours" },
               { cron_days, "cron-days" },
               { cron_days, "cronDays" },
               { cron_months, "cron-months" },
               { cron_months, "cronMonths", },
               { cron_days_of_week, "cron-days-of-week" },
               { cron_days_of_week, "cronDaysOfWeek" },
               { poll_station_enabled, "poll-station-enabled" },
               { poll_station_enabled, "pollStationEnabled" },
               { station_to_poll, "station-to-poll" },
               { station_to_poll, "stationToPoll" },
               { ftp_server_address, "ftp-server-address" },
               { ftp_server_address, "ftpServerAddress" },
               { ftp_send_files_enabled, "ftp-send-files-enabled" },
               { ftp_send_files_enabled, "ftpSendFilesEnabled" },
               { ftp_user_name, "ftp-user-name" },
               { ftp_user_name, "ftpUserName" },
               { ftp_password, "ftp-password" },
               { ftp_password, "ftpPassword" },
               { ftp_use_sftp, "ftp-use-sftp" },
               { ftp_use_sftp, "ftpUseSftp" },
               { ftp_protocol, "ftp-protocol" },
               { ftp_protocol, "ftpProtocol" },
               { ftp_queue_size, "ftp-queue-size" },
               { ftp_queue_size, "ftpQueueSize" },
               { ftp_remote_dir, "ftp-remote-dir" },
               { ftp_remote_dir, "ftpRemoteDir" },
               { ftp_enable_epsv, "ftp-enable-epsv" },
               { ftp_enable_epsv, "ftpEnableEpsv" },
               { exec_program_enabled, "exec-program-enabled" },
               { exec_program_enabled, "execProgramEnabled" },
               { exec_program_path, "exec-program-path" },
               { exec_program_path, "execProgramPath" },
               { exec_run_dir, "exec-run-dir" },
               { exec_run_dir, "execRunDir" },
               { exec_command_line, "exec-command-line" },
               { exec_command_line, "execCommandLine" },
               { exec_timeout, "exec-timeout" },
               { exec_timeout, "exec-time-out" },
               { exec_timeout, "execTimeOut" },
               { exec_run_minimised, "exec-run-minimised" },
               { exec_run_minimised, "execRunMinimised" },
               { exec_run_minimised, "exec-run-minimized" },
               { exec_run_minimised, "execRunMinmimized" },
               { status_last_time_triggered, "status-last-time-triggered" },
               { status_last_time_triggered, "statusLastTimeTriggered" },
               { status_next_expected_trigger, "status-next-expected-trigger" },
               { status_next_expected_trigger, "statusNextExpectedTrigger" },
               { status_actions_pending, "status-actions_pending" },
               { status_actions_pending, "statusActionsPending" },
               { status_poll_last_started, "status-poll-last-started" },
               { status_poll_last_started, "statusPollLastStarted" },
               { status_poll_last_finished, "status-poll-last-finished" },
               { status_poll_last_finished, "statusPollLastFinished" },
               { status_poll_last_outcome, "status-poll-last-outcome" },
               { status_poll_last_outcome, "statusPollLastOutcome" },
               { status_ftp_last_started, "status-ftp-last-started" },
               { status_ftp_last_started, "statusFtpLastStarted" },
               { status_ftp_last_finished, "status-ftp-last-finished" },
               { status_ftp_last_finished, "statusFtpLastFinished" },
               { status_ftp_last_outcome, "status-ftp-last-outcome" },
               { status_ftp_last_outcome, "statusFtpLastOutcome" },
               { status_exec_last_started, "status-exec-last-started" },
               { status_exec_last_started, "statusExecLastStarted" },
               { status_exec_last_finished, "status-exec-last-finished" },
               { status_exec_last_finished, "statusExecLastFinished" },
               { status_exec_last_outcome, "status-exec-last-outcome" },
               { status_exec_last_outcome, "statusExecLastOutcome" },
               { status_exec_last_exit_code, "status-exec-last-exit-code" },
               { status_exec_last_exit_code, "statusExecLastExitCode" },
               { success_expression, "success-expression" },
               { success_expression, "successExpression" },
               { trigger_delay, "trigger-delay" },
               { trigger_delay, "triggerDelay" },
               { 0, "" },
            };
         };

         
         char const *setting_id_to_string(uint4 setting_id)
         {
            char const *rtn = "";
            for(int i = 0; setting_maps[i].id != 0; ++i)
            {
               if(setting_id == setting_maps[i].id)
               {
                  rtn = setting_maps[i].name;
                  break;
               }
            }
            return rtn;
         }


         setting_id_type str_to_setting_id(char const *s)
         {
            StrAsc name(s);
            uint4 rtn(0);
            for(int i = 0; setting_maps[i].id != 0 && rtn == 0; ++i)
            {
               if(name == setting_maps[i].name)
                  rtn = setting_maps[i].id;
            }
            if(rtn == 0)
            {
               // the identifier may be numeric.
               rtn = strtoul(s, 0, 10);
               if(setting_id_to_string(rtn)[0] == 0)
                  throw std::invalid_argument("invalid setting identifier");
            }
            return static_cast<setting_id_type>(rtn);
         }
      };

      namespace Triggers
      {
         namespace
         {
            struct trigger_map_type
            {
               uint4 const type;
               char const *name;
            } trigger_type_maps[] =
            {
               {trigger_after_scheduled, "trigger-after-scheduled"},
               {trigger_after_scheduled, "trigger_after_scheduled"},
               {trigger_after_scheduled, "triggerAfterScheduled"},

               {trigger_successful_poll,"trigger-successful-poll"},
               {trigger_successful_poll,"trigger_successful_poll"},
               {trigger_successful_poll,"triggerSuccessfulPoll"},

               {trigger_failed_poll, "trigger-failed-poll"},
               {trigger_failed_poll, "trigger_failed_poll"},
               {trigger_failed_poll, "triggerFailedPoll"},

               {trigger_any_poll, "trigger-any-poll"},
               {trigger_any_poll, "trigger_any_poll"},
               {trigger_any_poll, "triggerAnyPoll"},

               {trigger_poll_with_some_data, "trigger-poll-with-some-data"},
               {trigger_poll_with_some_data, "trigger_poll_with_some_data"},
               {trigger_poll_with_some_data, "triggerPollWithSomeData"},

               {trigger_callback_begins, "trigger-callback-begins"},
               {trigger_callback_begins, "trigger_callback_begins"},
               {trigger_callback_begins, "triggerCallbackBegins"},

               {trigger_callback_ends, "trigger-callback-ends"},
               {trigger_callback_ends, "trigger_callback_ends"},
               {trigger_callback_ends, "triggerCallbackEnds"},

               {trigger_retry_poll_failed, "trigger-retry-poll-failed"},
               {trigger_retry_poll_failed, "trigger_retry_poll_failed"},
               {trigger_retry_poll_failed, "triggerRetryPollFailed"},

               {trigger_switch_to_secondary, "trigger-switch-to-secondary"},
               {trigger_switch_to_secondary, "trigger_switch_to_secondary"},
               {trigger_switch_to_secondary, "triggerSwitchToSecondary"},

               {trigger_one_way_data, "trigger-one-way-data"},
               {trigger_one_way_data, "trigger_one_way_data"},
               {trigger_one_way_data, "triggerOneWayData"},

               {trigger_data_file_closed, "trigger-data-file-closed,"},
               {trigger_data_file_closed, "trigger_data_file_closed,"},
               {trigger_data_file_closed, "triggerDataFileClosed,"},

               {trigger_after_task, "trigger-after-task"},
               {trigger_after_task, "trigger_after_task"},
               {trigger_after_task, "triggerAfterTask"},

               {trigger_on_schedule, "trigger-on-schedule"},
               {trigger_on_schedule, "trigger_on_schedule"},
               {trigger_on_schedule, "triggerOnSchedule"},

               {trigger_on_cron, "trigger-on-cron"},
               {trigger_on_cron, "trigger_on_cron"},
               {trigger_on_cron, "triggerOnCron"},

               {trigger_after_any_data_collected, "trigger-after-any-data-collected"},
               {trigger_after_any_data_collected, "trigger_after_any_data_collectedd"},
               {trigger_after_any_data_collected, "triggerAfterAnyDataCollected"},

               {trigger_after_file_synch, "trigger-after-file-synch"},
               {trigger_after_file_synch, "trigger_after_file_synch"},
               {trigger_after_file_synch, "triggerAfterFileSynch"},

               { 0, "" }
            };
         };

         char const *trigger_type_to_str(uint4 trigger_type)
         {
            char const *rtn = "";
            for(int i = 0; trigger_type_maps[i].type != 0; ++i)
            {
               if(trigger_type == trigger_type_maps[i].type)
               {
                  rtn = trigger_type_maps[i].name;
                  break;
               }
            }
            return rtn;
         }

         trigger_type str_to_trigger_type(char const *s)
         {
            StrAsc name(s);
            uint4 rtn(0);
            for(int i = 0; trigger_type_maps[i].type != 0 && rtn == 0; ++i)
            {
               if(name == trigger_type_maps[i].name)
                  rtn = trigger_type_maps[i].type;
            }
            if(rtn == 0)
            {
               rtn = strtoul(s, 0, 10);
               if(trigger_type_to_str(rtn)[0] == 0)
                  throw std::invalid_argument("invalid trigger type identifier");
            }
            return static_cast<trigger_type>(rtn);
         }
      }
   };
};

