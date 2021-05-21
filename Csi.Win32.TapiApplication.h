/* Csi.Win32.TapiApplication.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 27 September 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Win32_TapiApplication_h
#define Csi_Win32_TapiApplication_h

#define WIN32_LEAN_AND_MEAN
#include "CsiTypeDefs.h"
#include "StrAsc.h"
#include "Csi.SharedPtr.h"
#include "Csi.Thread.h"
#include "Csi.Events.h"
#include <tapi.h>
#include <map>
#include <list>


namespace Csi
{
   namespace Win32
   {
      //@group class forward declarations
      namespace TapiApplicationHelpers
      {
         class line_info_type;
      };
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class TapiApplication
      //
      // Encapsulates the application layer of Microsoft TAPI version 2. Performs the appropriate
      // initialisation and provides mechanisms for accessing data about and accessing individual
      // lines.
      //
      // When this class initialises the telephony layer, it will do so specifying that it wishes to
      // use the event model. Following initialisation, it will make available methods that can be
      // used to listen for specific events. It will also make the event handle available through a
      // public accessor method that will allow the application to use it with
      // WaitForMultipleObjects().
      ////////////////////////////////////////////////////////////
      class TapiApplication:
         public Csi::EvReceiver,
         protected Csi::Thread
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         TapiApplication(HINSTANCE module_handle = 0);

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~TapiApplication();

         ////////////////////////////////////////////////////////////
         // list_line_names
         //
         // Copies the names of all of the lines into the supplied list 
         ////////////////////////////////////////////////////////////
         typedef std::list<StrAsc> line_names_type;
         void list_line_names(line_names_type &line_names);

         ////////////////////////////////////////////////////////////
         // get_api_version
         //////////////////////////////////////////////////////////// 
         uint4 get_api_version() const { return api_version; }

         ////////////////////////////////////////////////////////////
         // get_kernel_handle
         //////////////////////////////////////////////////////////// 
         HLINEAPP get_kernel_handle() { return kernel_handle; }

         ////////////////////////////////////////////////////////////
         // translate_address
         //
         // Translates the canonical address into a dialiable address using the 
         // line_name as reference.
         ////////////////////////////////////////////////////////////
         char const *translate_address(
            StrAsc &dest,
            StrAsc const &line_name,
            StrAsc const &canonical_address);
            
         ////////////////////////////////////////////////////////////
         // get_line_info
         //
         // Retrieves the information about the line named by line_name. Returns true if the
         // specified line can be found.
         ////////////////////////////////////////////////////////////
         bool get_line_info(
            StrAsc const &line_name,
            uint4 &line_identifier,
            uint4 &line_api_version);

         ////////////////////////////////////////////////////////////
         // get_line_info_by_id
         //
         // Returns the line information associated with the specified identifier. Returns true of
         // the specified information exists.
         ////////////////////////////////////////////////////////////
         bool get_line_info_by_id(
            uint4 line_identifier,
            StrAsc &line_name,
            uint4 &line_api_version);

         ////////////////////////////////////////////////////////////
         // list_countries
         ////////////////////////////////////////////////////////////
         struct country_info_type
         {
            ////////////////////////////////////////////////////////////
            // country_code
            //
            // The code that should be dialed to select this country
            //////////////////////////////////////////////////////////// 
            uint4 country_code;

            ////////////////////////////////////////////////////////////
            // country_name
            //
            // The name of the country associated with this code. 
            //////////////////////////////////////////////////////////// 
            StrAsc country_name;

            ////////////////////////////////////////////////////////////
            // constructor
            //////////////////////////////////////////////////////////// 
            country_info_type(
               uint4 country_code_,
               char const *country_name_,
               uint4 country_name_len):
               country_code(country_code_)
            { country_name.setContents(country_name_,country_name_len); } 
         };
         typedef std::list<country_info_type> country_list_type;
         void list_countries(country_list_type &countries) const;

         ////////////////////////////////////////////////////////////
         // get_default_country_info
         //
         // Returns the default country information associated with the host computer.
         ////////////////////////////////////////////////////////////
         void get_default_location_info(
            uint4 &country_code,
            StrAsc &country_name,
            StrAsc &area_code);

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         //@group methods overloaded from class Csi::Thread
         ////////////////////////////////////////////////////////////
         // wait_for_end
         ////////////////////////////////////////////////////////////
         virtual void wait_for_end();
         
         ////////////////////////////////////////////////////////////
         // execute
         //
         // Overloads Thread::execute(). This method will wait for events to arrive from the OS
         // regarding TAPI device status.
         ////////////////////////////////////////////////////////////
         virtual void execute();
         //@endgroup

         ////////////////////////////////////////////////////////////
         // on_line_create
         //
         // Called when TAPI has informed the application that a line has been added.
         ////////////////////////////////////////////////////////////
         virtual void on_line_create(uint4 line_identifier);

         ////////////////////////////////////////////////////////////
         // on_line_remove
         //
         // Called when TAPi has posted an event that a line has been removed
         ////////////////////////////////////////////////////////////
         virtual void on_line_remove(uint4 line_identifier);

      private:
         ////////////////////////////////////////////////////////////
         // get_country_info_for_id
         //
         // Returns the current country information for the specified ID 
         ////////////////////////////////////////////////////////////
         void get_country_info_for_id(
            uint4 country_id,
            uint4 &country_code,
            StrAsc &country_name,
            uint4 &next_country_id) const;
         
      private:
         ////////////////////////////////////////////////////////////
         // line_records
         //
         // List of all modems known at the time the constructor was called.
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<TapiApplicationHelpers::line_info_type> line_info_handle;
         typedef std::map<StrAsc, line_info_handle> line_records_type;
         line_records_type line_records;

         ////////////////////////////////////////////////////////////
         // kill_event
         //
         // Used to force the thread to stop.
         ////////////////////////////////////////////////////////////
         Csi::Win32::Condition kill_event;

         ////////////////////////////////////////////////////////////
         // kernel_event
         //
         // Handle to the Win32 event kernel object that is allocated when the TAPI layer is
         // initialised.
         ////////////////////////////////////////////////////////////
         HANDLE kernel_event; 

         ////////////////////////////////////////////////////////////
         // kernel_handle
         //
         // Handle to the kernel resources constructed for this class when the telephony layer is
         // initialised. This handle is used either directly or indirectly for all subsequent TAPI
         // calls.
         ////////////////////////////////////////////////////////////
         HLINEAPP kernel_handle;

         ////////////////////////////////////////////////////////////
         // api_version
         //
         // Stores the TAPI version number being used with the major version stored in the most
         // significant 16 bits and the minor version number stored in the least significant 16
         // bits.
         ////////////////////////////////////////////////////////////
         uint4 api_version;
      };
   };
};

#endif
