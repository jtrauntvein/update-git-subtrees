/* MyApp.cpp

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 12 February 2015
   Last Change: Wednesday 18 February 2015
   Last Commit: $Date: 2015-02-18 14:34:22 -0600 (Wed, 18 Feb 2015) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "MyApp.h"
#include "MyFrame.h"
#include "Csi.Win32Dispatch.h"


bool MyApp::OnInit()
{
   // perform event dispatcher initialisation.
   Csi::MessageWindow::initialise(::GetModuleHandle(0));
   Csi::Event::set_dispatcher(new Csi::Win32Dispatch);
   
   // create the test frame
   MyFrame *frame(new MyFrame);
   SetTopWindow(frame);
   frame->Show();
   return true;
}


int MyApp::OnExit()
{
   Csi::Event::set_dispatcher(0);
   Csi::MessageWindow::uninitialise();
   return 0;
}


IMPLEMENT_APP(MyApp)
