/* Csi.MessageWindow.cpp

   Copyright (C) 1998, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 09 November 1999
   Last Change: Thursday 26 September 2019
   Last Commit: $Date: 2019-09-26 14:46:19 -0600 (Thu, 26 Sep 2019) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.MessageWindow.h"
#include "Csi.OsException.h"
#include "StrUni.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <typeinfo>


namespace Csi
{
   namespace
   {
      ////////////////////////////////////////////////////////////
      // app_instance
      ////////////////////////////////////////////////////////////
      HINSTANCE app_instance;

      ////////////////////////////////////////////////////////////
      // is_initialised
      ////////////////////////////////////////////////////////////
      bool is_initialised;

      ////////////////////////////////////////////////////////////
      // class_name
      ////////////////////////////////////////////////////////////
      uint4 const class_name_max_len(256);
      wchar_t class_name[class_name_max_len];
   };
   
   ////////////////////////////////////////////////////////////
   // class MessageWindow definitions
   ////////////////////////////////////////////////////////////
   MessageWindow::MessageWindow(char const *window_name_):
      window_handle(0)
   {
      if(is_initialised)
      {
         // create the window handle
         StrUni window_name(window_name_, true);
         window_handle = CreateWindowExW(
            0,// no extended windows styles
            class_name, // use the class registered by initialise()
            window_name.c_str(),
            WS_POPUP, // this window will be popup
            0,0, // (x,y)
            0,0, // (width,height)
            0, // default parent
            0, // no menu
            app_instance,
            0); // no creation parameters
         if(IsWindow(window_handle))
         {
            // set the window user data to point to this object
            SetWindowLongPtr(window_handle, GWLP_USERDATA, (LONG_PTR)this);
         }
         else
            throw ExcCreateFailed();
      }
      else
         throw ExcNotInitialised();
   } // constructor


   MessageWindow::~MessageWindow()
   {
      if(IsWindow(window_handle))
         DestroyWindow(window_handle);
   } // destructor


   LRESULT MessageWindow::on_message(uint4 message_id, WPARAM wparam, LPARAM lparam)
   { return DefWindowProc(window_handle,message_id,wparam,lparam); }


   void MessageWindow::post_message(uint4 message_id, WPARAM p1, LPARAM p2)
   {
      if(is_initialised && IsWindow(window_handle))
      {
         if(!::PostMessage(window_handle,message_id,p1,p2))
            throw ExcPostFailed();
      }
      else
         throw ExcNotInitialised();
   } // post_message


#pragma warning(disable: 4996)
   void MessageWindow::initialise(
      HINSTANCE app_instance_,
      char const *class_name_)
   {
      if(!is_initialised)
      {
         // initialise the static members
         StrUni name(class_name_, true);
         app_instance = app_instance_;
         memset(class_name,0,sizeof(class_name));
         wcsncpy(class_name, name.c_str(), class_name_max_len - 1);
         is_initialised = true;

         // register the windows class used
         WNDCLASSEXW window_class;

         memset(&window_class,0,sizeof(window_class));
         window_class.cbSize = sizeof(window_class);
         window_class.hInstance = app_instance;
         window_class.lpszClassName = class_name;
         window_class.cbWndExtra = sizeof(MessageWindow *);
         window_class.lpfnWndProc = MessageWindowWndProc;
         if(!RegisterClassExW(&window_class))
         {
            is_initialised = false;
            throw OsException("Csi::MessageWindow::initialise() failed");
         }
      }
   } // initialise
#pragma warning(default: 4996)


   void MessageWindow::uninitialise()
   {
      if(is_initialised)
      {
         is_initialised = false;
         if(!UnregisterClassW(class_name,app_instance))
            throw OsException("Csi::MessageWindow::uninitialise() failed");
      }
   } // uninitialise


   HINSTANCE MessageWindow::get_app_instance()
   { return app_instance; }
   

   int eval_structured_exception(
      HWND window_handle,
      UINT message_id,
      WPARAM p1,
      LPARAM p2);


   LRESULT CALLBACK MessageWindowWndProc(
      HWND window_handle,
      UINT message_id,
      WPARAM p1,
      LPARAM p2)
   {
      LRESULT rtn = 0;
      __try {
         LONG_PTR user_data;
         if(::IsWindow(window_handle) &&
            (user_data = GetWindowLongPtr(window_handle, GWLP_USERDATA)))
         {
            MessageWindow *mwin = reinterpret_cast<MessageWindow *>(user_data);
            if(MessageWindow::is_valid_instance(mwin) &&
               mwin->window_handle == window_handle)
               rtn = mwin->on_message(message_id,p1,p2);
         }
         else
            rtn = DefWindowProc(window_handle,message_id,p1,p2);
      }
      __except(eval_structured_exception(window_handle,message_id,p1,p2)) { }
      return rtn; 
   } // MessageWindowWndProc


   int eval_structured_exception(
      HWND window_handle,
      UINT message_id,
      WPARAM p1,
      LPARAM p2)
   {
      // now try to open the rpt file
      FILE *rpt_file = open_rpt_file();
      if(rpt_file)
      {
         fprintf(
            rpt_file,
            "////////////////////////////////////////////////////////////\n"
            "MessageWindowWndProc caught a structured exception\n");
         fprintf(rpt_file,"  window_handle: %p\n",window_handle);
         fprintf(rpt_file,"  message: %u (0x%x)\n",message_id,message_id);
         fprintf(rpt_file,"  wparam: %u (0x%x)\n",p1,p1);
         fprintf(rpt_file,"  lparam: %u (0x%x)\n",p2,p2);

         fprintf(rpt_file,"Window details:\n");
         if(::IsWindow(window_handle))
         {
            char window_class_name[256];
            char window_text[256];
            
            LONG_PTR user_data = GetWindowLongPtr(window_handle, GWLP_USERDATA);
            GetClassNameA(window_handle,window_class_name,sizeof(window_class_name));
            GetWindowTextA(window_handle,window_text,sizeof(window_text));
            fprintf(rpt_file,"  user data: %u (0x%x)\n",user_data,user_data);
            fprintf(rpt_file,"  class name: %s\n",window_class_name);
            fprintf(rpt_file,"  window text: %s\n",window_text); 
            if(user_data)
            {
               MessageWindow *mwin = reinterpret_cast<MessageWindow *>(user_data);
               if(MessageWindow::is_valid_instance(mwin))
               {
                  fprintf(rpt_file,"  user data type: %s\n",typeid(*mwin).name());
               }
               else
                  fprintf(rpt_file,"  user data is NOT a valid instance of MessageWindow");
            }
         }
         else
            fprintf(rpt_file,"  window handle is NOT a window\n");
         fclose(rpt_file);
      }

      // we want the rest of the handlers to work with the exception
      return EXCEPTION_CONTINUE_SEARCH;
   } // eval_structured_exception 
};
