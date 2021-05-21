/* Csi.MessageWindow.h

   Copyright (C) 1998, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 09 November 1999
   Last Change: Tuesday 20 November 2012
   Last Commit: $Date: 2012-11-20 12:22:03 -0600 (Tue, 20 Nov 2012) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_MessageWindow_h
#define Csi_MessageWindow_h

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>
#include <exception>
#include "CsiTypeDefs.h"
#include "Csi.InstanceValidator.h"
#include "Csi.OsException.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class MessageWindow
   //
   // A message window is an invisible window object that is able to receive
   // messages from the windows message queue. This class defines a base for
   // specific applications for message windows. It defines a virtual handler
   // that is invoked by the winproc and also defines a static method that
   // initialises the class for all message windows.
   //////////////////////////////////////////////////////////// 
   class MessageWindow: public Csi::InstanceValidator
   {
   public:
      ////////////////////////////////////////////////////////////
      // class ExcNotInitialised
      //
      // Thrown by the constructor and post_message() when these are called
      // before initialise() is invoked
      //////////////////////////////////////////////////////////// 
      class ExcNotInitialised: public std::exception
      {
      public:
         virtual char const *what() const throw ()
         { return "Csi::MessageWindow not initialised"; }
      };

      
      ////////////////////////////////////////////////////////////
      // class ExcPostFailed
      //
      // Thrown by post_message when a post fails
      //////////////////////////////////////////////////////////// 
      class ExcPostFailed: public std::exception
      {
      public:
         virtual char const *what() const throw ()
         { return "Csi::MessageWindow post failed"; }
      };
      

      ////////////////////////////////////////////////////////////
      // class ExcCreateFailed
      //
      // Thrown by the constructor when window creation failed
      //////////////////////////////////////////////////////////// 
      class ExcCreateFailed: public OsException
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ExcCreateFailed():
            OsException("CsiMessageWindow creation failed")
         { }
      };
      
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      //////////////////////////////////////////////////////////// 
      MessageWindow(char const *window_name = "");

      ////////////////////////////////////////////////////////////
      // destructor
      //////////////////////////////////////////////////////////// 
      virtual ~MessageWindow();

      ////////////////////////////////////////////////////////////
      // post_message
      //
      // Wrapper around ::PostMessage()
      //////////////////////////////////////////////////////////// 
      void post_message(uint4 message_id, WPARAM wparam = 0, LPARAM lparam = 0);

      ////////////////////////////////////////////////////////////
      // initialise
      //
      // Static method that should be invoked before any instance of
      // MessageWindow is ever created. This method will create the window
      // class as well as initialise the app_instance static member.
      //////////////////////////////////////////////////////////// 
      static void initialise(
         HINSTANCE app_instance_,
         char const *class_name_ = "Csi::MessageWindow::DefClassName");

      ////////////////////////////////////////////////////////////
      // uninitialise
      //
      // Static method that should be invoked when the module is
      // destroyed. This method should be used particularly when initialise()
      // was invoked in DLL initialisation so that the windows class can be
      // destroyed appropriately
      //////////////////////////////////////////////////////////// 
      static void uninitialise();

      ////////////////////////////////////////////////////////////
      // get_app_instance
      //
      // Returns the module handle that was passed in when initialise() was called. 
      ////////////////////////////////////////////////////////////
      static HINSTANCE get_app_instance();

   protected:
      ////////////////////////////////////////////////////////////
      // get_window_handle
      //
      // Allows derived classes to access the window handle
      //////////////////////////////////////////////////////////// 
      HWND get_window_handle() { return window_handle; }

      ////////////////////////////////////////////////////////////
      // on_message
      //
      // Defines the handler for windows messages. Unhandled messages should be
      // delegated to this class' version of the method.
      //////////////////////////////////////////////////////////// 
      virtual LRESULT on_message(uint4 message_id, WPARAM wparam, LPARAM lparam);

   private:
      ////////////////////////////////////////////////////////////
      // window_handle
      //
      // Handle to the window object created by the constructor
      //////////////////////////////////////////////////////////// 
      HWND window_handle;

      ////////////////////////////////////////////////////////////
      // MessageWindowWndProc
      ////////////////////////////////////////////////////////////
      friend LRESULT CALLBACK MessageWindowWndProc(
         HWND window_handle,
         UINT message_id,
         WPARAM p1,
         LPARAM p2);
   };
};

#endif
