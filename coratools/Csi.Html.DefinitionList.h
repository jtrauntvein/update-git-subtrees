/* Csi.Html.DefinitionList.h

   Copyright (C) 2003, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 22 January 2003
   Last Change: Saturday 26 August 2017
   Last Commit: $Date: 2017-08-28 13:23:30 -0600 (Mon, 28 Aug 2017) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Html_DefinitionList_h
#define Csi_Html_DefinitionList_h

#include "Csi.Html.Tag.h"
#include "Csi.PolySharedPtr.h"


namespace Csi
{
   namespace Html
   {
      /**
       * Defines an HTML tag generator that represents a definition list element.
       */
      class DefinitionList: public Tag
      {
      public:
         /**
          * Default constructor
          */
         DefinitionList():
            Tag("DL")
         { }

         /**
          * Overloads the base class version to throw an exception.  List tags must be added through
          * add_definition. 
          */
         virtual tag_handle add_tag(tag_handle tag)
         {
            throw invalid_tag_operation();
         }

         /**
          * Adds a definition tag as well as its associated body tag.
          *
          * @param heading Specifies the tag for the definition heading.
          *
          * @param body Specifies the tag for the definition body.
          */
         void add_definition(
            tag_handle heading,
            tag_handle body)
         {
            Tag *dt = new Tag("DT",true);
            Tag *dd = new Tag("DD",true);
            dt->add_tag(heading);
            dd->add_tag(body);
            Tag::add_tag(dt);
            Tag::add_tag(dd);
         }
      };
      typedef PolySharedPtr<Tag, DefinitionList> DefinitionListHandle;
   };
};


#endif
