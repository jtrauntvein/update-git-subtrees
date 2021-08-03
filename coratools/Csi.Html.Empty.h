/* Csi.Html.Empty.h

   Copyright (C) 2003, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 03 February 2003
   Last Change: Tuesday 20 November 2012
   Last Commit: $Date: 2012-11-20 12:24:32 -0600 (Tue, 20 Nov 2012) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Html_Empty_h
#define Csi_Html_Empty_h

#include "Csi.Html.Tag.h"
#include "Csi.PolySharedPtr.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class Empty
      //
      // Defines a class of tag that is meant solely to hold other tags.  It does no formatting on
      // its own but is merely a placeholder for its children. 
      ////////////////////////////////////////////////////////////
      class Empty: public Tag
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Empty():
            Tag("",true)
         { }

         ////////////////////////////////////////////////////////////
         // add_attribute
         ////////////////////////////////////////////////////////////
         virtual void add_attribute(char const *name, char const *value)
         { throw invalid_tag_operation(); }

         ////////////////////////////////////////////////////////////
         // before_render
         ////////////////////////////////////////////////////////////
         virtual bool before_render(std::ostream &out)
         { return true; }
         virtual bool before_render(std::wostream &out)
         { return true; }
      };
      typedef PolySharedPtr<Tag, Empty> EmptyHandle;
   };
};


#endif
