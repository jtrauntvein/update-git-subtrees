/* Csi.Win32.TapiLine.cpp

   Copyright (C) 2000, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 20 November 2000
   Last Change: Tuesday 10 July 2012
   Last Commit: $Date: 2012-07-10 10:40:39 -0600 (Tue, 10 Jul 2012) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32.TapiLine.h"
#include "Csi.Win32.TapiError.h"
#include <iostream>
#include <assert.h>


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class TapiLine definitions
      ////////////////////////////////////////////////////////////
      TapiLine::TapiLine(
         StrAsc const &line_name_,
         SharedPtr<TapiApplication> &application_):
         application(application_),
         line_handle(0),
         call_handle(0),
         call_state(state_standby),
         line_name(line_name_)
      { line_open(); }


      TapiLine::~TapiLine()
      {
      	 assert(call_handle == 0);
      	 if(line_handle)
      	 {
      	    lineClose(line_handle);
      	    line_handle = 0;
      	 }
      } // destructor


      void TapiLine::line_open()
      {
         // get the information about the line
         uint4 identifier;
         uint4 api_version;

         if(application->get_line_info(line_name,identifier,api_version))
         {
            // configure a call parameters structure to be passed to lineOpen. This should give us
            // the ability to specify the baud rate?
            LINECALLPARAMS call_params;

            memset(&call_params,0,sizeof(call_params));
            call_params.dwTotalSize = sizeof(call_params);
            call_params.dwBearerMode = LINEBEARERMODE_VOICE;
            call_params.dwMinRate = get_min_rate();
            call_params.dwMaxRate = get_max_rate();
            call_params.dwMediaMode = LINEMEDIAMODE_UNKNOWN|LINEMEDIAMODE_DATAMODEM;
            call_params.dwAddressMode = LINEADDRESSMODE_DIALABLEADDR;
            
            // open the line handle
            int4 rcd;
            rcd = lineOpen(
               application->get_kernel_handle(),
               identifier,
               &line_handle,
               api_version,
               0,               // don't use ext version
               reinterpret_cast<uint4>(this),
               LINECALLPRIVILEGE_OWNER,
               LINEMEDIAMODE_DATAMODEM,
               &call_params);
            if(rcd != 0)
               throw TapiError(rcd);
         }
         else
            throw MsgExcept("TAPI line does not exist");
      } // line_open


      void TapiLine::line_drop()
      {
         // if the call is still in progress (evidenced by a valid value in the call_handle member,
         // then we need to initiate the process of dropping that call. the lineDrop() function
         // initiates this process. When the call is in state where it can be deleted, we will get
         // an idle message from telephony.
         if(call_state != state_standby &&
            call_state != state_dropping_call &&
            call_state != state_cancelling_call)
         {
            long rcd;
            assert(call_handle != 0);
            if(call_state == state_call_started)
               call_state = state_cancelling_call;
            else
               call_state = state_dropping_call;
            rcd = lineDrop(call_handle,0,0);
            if(rcd <= 0)
               on_idle(0);
         }
      } // line_drop


      void TapiLine::start_data_call(bool is_dialable, StrAsc const &address)
      {
         assert(line_handle != 0);
         if(call_state == state_standby)
         {
            assert(call_handle == 0);
            
            // we might need to have telephony translate a canonical address into a dialable
            // one. This is controlled by the is_dialable parameter.
            StrAsc dialable;

            if(!is_dialable)
               application->translate_address(dialable,line_name,address);
            else
               dialable = address;

            // we can now initiate the call.
            LINECALLPARAMS params;
            int4 rcd;

            memset(&params,0,sizeof(params));
            params.dwTotalSize = sizeof(params);
            params.dwMediaMode = LINEMEDIAMODE_DATAMODEM;
            params.dwMinRate = get_min_rate();
            params.dwMaxRate = get_max_rate();
            rcd = lineMakeCallA(
               line_handle,
               &call_handle,
               dialable.c_str(),
               0,
               &params);
            if(rcd < 0)
               throw TapiError(rcd);
            call_state = state_call_started;
         }
         else
            throw MsgExcept("Call in progress");
      } // start_data_call


      bool TapiLine::start_end_data_call()
      {
         bool rtn = false;
         if(call_state != state_standby)
         {
            line_drop();
            rtn = true;
         }
         return rtn;
      } // start_end_data_call


      void TapiLine::on_tapi_event(
         uint4 tapi_message,
         uint4 param1,
         uint4 param2,
         uint4 param3)
      {
         switch(tapi_message)
         {
         case LINE_REPLY:
            on_reply(static_cast<int4>(param1), static_cast<int4>(param2));
            break;

         case LINE_CALLSTATE:
            on_callstate(param1,param2,param3);
            break;

         case LINE_APPNEWCALL:
            on_new_callback(param1,param2,param3);
            break;

         default:
            break;
         }
      } // on_tapi_event


      void TapiLine::on_reply(int4 requestid, int4 rcd)
      {
         // zero indicates an error
         if(rcd < 0)
         {
            if(call_state == state_call_started)
               on_data_modem_call_failed(call_failed_dial_not_complete);
         }
      } // on_reply


      void TapiLine::on_callstate(
         uint4 new_state,
         uint4 state_dependent_info,
         uint4 new_privilege)
      {
         switch(new_state)
         {
         case LINECALLSTATE_IDLE:
            on_idle(state_dependent_info);
            break;

         case LINECALLSTATE_DIALTONE:
      	   on_data_modem_call_status(call_status_dial_tone);
            break;

         case LINECALLSTATE_DIALING:
      	   on_data_modem_call_status(call_status_dialing);
            break;

         case LINECALLSTATE_RINGBACK:
      	   on_data_modem_call_status(call_status_ringback);
            break;

         case LINECALLSTATE_BUSY:
      	   on_data_modem_call_status(call_status_busy);
      	   on_data_modem_call_failed(call_failed_busy);
            break;

         case LINECALLSTATE_SPECIALINFO:
            on_data_modem_call_status(call_status_no_dial_tone);
            on_data_modem_call_failed(call_failed_no_dial_tone);
            break;

         case LINECALLSTATE_PROCEEDING:
            on_data_modem_call_status(call_status_proceeding);
            break;

         case LINECALLSTATE_CONNECTED:
            on_data_modem_call_status(call_status_connected);
            on_connected(state_dependent_info);
            break;

         case LINECALLSTATE_DISCONNECTED:
            on_data_modem_call_status(call_status_disconnected);
            on_disconnected(state_dependent_info);
            break;

         case LINECALLSTATE_UNKNOWN:
            on_data_modem_call_status(call_status_unknown);
            break;
         }
      } // on_callstate


      void TapiLine::on_new_callback(
         uint4 dwAddressID,
         uint4 hCall,
         uint4 dwPrivilege)
      {
         if(call_state == state_standby)
         {
            call_handle = static_cast<HCALL>(hCall);
            call_state = state_connected_through_callback;
            lineAnswer(call_handle,0,0);
         }
      } // on_new_callback


      void TapiLine::on_connected(uint4 connected_mode)
      {
         // we will assume that this is a data call and therfore
         // get the data modem line id
         typedef Csi::Win32::VariableSizedStruct<VARSTRING> vardata_type;
         vardata_type vardataInfo;
         long rcd;
         bool has_sufficient_storage = false;
         
         while(!has_sufficient_storage)
         {
            rcd = lineGetIDA(
               0,               // line handle
               0,               // dwAddressID 
               call_handle,
               LINECALLSELECT_CALL,
               vardataInfo.get_struct(),
               "comm/datamodem" );
            if(rcd == 0 && !vardataInfo.has_sufficient_storage())
               vardataInfo.reallocate();
            else
               has_sufficient_storage = true;
         }
         
         if(rcd == 0)
         {
            if(vardataInfo->dwStringFormat == STRINGFORMAT_BINARY)
            {
               // the com port handle (and name) are stored in a structure within the structure that
               // we passed to lineGetID(). We need to do some rather nasty casts to get to this
               // information. 
               struct handle_and_string_type
               {
                  HANDLE comm_handle;
                  char const *comm_port_name;
               } *handle_and_string;
               byte *handle_and_string_storage;

               handle_and_string_storage =
                  static_cast<byte *>(vardataInfo.get_storage()) +
                  vardataInfo->dwStringOffset;
               handle_and_string =
                  reinterpret_cast<handle_and_string_type *>(handle_and_string_storage);
               
               // report the connection event and pass the comm handle to the application
               if(call_state == state_call_started)
                  call_state = state_connected;
               on_data_modem_connection(handle_and_string->comm_handle);
            }
         }
      } // on_connected


      void TapiLine::on_disconnected(uint4 disconnected_mode)
      { line_drop(); }
      
      
      void TapiLine::on_idle(uint4 idle_mode)
      {
         // since we are idle, we can release the call handle.
         if(call_state != state_standby)
         {
            assert(call_handle != 0);
            lineDeallocateCall(call_handle);
            call_handle = 0;

            // we need to notify the application of the event
            call_state_type old_call_state = call_state;
            
            call_state = state_standby;
            if(old_call_state == state_call_started || old_call_state == state_cancelling_call)
               on_data_modem_call_failed(call_failed_dial_not_complete);
            else
               on_data_modem_disconnection(old_call_state);
         }
      } // on_idle
   };
};
