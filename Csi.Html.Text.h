/* Csi.Html.Text.h

   Copyright (c) 2000, 2007 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 3 May 2000
   Last Change: Tuesday 25 September 2007
   Last Commit: $Date: 2012-11-20 12:24:32 -0600 (Tue, 20 Nov 2012) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Html_Text_h
#define Csi_Html_Text_h

#include "Csi.Html.Tag.h"
#include "StrAsc.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class Text
      //
      // Represents a sequence of plain text in an HTML document
      ////////////////////////////////////////////////////////////
      class Text: public Tag
      {
      protected:
         ////////////////////////////////////////////////////////////
         // content
         ////////////////////////////////////////////////////////////
         StrAsc content;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Text(
            StrAsc const &content_ = "",
            StrAsc const &tag_name = "",
            bool allow_tags_in_content = true);

         ////////////////////////////////////////////////////////////
         // add_attribute
         ////////////////////////////////////////////////////////////
         virtual void add_attribute(
            StrAsc const &name,
            StrAsc const &value)
         { throw invalid_tag_operation(); }

         ////////////////////////////////////////////////////////////
         // add_tag
         ////////////////////////////////////////////////////////////
         virtual tag_handle add_tag(tag_handle tag)
         { throw invalid_tag_operation(); }

         ////////////////////////////////////////////////////////////
         // before_render
         ////////////////////////////////////////////////////////////
         virtual bool before_render(std::ostream &out)
         {
            if(tag_name.length())
               Tag::before_render(out);
            return true;
         }
         virtual bool before_render(std::wostream &out)
         {
            if(tag_name.length())
               Tag::before_render(out);
            return true;
         }

         ////////////////////////////////////////////////////////////
         // render
         ////////////////////////////////////////////////////////////
         virtual void render(std::ostream &out)
         { out << content; }
         virtual void render(std::wostream &out)
         { out << content; }

         ////////////////////////////////////////////////////////////
         // after_render
         ////////////////////////////////////////////////////////////
         virtual void after_render(std::ostream &out)
         {
            if(tag_name.length())
               Tag::after_render(out);
         }
         virtual void after_render(std::wostream &out)
         {
            if(tag_name.length())
               Tag::after_render(out);
         }

         ////////////////////////////////////////////////////////////
         // set_content
         ////////////////////////////////////////////////////////////
         virtual void set_content(StrAsc const &content_)
         { content = content_; }
      };
   };
};


#endif
