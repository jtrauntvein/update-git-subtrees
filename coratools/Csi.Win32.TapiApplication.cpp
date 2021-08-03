/* Csi.Win32.TapiApplication.cpp

   Copyright (C) 2000, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 27 September 2000
   Last Change: Wednesday 26 November 2014
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32.TapiApplication.h"
#include "Csi.Win32.VariableSizedStruct.h"
#include "Csi.Win32.TapiError.h"
#include "Csi.Win32.TapiLine.h"
#include "MsgExcept.h"
#include <sstream>

namespace Csi
{
   namespace Win32
   {
      namespace TapiApplicationHelpers
      {
         ////////////////////////////////////////////////////////////
         // class line_info_type
         //
         // Lists information about the line that will be needed to open the line later
         ////////////////////////////////////////////////////////////
         class line_info_type
         {
         public:
            ////////////////////////////////////////////////////////////
            // api_version
            //
            // The API version that was negotiated for this line
            ////////////////////////////////////////////////////////////
            uint4 api_version;

            ////////////////////////////////////////////////////////////
            // supported_media_modes
            //
            // A bitmap that describes the media modes that are supported by this line
            ////////////////////////////////////////////////////////////
            uint4 supported_media_modes;

            ////////////////////////////////////////////////////////////
            // identifier
            //
            // The sequence in which this line appeared when the lines were scanned
            ////////////////////////////////////////////////////////////
            uint4 identifier;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            line_info_type(
               uint4 api_version_,
               uint4 supported_media_modes_,
               uint4 identifier_):
               api_version(api_version_),
               supported_media_modes(supported_media_modes_),
               identifier(identifier_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class event_line
         //
         // Defines a line event. This type will be posted when an event has been signalled from the
         // TAPI application.
         ////////////////////////////////////////////////////////////
         class event_line: public ::Csi::Event
         {
         public:
            static uint4 const event_id;
            uint4 device_handle;
            uint4 tapi_message;
            uint4 callback_instance;
            uint4 param1;
            uint4 param2;
            uint4 param3;

         private:
            event_line(
               TapiApplication *receiver,
               uint4 device_handle_,
               uint4 tapi_message_,
               uint4 callback_instance_,
               uint4 param1_,
               uint4 param2_,
               uint4 param3_):
               ::Csi::Event(event_id,receiver),
               device_handle(device_handle_),
               tapi_message(tapi_message_),
               callback_instance(callback_instance_),
               param1(param1_),
               param2(param2_),
               param3(param3_)
            { }

         public:
            static void create_and_post(
               TapiApplication *receiver,
               uint4 device_handle,
               uint4 tapi_message,
               uint4 callback_instance,
               uint4 param1,
               uint4 param2,
               uint4 param3)
            {
               (new event_line(
                  receiver,
                  device_handle,
                  tapi_message,
                  callback_instance,
                  param1,
                  param2,
                  param3))->post();
            }
         };


         uint4 const event_line::event_id =
         Event::registerType("Csi::Win32::TapiApplication::event_line");
      };


      ////////////////////////////////////////////////////////////
      // class TapiApplication definitions
      ////////////////////////////////////////////////////////////
      TapiApplication::TapiApplication(HINSTANCE module_handle):
         api_version(0x00020001) // we want to require at least TAPI version 2.1 on the system
      {
         // make the operating system call to initialise the TAPI layer
         LINEINITIALIZEEXPARAMS initialise_parameters;
         int4 rcd;
         uint4 number_of_lines;

         memset(&initialise_parameters,0,sizeof(initialise_parameters));
         initialise_parameters.dwTotalSize = sizeof(initialise_parameters);
         initialise_parameters.dwOptions = LINEINITIALIZEEXOPTION_USEEVENT;
         rcd = lineInitializeEx(
            &kernel_handle,
            module_handle,
            0,                  // no callback function
            0,                  // use default app name
            &number_of_lines,
            &api_version,
            &initialise_parameters);
         if(rcd != 0)
            throw TapiError(rcd);
         kernel_event = initialise_parameters.Handles.hEvent;
         for(uint4 i = 0; i < number_of_lines; ++i)
            on_line_create(i);
         kill_event.reset();
         start();
      } // constructor


      TapiApplication::~TapiApplication()
      {
         wait_for_end();
         lineShutdown(kernel_handle);
      } // destructor


      void TapiApplication::list_line_names(line_names_type &line_names)
      {
         line_names.clear();
         for(line_records_type::const_iterator li = line_records.begin();
             li != line_records.end();
             ++li)
            line_names.push_back(li->first);
      } // list_line_names


      char const *TapiApplication::translate_address(
         StrAsc &dest,
         StrAsc const &line_name,
         StrAsc const &canonical_address)
      {
         // locate the line record associated with the specified line name
         line_records_type::const_iterator li = line_records.find(line_name);
         if(li == line_records.end())
            throw MsgExcept("TapiApplication::translate_address -- invalid line specified");

         // call the function to translate the address
         typedef Csi::Win32::VariableSizedStruct<LINETRANSLATEOUTPUT> output_type;
         output_type output;
         long rcd;
         bool address_translated = false;

         while(!address_translated)
         {
            rcd = lineTranslateAddressA(
               kernel_handle,
               li->second->identifier,
               api_version,
               canonical_address.c_str(),
               0,               // no card info
               0,               // no translate options
               output.get_struct());
            if(rcd == 0 && !output.has_sufficient_storage())
               output.reallocate();
            else if(rcd != 0)
               throw TapiError(rcd);
            else
               address_translated = true;
         }

         // extract the dialable string from the structure
         char const *output_string = static_cast<char const *>(output.get_storage());
         if(rcd == 0)
            dest.setContents( 
               output_string + output->dwDialableStringOffset,
               output->dwDialableStringSize);
         else
            throw TapiError(rcd);
         return dest.c_str();
      } // translate_address


      bool TapiApplication::get_line_info(
         StrAsc const &line_name,
         uint4 &line_identifier,
         uint4 &line_api_version)
      {
         line_records_type::iterator li = line_records.find(line_name);
         if(li != line_records.end())
         {
            line_identifier = li->second->identifier;
            line_api_version = li->second->api_version;
            return true;
         }
         else
            return false;
      } // get_line_info


      bool TapiApplication::get_line_info_by_id(
         uint4 line_identifier,
         StrAsc &line_name,
         uint4 &line_api_version)
      {
         line_records_type::iterator li = line_records.begin();
         bool rtn = false;

         line_name.cut(0);
         line_api_version = 0;
         while(!rtn && li != line_records.end())
         {
            if(li->second->identifier == line_identifier)
            {
               rtn = true;
               line_name = li->first;
               line_api_version = li->second->api_version;
            }
            else
               ++li;
         }
         return rtn;
      } // get_line_info_by_id


      void TapiApplication::list_countries(country_list_type &output) const
      {
         uint4 current_country_id = 1;
         bool enumeration_complete = false;
         uint4 country_code;
         StrAsc country_name;
         uint4 next_country_id;
         
         output.clear();
         while(!enumeration_complete)
         {
            get_country_info_for_id(
               current_country_id,
               country_code,
               country_name,
               next_country_id);
            output.push_back(
               country_info_type(
                  country_code,
                  country_name.c_str(),
                  (uint4)country_name.length()));
            
            // we can now decide whether we are done iterating the list of countries
            current_country_id = next_country_id;
            if(current_country_id == 0)
               enumeration_complete = true;
         }
      } // list_countries


      void TapiApplication::get_default_location_info(
         uint4 &country_code,
         StrAsc &country_name,
         StrAsc &area_code)
      {
         // initialise the parameters
         country_code = 0;
         country_name.cut(0);
         area_code.cut(0);
         
         // we will get the current location by calling lineGetTranslateCaps(). we might have to do
         // so repeatedly until we have enough storage allocated.
         typedef VariableSizedStruct<LINETRANSLATECAPS> translate_caps_type;
         translate_caps_type translate_caps;
         bool has_sufficient_storage = false;

         while(!has_sufficient_storage)
         {
            long rcd = lineGetTranslateCaps(
               kernel_handle,
               api_version,
               translate_caps.get_struct());
            if(rcd == 0 && !translate_caps.has_sufficient_storage())
               translate_caps.reallocate();
            else if(rcd != 0)
               throw TapiError(rcd);
            else
               has_sufficient_storage = true;
         }

         // we now get the joy of sorting through the list of location entries that were returned
         // until we find one that matches the default location
         bool found_default_location = false;
         LINELOCATIONENTRY const *location_entries =
            reinterpret_cast<LINELOCATIONENTRY const *>(
               translate_caps.get_storage_bytes() + translate_caps->dwLocationListOffset);
         for(uint4 i = 0;
             !found_default_location && i < translate_caps->dwNumLocations;
             ++i)
         {
            if(location_entries[i].dwPermanentLocationID == translate_caps->dwCurrentLocationID)
            {
               // we have found the location. Now we need to get the country information for it.
               uint4 next_country_id;
               
               found_default_location = true;
               get_country_info_for_id(
                  location_entries[i].dwCountryID,
                  country_code,
                  country_name,
                  next_country_id);
               area_code.setContents(
                  static_cast<char *>(translate_caps.get_storage()) +
                     location_entries[i].dwCityCodeOffset,
                  location_entries[i].dwCityCodeSize - 1);
            }
         }
      } // get_default_location_info

      
      void TapiApplication::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace TapiApplicationHelpers;
         if(ev->getType() == event_line::event_id)
         {
            event_line *event = static_cast<event_line *>(ev.get_rep());
            TapiLine *associated_line =
               reinterpret_cast<TapiLine *>(event->callback_instance);

            if(!TapiLine::is_valid_instance(associated_line))
               associated_line = 0;
            switch(event->tapi_message)
            {
            case LINE_CREATE:
               on_line_create(event->param1);
               break;
               
            case LINE_REMOVE:
               on_line_remove(event->param1);
               break;

            default:
               if(associated_line)
                  associated_line->on_tapi_event(
                     event->tapi_message,
                     event->param1,
                     event->param2,
                     event->param3);
               break;
            }
         }
      } // receive


      void TapiApplication::wait_for_end()
      {
         kill_event.set();
         Thread::wait_for_end();
      } // wait_for_end


      void TapiApplication::execute()
      {
         // initialise the set of events that we will wait for 
         HANDLE waiting_events[2];

         waiting_events[0] = kill_event.get_handle();
         waiting_events[1] = kernel_event;

         // we will now enter a loop waiting for one of the events to be signalled. Each time the
         // kernel event is signalled, we will invoke lineGetMessage() in order to get TAPI
         // events. If the kill event is signalled, we will shut down
         bool done = false;
         while(!done)
         {
            uint4 wait_rcd = WaitForMultipleObjects(
               2,               // objects count
               waiting_events,
               FALSE,
               INFINITE);
            if(wait_rcd == WAIT_OBJECT_0 + 1)
            {
               int4 line_rcd = 0;
               LINEMESSAGE msg;

               while(line_rcd == 0)
               {
                  line_rcd = lineGetMessage(kernel_handle,&msg,0);
                  if(line_rcd == 0)
                     TapiApplicationHelpers::event_line::create_and_post(
                        this,
                        msg.hDevice,
                        msg.dwMessageID,
                        msg.dwCallbackInstance,
                        msg.dwParam1,
                        msg.dwParam2,
                        msg.dwParam3);
               }
            }
            else
               done = true;
         }
      } // execute


      void TapiApplication::on_line_create(uint4 line_identifier)
      {
         typedef Csi::Win32::VariableSizedStruct<LINEDEVCAPS> line_caps_type;
         line_caps_type line_caps;

         // negotiate the api version for the device
         uint4 device_api_version;
         LINEEXTENSIONID extension_id;
         int4 rcd;
         
         rcd = lineNegotiateAPIVersion(
            kernel_handle,
            line_identifier,    // device id
            0x00010004,      // low version
            0x00020001,      // high version
            &device_api_version,
            &extension_id);
         if(rcd != 0)
            device_api_version = 0;
         
         // get device capabilities. We might have to call the function multiple times until
         // there is enough storage allocated
         bool has_sufficient_storage = false;
         while(!has_sufficient_storage)
         {
            rcd = lineGetDevCaps(
               kernel_handle,
               line_identifier,
               device_api_version,
               0,               // use default EXT version
            line_caps.get_struct());
            if(rcd == 0 && !line_caps.has_sufficient_storage())
               line_caps.reallocate();
            else
               has_sufficient_storage = true;
         }
         
         // process the capabilities structure
         if(rcd == 0 && (line_caps->dwMediaModes & LINEMEDIAMODE_DATAMODEM))
         {
            switch(line_caps->dwStringFormat)
            {
            case STRINGFORMAT_ASCII:
            case STRINGFORMAT_DBCS:
            {
               // extract the line name
               StrAsc line_name;
               char const *line_name_storage =
                  static_cast<char const *>(line_caps.get_storage());
               
               line_name.setContents(
                  line_name_storage + line_caps->dwLineNameOffset,
                  line_caps->dwLineNameSize - 1);
               
               // create a record for this line
               line_info_handle info(
                  new TapiApplicationHelpers::line_info_type(
                     device_api_version,
                     line_caps->dwMediaModes,
                     line_identifier));
               line_records[line_name] = info;
               break;
            }

            case STRINGFORMAT_UNICODE:
            {
               // extract the line name
               StrUni line_name;
               wchar_t const *line_name_storage(
                  reinterpret_cast<wchar_t const *>(
                     line_caps.get_storage_bytes() + line_caps->dwLineNameOffset));
               line_name.setContents(line_name_storage, line_caps->dwLineNameSize - 1);

               // create a record for this line
               line_info_handle info(
                  new TapiApplicationHelpers::line_info_type(
                     device_api_version,
                     line_caps->dwMediaModes,
                     line_identifier));
               line_records[line_name.to_utf8()] = info;
               break;
            }
            
            default:
               throw MsgExcept("Unsupported string types");
               break;
            }
         }
      } // on_line_create


      void TapiApplication::on_line_remove(uint4 line_identifier)
      {
         // scan for the appropriate line identifier
         for(line_records_type::iterator li = line_records.begin();
             li != line_records.end();
             ++li)
         {
            if(li->second->identifier == line_identifier)
            {
               line_records.erase(li);
               break;
            }
         }
      } // on_line_remove


      void TapiApplication::get_country_info_for_id(
         uint4 country_id,
         uint4 &country_code,
         StrAsc &country_name,
         uint4 &next_country_id) const
      {
         typedef Csi::Win32::VariableSizedStruct<LINECOUNTRYLIST> local_country_list_type;
         local_country_list_type local_country_list;
         long rcd = 0;

         // get the current country information
         bool has_sufficient_storage = false;
         while(!has_sufficient_storage)
         {
            rcd = lineGetCountry(
               country_id,
               api_version,
               local_country_list.get_struct());
            if(rcd == 0 && !local_country_list.has_sufficient_storage())
               local_country_list.reallocate();
            else if(rcd != 0)
               throw TapiError(rcd);
            else
               has_sufficient_storage = true;
         }

         // windows stores the country information as a list of country records. The country
         // structure is stored at an offset from the beginning of the local_country record.
         LINECOUNTRYENTRY *local_country;
         
         local_country = reinterpret_cast<LINECOUNTRYENTRY *>(
            local_country_list.get_storage_bytes() + local_country_list->dwCountryListOffset);
         
         // we now need to initialise an entry to go into the output list
         country_code = local_country->dwCountryCode;
         country_name.setContents(
            static_cast<char *>(local_country_list.get_storage()) +
            local_country->dwCountryNameOffset,
            local_country->dwCountryNameSize - 1);
         next_country_id = local_country->dwNextCountryID;
      } // get_country_info_for_id
   };
};
