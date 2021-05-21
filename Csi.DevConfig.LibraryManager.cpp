/* Csi.DevConfig.LibraryManager.cpp

   Copyright (C) 2003, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 10 December 2003
   Last Change: Monday 09 November 2020
   Last Commit: $Date: 2020-11-09 16:50:05 -0600 (Mon, 09 Nov 2020) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include <algorithm>
#include "Csi.DevConfig.LibraryManager.h"
#include "Csi.FileSystemObject.h"
#include "Csi.RegistryManager.h"
#include "Csi.Utils.h"
#include "Csi.StringLoader.h"
#include "Csi.StrAscStream.h"
#include "Csi.MsgExcept.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // obsolete_names
         //
         // Specifies the list of model numbers that should not be loaded from the library.  These
         // device descriptions should be removed from the install but the install does not remove
         // unused files unless the program is uninstalled.  
         ////////////////////////////////////////////////////////////
         StrAsc const obsolete_names[] =
         {
            "CS150",
            "CWS650",
            "NL250",
            "CR2",
            "CR600",
            "TDR200",
            "CR9",
            "CR9 Series",
            "CS660",
            "CC640",
            ""
         };
         bool is_obsolete_model(StrAsc const &model)
         {
            bool rtn = false;
            for(int i = 0; !rtn && obsolete_names[i].length() > 0; ++i)
               if(obsolete_names[i] == model)
                  rtn = true;
            return rtn;
         }
      };

      
      ////////////////////////////////////////////////////////////
      // class LibraryManager definitions
      ////////////////////////////////////////////////////////////
      LibraryManager::LibraryManager(StrAsc const &library_dir_):
         library_dir(library_dir_)
      {
         // we want to get the list of all .dd files in the library directory.  
         FileSystemObject dir(library_dir.c_str());
         FileSystemObject::children_type children;

         dir.get_children(children,"*.dd");

         // we can now attempt to build a device description object from each of these file names
         for(FileSystemObject::children_type::const_iterator ci = children.begin();
             ci != children.end();
             ++ci)
         {
            try
            {
               value_type device(
                  new DeviceDesc(
                     ci->get_complete_name(),
                     ci->get_path()));
               if(!is_obsolete_model(device->get_model_no()))
                  descriptions.push_back(device);
            }
            catch(std::exception &e)
            {
               OStrAscStream msg;
               msg << "\"error reading dd\",\"" << ci->get_name() << "\",\"" << e.what() << "\"";
               load_errors.push_back(msg.str());
            }
         }
      } // constructor


      LibraryManager::LibraryManager(
         void const *dd_image,
         uint4 dd_image_len,
         StrAsc const &library_dir_):
         library_dir(library_dir_)
      {
         descriptions.push_back(
            new DeviceDesc(dd_image, dd_image_len, library_dir));
      } // constructor (from device image)


      LibraryManager::~LibraryManager()
      { }


      void LibraryManager::add_description(
         void const *dd_image,
         uint4 dd_image_len,
         StrAsc const &library_dir)
      {
         descriptions.push_back(
            new DeviceDesc(dd_image, dd_image_len, library_dir));
      } // constructor (from device image)


      namespace
      {
         ////////////////////////////////////////////////////////////
         // class device_has_model_no
         ////////////////////////////////////////////////////////////
         class device_has_model_no
         {
         private:
            ////////////////////////////////////////////////////////////
            // model_no
            ////////////////////////////////////////////////////////////
            StrAsc model_no;

         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            device_has_model_no(StrAsc const &model_no_):
               model_no(model_no_)
            { }

            ////////////////////////////////////////////////////////////
            // evaluation operator
            ////////////////////////////////////////////////////////////
            bool operator() (SharedPtr<DeviceDesc> const &device) const
            {
               bool rtn(device->get_model_no(true) == model_no ||
                        device->get_model_no(false) == model_no);
               return rtn;
            }
         };
      };

    
      LibraryManager::iterator LibraryManager::get_device(StrAsc const &model_no)
      {
         return std::find_if(
            descriptions.begin(),
            descriptions.end(),
            device_has_model_no(model_no));
      } // get_device



      namespace
      {
         ////////////////////////////////////////////////////////////
         // class device_has_type
         ////////////////////////////////////////////////////////////
         class device_has_type
         {
         private:
            uint2 type;

         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            device_has_type(uint2 type_):
               type(type_)
            { }

            ////////////////////////////////////////////////////////////
            // evaluation operator
            ////////////////////////////////////////////////////////////
            bool operator ()(SharedPtr<DeviceDesc> const &device) const
            { return device->get_device_type() == type; }
         };
      };

      
      LibraryManager::iterator LibraryManager::get_device(uint2 type)
      {
         return std::find_if(
            descriptions.begin(),
            descriptions.end(),
            device_has_type(type)); 
      } // get_device


      DeviceDesc::value_type LibraryManager::get_catalog(
         uint2 device_type, byte major_version)
      {
         DeviceDesc::value_type rtn;
         iterator di = get_device(device_type);
         if(di != end())
         {
            value_type &desc = *di;
            DeviceDesc::iterator ci = desc->find_catalog(major_version);
            if(ci != desc->end())
               rtn = *ci;
         }
         return rtn;
      } // get_catalog


      StrAsc LibraryManager::default_library_dir()
      {
         // we will start out getting the name of the english directory.  We do this in order to
         // maintain compatibility with older clients that require the library.
         StrAsc english_dir;
         StrAsc parent_dir;
         StrAsc grandparent_dir;
         RegistryManager::read_shared_value(
            english_dir,
            "DevConfigLib");
         if(english_dir.length() == 0)
         {
            std::deque<StrAsc> path;
            get_app_dir(english_dir);
            path.push_back(english_dir);
            path.push_back("library");
            path.push_back("en");
            path.push_back("v2");
            english_dir = Csi::join_path(path.begin(), path.end());
         }
         else
         {
            std::deque<StrAsc> path;
            path.push_back(english_dir);
            path.push_back("v2");
            english_dir = Csi::join_path(path.begin(), path.end());
         }
         get_path_from_file_name(parent_dir, english_dir.c_str());
         get_path_from_file_name(grandparent_dir, parent_dir.c_str());

         // we will scan the parent of the english directory for all child sub-directories and
         // attempt to convert these names into language identifiers.  These must be compared
         // against the preferred language identifier (obtained from the StringLoader namespace)
         StrAsc best_dir(english_dir);
         uint4 best_language_id = 1033;
         FileSystemObject grandparent(grandparent_dir.c_str());
         FileSystemObject::children_type children;
         uint4 preferred_language_id = StringLoader::get_os_preferred_language_id(); 

         grandparent.get_children(children);
         while(!children.empty())
         {
            FileSystemObject &child = children.front();
            if(child.is_directory() && child.get_name() != "." && child.get_name() != "..")
            {
               uint4 candidate_id = StringLoader::rfc1766_to_languageid(child.get_name());
               bool is_better = StringLoader::is_better_languageid(candidate_id, preferred_language_id, best_language_id);
               if(is_better)
               {
                  std::deque<StrAsc> path;
                  path.push_back(child.get_complete_name());
                  path.push_back("v2");
                  best_dir = join_path(path.begin(), path.end());
                  best_language_id = candidate_id;
               } 
            }
            children.pop_front();
         }
         return best_dir;
      } // default_library_dir
   };
};

