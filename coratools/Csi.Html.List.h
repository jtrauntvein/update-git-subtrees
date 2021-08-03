/* Csi.Html.List.h

   Copyright (C) 2007, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 11 January 2007
   Last Change: Thursday 26 May 2011
   Last Commit: $Date: 2011-05-26 10:28:31 -0600 (Thu, 26 May 2011) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_Html_List_h
#define Csi_Html_List_h

#include "Csi.Html.Tag.h"
#include "Csi.PolySharedPtr.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class List
      //
      // Represents an ordered or unordered list element in the HTML document. 
      ////////////////////////////////////////////////////////////
      class List: public Tag
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         List(bool ordered = false):
            Tag(ordered ? "ol" : "ul")
         { }

         ////////////////////////////////////////////////////////////
         // add_tag
         ////////////////////////////////////////////////////////////
         virtual tag_handle add_tag(tag_handle tag)
         {
            tag_handle item_tag(Tag::add_tag(new Tag("li")));
            return item_tag->add_tag(tag);
         }
      };
      typedef PolySharedPtr<Tag, List> ListHandle; 
   };
};


#endif
