/* Csi.Html.Text.cpp

   Copyright (C) 2007, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Tuesday 25 September 2007
   Last Change: Tuesday 25 September 2007
   Last Commit: $Date: 2007-09-25 12:34:44 -0600 (Tue, 25 Sep 2007) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Html.Text.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class Text definitions
      ////////////////////////////////////////////////////////////
      Text::Text(
         StrAsc const &content_,
         StrAsc const &tag_name,
         bool allow_tags_in_content):
         Tag(tag_name)
      {
         if(allow_tags_in_content)
            content = content_;
         else
         {
            content.reserve(content_.length());
            for(size_t i = 0; i < content_.length(); ++i)
            {
               char ch = content_[i];
               switch(ch)
               {
               case '&':
                  content.append("&amp;");
                  break;
                  
               case '<':
                  content.append("&lt;");
                  break;
                  
               case '>':
                  content.append("&gt");
                  break;
                  
               default:
                  content.append(ch);
                  break;
               }
            }
         } 
      } // constructor
   };
};

