/* Csi.Win32.TapiError.cpp

   Copyright (C) 2000, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 28 September 2000
   Last Change: Friday 20 August 2010
   Last Commit: $Date: 2010-08-20 13:08:46 -0600 (Fri, 20 Aug 2010) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32.TapiError.h"
#define  WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <tapi.h>
#include <sstream>


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class TapiError definitions
      //////////////////////////////////////////////////////////// 
      TapiError::TapiError(int4 error_code_):
         error_code(error_code_),
         MsgExcept("")
      {
         // we want to map the error message to a descriptive string
         std::ostringstream out;
         out << "TAPI error " << std::hex << static_cast<uint4>(error_code) << "\",\"";
         switch(error_code)
         {
         case LINEERR_ADDRESSBLOCKED:
            out << "specified address is blocked or call blocking enabled";
            break;
            
         case LINEERR_ALLOCATED:
            out << "line cannot be opened";
            break;
            
         case LINEERR_BADDEVICEID:
            out << "device identifier invalid or out of range";
            break;
            
         case LINEERR_BEARERMODEUNAVAIL:
            out << "bearer mode is not available for the line";
            break;
            
         case LINEERR_BILLINGREJECTED:
            out << "billing mode of the call was rejected";
            break;
            
         case LINEERR_CALLUNAVAIL:
            out << "all call appearances on the specified address are in use";
            break;
            
         case LINEERR_COMPLETIONOVERRUN:
            out << "the maximum of call completions has been exceeded";
            break;
            
         case LINEERR_CONFERENCEFULL:
            out << "the maximum numbers of parties for a conference call has been reached";
            break;
            
         case LINEERR_DIALBILLING:
         case LINEERR_DIALDIALTONE:
         case LINEERR_DIALPROMPT:
         case LINEERR_DIALQUIET:
            out << "the dialable address contains control characters that are not processed";
            break;

         case LINEERR_DIALVOICEDETECT:
            out << "use of the \":\" dial modifier is not supported";
            break;
            
         case LINEERR_INCOMPATIBLEAPIVERSION:
            out << "requested a TAPI version that is not supported by TAPI";
            break;
            
         case LINEERR_INCOMPATIBLEEXTVERSION:
            out << "requested an extension version that is not supported by TAPI";
            break;
            
         case LINEERR_INIFILECORRUPT:
            out << "TAPI cannot be initialized because of internal inconsistencies or formatting problems";
            break;
            
         case LINEERR_INUSE:
            out << "the line device is in use";
            break;
            
         case LINEERR_INVALADDRESS:
            out << "the specified address is invalid or not allowed";
            break;
            
         case LINEERR_INVALADDRESSID:
            out << "the specified address identifier is invalid or out of range";
            break;
            
         case LINEERR_INVALADDRESSMODE:
            out << "the specified address mode is invalid";
            break;

         case LINEERR_INVALADDRESSSTATE:
            out << "the specified address state is invalid";
            break;
            
         case LINEERR_INVALAGENTACTIVITY:
            out << "the specified agent activity is invalid";
            break;
            
         case LINEERR_INVALAGENTGROUP:
            out << "the specified agent group is invalid";
            break;
            
         case LINEERR_INVALAGENTID:
            out << "the specified agent id is invalid";
            break;
            
         case LINEERR_INVALAGENTSTATE:
            out << "the specified agent state is invalid";
            break;
            
         case LINEERR_INVALAPPHANDLE:
            out << "the application handle is invalid";
            break;
            
         case LINEERR_INVALAPPNAME:
            out << "the specified application name is invalid";
            break;

         case LINEERR_INVALBEARERMODE:
            out << "the specified bearer mode is invalid";
            break;
            
         case LINEERR_INVALCALLCOMPLMODE:
            out << "the specified completion is invalid";
            break;
            
         case LINEERR_INVALCALLHANDLE:
            out << "the specified call handle is invalid";
            break;
            
         case LINEERR_INVALCALLPARAMS:
            out << "invalid call parameters";
            break;
            
         case LINEERR_INVALCALLPRIVILEGE:
            out << "invalid call privilege parameter";
            break;
            
         case LINEERR_INVALCALLSELECT:
            out << "invalid select parameter";
            break;
            
         case LINEERR_INVALCALLSTATE:
            out << "invalid call state for the requested operation";
            break;
            
         case LINEERR_INVALCALLSTATELIST:
            out << "invalid call state list";
            break;
            
         case LINEERR_INVALCARD:
            out << "the permanent card  ID could not be found in any entry of the [Cards] section"
                << "in the registry";
            break;
            
         case LINEERR_INVALCOMPLETIONID:
            out << "the completion identifier is invalid";
            break;
            
         case LINEERR_INVALCONFCALLHANDLE:
            out << "the conference call handle is invalid";
            break;
            
         case LINEERR_INVALCONSULTCALLHANDLE:
            out << "invalid consultation call handle";
            break;
            
         case LINEERR_INVALCOUNTRYCODE:
            out << "invalid country code";
            break;
            
         case LINEERR_INVALDEVICECLASS:
            out << "invalid device class";
            break;
            
         case LINEERR_INVALDEVICEHANDLE:
            out << "invalid device handle";
            break;

         case LINEERR_INVALDIALPARAMS:
            out << "invalid dialing parameters";
            break;
            
         case LINEERR_INVALDIGITLIST:
            out << "invalid digit list";
            break;
            
         case LINEERR_INVALDIGITMODE:
            out << "invalid digit mode";
            break;
            
         case LINEERR_INVALDIGITS:
            out << "invalid digits";
            break;
            
         case LINEERR_INVALEXTVERSION:
            out << "invalid extension version";
            break;
            
         case LINEERR_INVALFEATURE:
            out << "invalid or unavailable feature parameter";
            break;
            
         case LINEERR_INVALGROUPID:
            out << "invalid group ID";
            break;
            
         case LINEERR_INVALLINEHANDLE:
            out << "invalid line handle";
            break;
            
         case LINEERR_INVALLINESTATE:
            out << "the device configuration may not be changed in the current line state";
            break;
            
         case LINEERR_INVALLOCATION:
            out << "the location identifier cannot be found in any entry in the [Locations]"
                << "section of the registry";
            break;
            
         case LINEERR_INVALMEDIALIST:
            out << "invalid media list";
            break;
            
         case LINEERR_INVALMEDIAMODE:
            out << "the list of media modes to be monitored contains invalid information";
            break;
            
         case LINEERR_INVALMESSAGEID:
            out << "message ID is out of range";
            break;
            
         case LINEERR_INVALPARAM:
            out << "a structure or parameter contains invalid information";
            break;
            
         case LINEERR_INVALPARKID:
            out << "invalid park identifier";
            break;
               
         case LINEERR_INVALPARKMODE:
            out << "invalid park mode";
            break;
            
         case LINEERR_INVALPASSWORD:
            out << "the specified password is incorrect";
            break;

         case LINEERR_INVALPOINTER:
            out << "one or more specified pointer parameters are invalid";
            break;
            
         case LINEERR_INVALPRIVSELECT:
            out << "invalid flag or combination of flags was specified for privileges";
            break;
            
         case LINEERR_INVALRATE:
            out << "the specified rate is invalid";
            break;
            
         case LINEERR_INVALREQUESTMODE:
            out << "invalid request mode";
            break;
            
         case LINEERR_INVALTERMINALID:
            out << "invalid terminal ID";
            break;
            
         case LINEERR_INVALTERMINALMODE:
            out << "invalid terminal mode";
            break;
            
         case LINEERR_INVALTIMEOUT:
            out << "timeouts are not supported or out of range for the line";
            break;
            
         case LINEERR_INVALTONE:
            out << "custom tone does not represent a valid tone";
            break;
            
         case LINEERR_INVALTONELIST:
            out << "invalid tone list";
            break;
            
         case LINEERR_INVALTONEMODE:
            out << "invalid tone moded";
            break;
            
         case LINEERR_INVALTRANSFERMODE:
            out << "invalid transfer mode";
            break;
            
         case LINEERR_LINEMAPPERFAILED:
            out << "no lines found to match the requirements";
            break;
            
         case LINEERR_NOCONFERENCE:
            out << "the specified call handle is not a conference call";
            break;
            
         case LINEERR_NODEVICE:
            out << "the specified device ID is no longer valid";
            break;
            
         case LINEERR_NODRIVER:
            out << "either tapi.dll could not be located or a"
                << " telephone service provider could not load";
            break;
            
         case LINEERR_NOMEM:
            out << "insufficient memory for the specified operation";
            break;
            
         case LINEERR_NOMULTIPLEINSTANCE:
            out << "multiple instances of the service provider are not allowed";
            break;
            
         case LINEERR_NOREQUEST:
            out << "there is currently no request pending";
            break;
            
         case LINEERR_NOTOWNER:
            out << "the application does not have owner privilege for the specified call";
            break;
            
         case LINEERR_NOTREGISTERED:
            out << "the application is not registered as a recipient for the "
                << "indicated request mode";
            break;
            
         case LINEERR_OPERATIONFAILED:
            out << "the operation failed for an unknown reason";
            break;
            
         case LINEERR_OPERATIONUNAVAIL:
            out << "the operation is not available for the specified line";
            break;
            
         case LINEERR_RATEUNAVAIL:
            out << "the service provider does not have enough bandwidth for the speciifed rate";
            break;
            
         case LINEERR_REINIT:
            out << "the application attempted to initialise TAPI twice";
            break;
            
         case LINEERR_REQUESTOVERRUN:
            out << "more requests are pending than the device can handle";
            break;
            
         case LINEERR_RESOURCEUNAVAIL:
            out << "insufficient resource available for the specified operation";
            break;
            
         case LINEERR_STRUCTURETOOSMALL:
            out << "the specified structure is too small";
            break;
            
         case LINEERR_TARGETNOTFOUND:
            out << "a target for a call hand off was not found";
            break;
            
         case LINEERR_TARGETSELF:
            out << "the application invoking this operation is the target of the indirect handoff";
            break;
            
         case LINEERR_UNINITIALIZED:
            out << "the operation was invoked before the application called lineInitialize()";
            break;
            
         case LINEERR_USERUSERINFOTOOBIG:
            out << "string containing user-user info is too long";
            break;
            
         default:
            out << "unknown error";
            break;
         }
         msg.append(
            out.str().c_str(),
            out.str().length());
      } // constructor
   };
};
