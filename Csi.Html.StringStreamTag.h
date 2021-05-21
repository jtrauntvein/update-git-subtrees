/* Csi.Html.StringStreamTag.h

   Copyright (C) 2003, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 22 January 2003
   Last Change: Tuesday 16 June 2015
   Last Commit: $Date: 2015-06-16 11:36:48 -0600 (Tue, 16 Jun 2015) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Html_StringStreamTag_h
#define Csi_Html_StringStreamTag_h

#include "Csi.Html.Tag.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class StringStreamTag
      //
      // Defines a text tag that is derived from std::ostringstream.
      ////////////////////////////////////////////////////////////
      class StringStreamTag:
         public Tag,
         public OStrAscStream
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         StringStreamTag(StrAsc const &tag_name = ""):
            Tag(tag_name)
         { }

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
         { out << str(); }
         virtual void render(std::wostream &out)
         { out << str(); }

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
      };


      typedef PolySharedPtr<Tag, StringStreamTag> StringStreamHandle;
   };
};


#endif
