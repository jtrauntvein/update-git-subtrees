/* Csi.OsLoader.Sr50aLoader.h

   Copyright (C) 2016, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Friday 18 March 2016
   Last Change: Friday 18 March 2016
   Last Commit: $Date: 2016-03-21 15:01:50 -0600 (Mon, 21 Mar 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_OsLoader_Sr50aLoader_h
#define Csi_OsLoader_Sr50aLoader_h

#include "Csi.OsLoader.OsLoaderBase.h"
#include "Csi.StrAscStream.h"
#include "OneShot.h"
#include <deque>


namespace Csi
{
   namespace OsLoader
   {
      /**
       * Defines an OS Loader that uses the XON/XOFF protocol combined with a delay after every line
       * to transmit an operating system to the SR50A.
       */
      class Sr50aLoader: public OsLoaderBase, public OneShotClient
      {
      public:
         /**
          * Constructor
          *
          * @param timer_ Specifies the one shot timer that this object will use.
          */
         typedef Csi::SharedPtr<OneShot> timer_handle;
         Sr50aLoader(timer_handle &timer_);

         /**
          * Destructor
          */
         virtual ~Sr50aLoader();

         /**
          * Overloads the base class version to open the input file and validate its contents.
          */
         virtual void open_and_validate(char const *file_name);

         /**
          * Overloads the base class version to start the process of transmitting the input file to
          * the device.
          */
         virtual void start_send(driver_handle driver, EventReceiver *client);

         /**
          * Overloads the base class version to cancel the send process.
          */
         virtual void cancel_send();

         /**
          * Overloads the base class version to handle received data.
          */
         virtual void on_receive(OsLoaderDriver *driver, void const *buff, uint4 buff_len);
         
         /**
          * Overloads the one shot timer handler.
          */
         virtual void onOneShotFired(uint4 id);

      protected:
         /**
          * Overloads the base class version to clean up any timers.
          */
         virtual void on_complete(char const *message, bool succeeded, bool display_elapsed_time = false);
         
      private:
         /**
          * Transmits the next line to the device.
          */
         void send_next_line(bool advance_first = true);

      private:
         /**
          * Specifies the timer that this component will use.
          */
         timer_handle timer;

         /**
          * Specifies a collection of lines read from the file.
          */
         typedef std::deque<StrAsc> lines_type;
         lines_type lines;

         /**
          * Specifies current line number.
          */
         uint4 current_line;

         /**
          * Specifies the timer identifier for waiting between lines.
          */
         uint4 between_lines_id;

         /**
          * Set to true if we are waiting for XON.
          */
         bool waiting_for_xon;

         /**
          * Used to format messages to the application.
          */
         OStrAscStream message;

         /**
          * Stores the namer of the operating system image file.
          */
         StrAsc os_file_name;

         /**
          * Specifies the signature of the OS image.
          */
         uint2 file_sig;
      };
   };
};


#endif
