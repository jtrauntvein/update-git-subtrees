/* Csi.Posix.FileSystemObject.h

   Copyright (C) 2005, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 14 April 2005
   Last Change: Tuesday 05 March 2019
   Last Commit: $Date: 2019-03-05 17:29:13 -0600 (Tue, 05 Mar 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Posix_FileSystemObject_h
#define Csi_Posix_FileSystemObject_h

#include <list>
#include "StrAsc.h"
#include "Csi.LgrDate.h"
#include "CsiTypeDefs.h"


namespace Csi
{
   namespace Posix
   {
      /**
       * Defines an object that manages metadata for a file or directory in a file system.  Metadata
       * includes the name of the file or directory, its path, and other attributes such as creation
       * time and last modification time.  It also provides methods that allow the application to
       * get a list of children within a directory with or without wildcards.
       */
      class FileSystemObject
      {
      public:
         /**
          * Set to true if this object references a valid file system entity.
          */
         bool is_valid;

         /**
          * Specifies the name of this object within the parent directory.
          */
         StrAsc name;

         /**
          * Specifies the path to this object.  if this object represents the root, the path will be
          * the same as the name.
          */
         StrAsc path;

         /**
          * Represents a bitmap that represents object attributes.
          */
         enum flag_values_type
         {
            flag_root = 0x0001,
            flag_directory = 0x0002,
            flag_read_only = 0x0004,
            flag_hidden = 0x0008,
            flag_system = 0x0010,
            flag_temporary = 0x0020,
            flag_compressed = 0x0040,
            flag_encrypted = 0x0080,
            flag_offline = 0x0100,
            flag_symlink = 0x200
         };
         uint4 flags;

         /**
          * Specifies the date when the file or directory was created.
          */
         LgrDate creation_time;

         /**
          * Specifies the date and time when the file or directory was last read.
          */
         LgrDate last_read_date;

         /**
          * Specifies the date and time when the file or directory was last modified.
          */
         LgrDate last_write_date;

         /**
          * Represents the size of the file in bytes.
          */
         int8 size;
         
      public:
         // @group: creation and assignment

         /**
          * Creates the object in an oinvalid state.  It can be rendered usable with the assignment
          * or the copy operators.
          */
         FileSystemObject();

         /**
          * Creates the object using the supplied path.  Uses this path to look up metadata from the
          * operating system.  If the path is is not valid, the is_valid flag will be set to false.
          *
          * @param object_name_ Specifies the path of the file or directory to be located.
          */
         FileSystemObject(char const *object_name_);

         /**
          * Copy constructor
          *
          * @param other Specifies the file system object to copy.
          */
         FileSystemObject(FileSystemObject const &other);

         /**
          * Destructor
          */
         virtual ~FileSystemObject();

         /**
          * Copy operator.
          *
          * @param other Specifies the object to copy.
          */
         FileSystemObject &operator =(FileSystemObject const &other);

         /**
          * String assignment operator.
          *
          * @param object_name_ Specifies the path of the object.
          */
         FileSystemObject &operator =(char const *object_name_);

         // @endgroup:

         // @group: meta-data access methods

         /**
          * @return Returns true if the object is describing a valid file system object.
          */
         bool get_is_valid() const
         { return is_valid; }

         /**
          * @return Returns the name of the file system object without the path.
          */
         StrAsc const &get_name() const
         { return name; }

         /**
          * @return Returns the path of the file system object without the name.
          */
         StrAsc const &get_path() const
         { return path; }

         /**
          * @return Returns both the name and the path formatted as a string.
          */
         StrAsc get_complete_name() const
         {
            StrAsc rtn(path);

            if(!is_root())
            {
               if(rtn.last() != '/')
                  rtn += '/';
               rtn += name;
            }
            return rtn;
         }

         /**
          * @return Returns the boolean attributes bitmap for the object.
          */
         uint4 get_flags() const
         { return flags; }

         /**
          * @return Returns true if this object is a root level directory.
          */
         bool is_root() const
         { return (flags & flag_root) != 0; }

         /**
          * @return Returns true if this object is a directory.
          */
         bool is_directory() const
         { return (flags & flag_directory) != 0; }

         /**
          * @return Returns true if this object is marked as read-only.
          */
         bool is_read_only() const
         { return (flags & flag_read_only) != 0; }

         /**
          * @return Returns true if this object is marked as hidden.
          */
         bool is_hidden() const
         { return (flags & flag_hidden) != 0; }

         /**
          * @return Returns true if this object belongs to the operating system.
          */
         bool is_system() const
         { return (flags & flag_system) != 0; }

         /**
          * @return Returns true if this object is marked as temporary.
          */
         bool is_temporary() const
         { return (flags & flag_temporary) != 0; }

         /**
          * @return Returns true if this object is marked as compressed.
          */
         bool is_compressed() const
         { return (flags & flag_compressed) != 0; }

         /**
          * @return Returns true if this object is marked as encrypted.
          */
         bool is_encrypted() const
         { return (flags & flag_encrypted) != 0; }


         /**
          * @return Returns true if this object is marked as off-line.
          */
         bool is_offline() const
         { return (flags & flag_offline) != 0; }

         /**
          * @return Returns true if the object represents a symbolic link.
          */
         bool is_symlink() const
         { return (flags & flag_symlink) != 0; }

         /**
          * @return Returns the date and time when this object was created.
          */
         LgrDate const &get_creation_time() const
         { return creation_time; }

         /**
          * @return Returns the date and time when this object was last read.
          */
         LgrDate const &get_last_read_date() const
         { return last_read_date; }

         /**
          * @return Returns the date and time when this object was last modified.
          */
         LgrDate const &get_last_write_date() const
         { return last_write_date; }

         /**
          * @return Returns the size of the object in bytes.
          */
         int8 get_size() const
         { return size; }

         // @endgroup:

         /**
          * Returns information about the volume on which this object resides.
          *
          * @param totlal_size Reference to a value to which the volume size will be written.
          *
          * @param free_space Reference to a value to which the free space on the volume will be
          * written.
          */
         void get_volume_info(
            int8 &total_size,
            int8 &free_space);

         /**
          * Generates a list of children of a directory object.
          *
          * @param children Specifies the container provided by the application in which the list of
          * children objects will be added.
          *
          * @param mark Specifies the mask expression to filter the list of children.  If omitted,
          * this defaults to select all children.
          */
         typedef std::list<FileSystemObject> children_type;
         void get_children(
            children_type &children,
            char const *mask = "*");

         /**
          * @return Returns the root volume for this file system object.
          */
         StrAsc get_root() const
         { return "/"; }

         /**
          * @return Returns the character used to separate objects in a path.
          */
         static char dir_separator()
         { return '/'; }

      private:
         /**
          * Performs the actual work of looing up the object metadata.
          */
         void set_object_name(char const *object_name);
      };
   };
};


#endif
