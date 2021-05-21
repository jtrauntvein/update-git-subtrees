/* Csi.Win32.TapiLine.h

   Copyright (C) 2000, 2006 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 20 November 2000
   Last Change: Tuesday 10 January 2006
   Last Commit: $Date: 2007-11-13 14:47:16 -0600 (Tue, 13 Nov 2007) $ (UTC)
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Win32_TapiLine_h
#define Csi_Win32_TapiLine_h

#include "Csi.Win32.TapiApplication.h"
#include "Csi.Win32.VariableSizedStruct.h"
#include "Csi.InstanceValidator.h"
#include "Csi.SharedPtr.h"

 
namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class TapiLine
      //
      // Represents the resources used by a TAPI line object
      ////////////////////////////////////////////////////////////
      class TapiLine: public InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         TapiLine(
            StrAsc const &line_name_,
            Csi::SharedPtr<TapiApplication> &application_);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~TapiLine();

         ////////////////////////////////////////////////////////////
         // start_data_call
         //
         // Starts the process of making a call with the purpose of obtaining a 
         // data modem connection. on_data_modem_connection() will be invoked when
         // the connection has been completed successfully. If the call fails,
         // on_data_modem_call_failed() will be invoked with a value that indicates the
         // nature of the failure.
         ////////////////////////////////////////////////////////////
         void start_data_call(bool isDialable, StrAsc const &address);

         ////////////////////////////////////////////////////////////
         // start_end_data_call
         //
         // Starts the process of closing the call (if any) that this object is 
         // managing. on_data_modem_disconnection() will be called when the call
         // has been closed.
         //
         //  The return value will be false if there is no call to end.
         ////////////////////////////////////////////////////////////
         bool start_end_data_call();

         ////////////////////////////////////////////////////////////
         // on_tapi_event
         //
         // Called when a telephony event has been reported through the application
         ////////////////////////////////////////////////////////////
         virtual void on_tapi_event(
            uint4 tapi_message,
            uint4 param1,
            uint4 param2,
            uint4 param3);

      protected:
         ////////////////////////////////////////////////////////////
         // call_state
         ////////////////////////////////////////////////////////////
         enum call_state_type
         {
            state_standby,
            state_call_started,
            state_connected,
            state_connected_through_callback,
            state_dropping_call,
            state_cancelling_call
         } call_state;
    
         //@group application level events	
         ////////////////////////////////////////////////////////////
         // on_data_modem_connection
         //
         // Called when a data modem connection has been made. This method should be overloaded in
         // order to access the communication handle. The default version will close the handle.
         ////////////////////////////////////////////////////////////
         virtual void on_data_modem_connection(HANDLE comm_handle) = 0;

         ////////////////////////////////////////////////////////////
         // on_data_modem_call_status
         //
         // Invoked when information is received regarding the progress of a pending call. A derived
         // class can overload this method in order to receive these notifications.
         ////////////////////////////////////////////////////////////
         enum call_status_type
         {
            call_status_unknown      = 0,
            call_status_dial_tone    = 1,
            call_status_dialing      = 2,
            call_status_ringback     = 3,
            call_status_busy         = 4,
            call_status_no_dial_tone = 5,
            call_status_proceeding   = 6,
            call_status_connected    = 7,
            call_status_disconnected = 8,
         };
         virtual void on_data_modem_call_status(call_status_type status) { }

         ////////////////////////////////////////////////////////////
         // on_data_modem_call_failed
         //
         // Called to notify the application that the call failed
         ////////////////////////////////////////////////////////////
         enum call_failed_type
         {
            call_failed_unknown = 0,
            call_failed_busy = 1,
            call_failed_no_dial_tone = 2,
            call_failed_dial_not_complete = 3,
         };
         virtual void on_data_modem_call_failed(call_failed_type failure) = 0;
	 
         ////////////////////////////////////////////////////////////
         // on_data_modem_disconnection
         //
         // Called when a data modem disconnection has been made. This method should be overloaded
         // in order to know when a line owner is deleted.
         ////////////////////////////////////////////////////////////
         virtual void on_data_modem_disconnection(
            call_state_type previous_call_state) = 0;
	 //@endgroup

         //@group rate query methods
         // These methods allow objects of derived classes to specify the minimum and maximum rates
         // that are used when a call is made. If they are not overloaded, the telephony defaults
         // will be used.

         ////////////////////////////////////////////////////////////
         // get_min_rate
         ////////////////////////////////////////////////////////////
         virtual uint4 get_min_rate() { return 0; }

         ////////////////////////////////////////////////////////////
         // get_max_rate
         ////////////////////////////////////////////////////////////
         virtual uint4 get_max_rate() { return 0; }
         //@endgroup
      
      protected:
         ////////////////////////////////////////////////////////////
         // call_handle
         //
         // Handle to the current call (if any)
         ////////////////////////////////////////////////////////////
         HCALL call_handle;

      protected:
         //@group low level event handlers
         // These methods handle the low level events coming from telephony. These can be overloaded
         // by derived classes but there is little need to.
         
         ////////////////////////////////////////////////////////////
         // on_reply
         ////////////////////////////////////////////////////////////
         virtual void on_reply(int4 requestid, int4 rcd);

         ////////////////////////////////////////////////////////////
         // on_callstate
         ////////////////////////////////////////////////////////////
         virtual void on_callstate(
            uint4 new_state,
            uint4 state_dependent_info,
            uint4 new_privilege);
            
         ////////////////////////////////////////////////////////////
         // on_new_callback
         ////////////////////////////////////////////////////////////
         virtual void on_new_callback(
            uint4 dwAddressID,
            uint4 hCall,
            uint4 dwPrivilege);

         ////////////////////////////////////////////////////////////
         // on_connected
         ////////////////////////////////////////////////////////////
         virtual void on_connected(uint4 connected_mode);

         ////////////////////////////////////////////////////////////
         // on_disconnected
         ////////////////////////////////////////////////////////////
         virtual void on_disconnected(uint4 disconnected_mode);
         
         ////////////////////////////////////////////////////////////
         // on_idle
         ////////////////////////////////////////////////////////////
         virtual void on_idle(uint4 idle_mode);
         //@endgroup
	 
      private:         
         ////////////////////////////////////////////////////////////
         // line_open
         //
         // Responsible for opening the TAPI line handle
         ////////////////////////////////////////////////////////////
         void line_open();

         ////////////////////////////////////////////////////////////
         // line_drop
         //
         // Responsible for releasing the TAPI line handle
         //////////////////////////////////////////////////////////// 
         void line_drop();

      private:
         ////////////////////////////////////////////////////////////
         // application
         //
         // Reference to the application level handler for TAPI. This object is used for several
         // purposes by the line object and is also the source of events that will be processed by
         // this line object.
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<TapiApplication> application;

         ////////////////////////////////////////////////////////////
         // line_name
         //
         // The name of the line that should be selected when the line handle is opened. This should
         // ideally match one of those reported by the operating system. This value is initialised
         // by the constructor.
         //////////////////////////////////////////////////////////// 
         StrAsc line_name;

         ////////////////////////////////////////////////////////////
         // line_handle
         //
         // Handle to the OS object that represents the line resource. This value is initialised by
         // the constructor (if that initialisation fails, the constructor will throw an exception)
         // and is released by the destructor.
         ////////////////////////////////////////////////////////////
         HLINE line_handle;
      };
   };
};

#endif
