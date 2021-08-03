/* Csi.Html.Tag.h

   Copyright (C) 2000, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 2 May 2000
   Last Change: Tuesday 20 November 2012
   Last Commit: $Date: 2012-11-20 12:24:32 -0600 (Tue, 20 Nov 2012) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Html_Tag_h
#define Csi_Html_Tag_h

#include "Csi.SharedPtr.h"
#include "StrAsc.h"
#include <iostream>
#include <list>
#include <map>



namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class Tag
      //
      // Declares a base class for all HTML tags that can appear in an HTML
      // document. All tags including the HTML document type itself should
      // inherit from this class.
      //
      // This class both represents an HTML tag and can be a container for
      // other tags. This is therefore a recursive structure.
      ////////////////////////////////////////////////////////////
      class Tag
      {
      public:
         ////////////////////////////////////////////////////////////
         // typedef tag_handle
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Tag> tag_handle;
         typedef tag_handle value_type;

         ////////////////////////////////////////////////////////////
         // class invalid_tag_operation
         //
         // Defines a class of exception that can be thrown by a derived class
         // when an invalid operation is attempted.
         ////////////////////////////////////////////////////////////
         class invalid_tag_operation: public std::exception
         {
         public:
            virtual char const *what() const throw ()
            { return "An invalid operation was attempted on a tag"; }
         };
      
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Tag(
            StrAsc const &tag_name_ = "",
            bool on_same_line_ = false):
            tag_name(tag_name_),
            on_same_line(on_same_line_)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Tag()
         { }

         ////////////////////////////////////////////////////////////
         // on_create
         //
         // Called after the construction of the tag object has completed. Most
         // of the building should take place here rather than in the
         // constructor because virtual methods will not work as expected when
         // called from the constructor
         ////////////////////////////////////////////////////////////
         virtual void on_create()
         { }

         ////////////////////////////////////////////////////////////
         // add_attribute
         //
         // Adds an attribute that will be printed inside braces along with the
         // tag name
         ////////////////////////////////////////////////////////////
         virtual void add_attribute(
            StrAsc const &name,
            StrAsc const &value)
         { attributes[name] = value; }

         ////////////////////////////////////////////////////////////
         // add_tag
         //
         // Appends a tag to the end of the list of tags that will be rendered
         // when render() is invoked.  Because the parameter is defined as a copy
         // rather than a reference, any raw pointers passed in will be converted
         // to shared pointers.
         ////////////////////////////////////////////////////////////
         virtual tag_handle add_tag(tag_handle tag)
         {
            tags.push_back(tag);
            return tag;
         } // add_tag

         ////////////////////////////////////////////////////////////
         // add_tag
         //
         // Creates a tag with the specified name as a child of this tag.  The
         // created tag will be the return value.
         ////////////////////////////////////////////////////////////
         virtual tag_handle add_tag(
            StrAsc const &tag_name = "",
            bool on_same_line = false)
         {
            tag_handle rtn(new Tag(tag_name,on_same_line));
            add_tag(rtn);
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // before_render
         //
         // Called before render() is invoked to write the first part of the
         // tag to the stream. The return value will be true if the rest of the
         // tag should be rendered. This default version will write "<tag_name"
         // followed by each of the values followed in turn by ">\n" and will
         // return true unless tag_name is an empty string.
         ////////////////////////////////////////////////////////////
         virtual bool before_render(std::ostream &out);
         virtual bool before_render(std::wostream &out);

         ////////////////////////////////////////////////////////////
         // render
         //
         // called to render the body of the tag. The default version will call
         // before_render(), render(), and after_render() for each of the tags
         // that this tag contains.
         ////////////////////////////////////////////////////////////
         virtual void render(std::ostream &out);
         virtual void render(std::wostream &out);

         ////////////////////////////////////////////////////////////
         // after_render
         //
         // Called to terminate the tag in the output. This default version
         // will write </tag_name> followed by a line feed.
         ////////////////////////////////////////////////////////////
         virtual void after_render(std::ostream &out);
         virtual void after_render(std::wostream &out);

         // @group: tag container declarations

         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef std::list<tag_handle> tags_type;
         typedef tags_type::iterator iterator;
         typedef tags_type::const_iterator const_iterator;
         iterator begin()
         { return tags.begin(); }
         const_iterator begin() const
         { return tags.end(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end()
         { return tags.end(); }
         const_iterator end() const
         { return tags.end(); }

         ////////////////////////////////////////////////////////////
         // clear
         ////////////////////////////////////////////////////////////
         void clear()
         { tags.clear(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return tags.empty(); }
         
         // @endgroup:

      protected:
         ////////////////////////////////////////////////////////////
         // tags
         //
         //  Defines the list of tags that are arranged under this tag
         //////////////////////////////////////////////////////////// 
         tags_type tags;

         ////////////////////////////////////////////////////////////
         // attributes
         //
         // Defines the list of attributes that will be printed with this tag
         ////////////////////////////////////////////////////////////
         typedef std::map<StrAsc, StrAsc> attributes_type;
         attributes_type attributes;

         ////////////////////////////////////////////////////////////
         // tag_name
         ////////////////////////////////////////////////////////////
         StrAsc tag_name;

         ////////////////////////////////////////////////////////////
         // on_same_line
         //
         // Set to true if the tag body should be written to the same line as
         // the opening and closing markers
         ////////////////////////////////////////////////////////////
         bool on_same_line;
      };
   };
};


#endif
