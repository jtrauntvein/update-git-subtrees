/* MyApp.h

   Copyright (C) 2015, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Thursday 12 February 2015
   Last Change: Thursday 12 February 2015
   Last Commit: $Date: 2015-02-13 10:38:20 -0600 (Fri, 13 Feb 2015) $
   Last Changed by: $Author: jon $

*/

#ifndef MyApp_h
#define MyApp_h
#include "wxlib_config.h"
#include "wx/wx.h"


/**
 * Defines an application class for an application that can test the graph component.
 */
class MyApp: public wxApp
{
public:
   /**
    * Overloads the base class version to initialise this application.
    */
   virtual bool OnInit();

   /**
    * Overloads the base class version to clean up this application.
    */
   virtual int OnExit();
};


DECLARE_APP(MyApp)


#endif
