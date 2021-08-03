/* Csi.DevConfig.LibraryManager.h

   Copyright (C) 2003, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 10 December 2003
   Last Change: Thursday 27 August 2015
   Last Commit: $Date: 2019-07-31 14:34:53 -0600 (Wed, 31 Jul 2019) $ 
   Last Changed by: $Author: jbritt $

*/

#ifndef Csi_DevConfig_LibraryManager_h
#define Csi_DevConfig_LibraryManager_h

#include <list>
#include "Csi.DevConfig.DeviceDesc.h"
#include "Csi.SharedPtr.h"


namespace Csi
{
   namespace DevConfig
   {
      /**
       * Defines a component that can be used to keep track of the set of
       * device description objects in the devconfig library directory.  This
       * component will initialise itself using the DevConfigLib shared
       * component registry key and will read each file in the referenced
       * directory that has a .dd extension.
       *
       * The initialisation process will be fairly expensive since it involves
       * reading the contebt of available descriptions.
       */
      class LibraryManager
      {
      public:
         /**
          * @return Evaluates the directory that should be used to locate the
          * devconfig library directory.  This will be a combination of
          * registry settings and the selected language.
          */
         static StrAsc default_library_dir();
         
         /**
          * Construct from the devconfig library directory.
          *
          * @param library_dir Specifies the directory to search for device
          * descriptions.
          */
         LibraryManager(StrAsc const &library_dir = default_library_dir());

         /**
          * Construct from an in memory device description image.
          *
          * @param dd_image Pointer to the place where the formatted device
          * description can be found.
          *
          * @param dd_image_len Specifies the number of bytes to read.
          *
          * @param library_dir Specifies the location of the devconfig library.
          */
         LibraryManager(
            void const *dd_image,
            uint4 dd_image_len,
            StrAsc const &library_dir = default_library_dir());

         /**
          * Destructor
          */
         ~LibraryManager();

         /**
          * @returns Returns an iterator to the device description identified
          * by the specified model number.
          *
          * @param model_no Specifies the model of the device to search for.
          */
         typedef SharedPtr<DeviceDesc> value_type;
         typedef std::list<value_type> descriptions_type;
         typedef descriptions_type::iterator iterator; 
         iterator get_device(StrAsc const &model_no);

         /**
          * @return Returns an iterator to the device description identified by
          * the specified devconfig device type identifier.
          *
          * @param device_type Specifies the devconfig device type identifier.
          */
         iterator get_device(uint2 device_type);

         /**
          * @return Returns the settings catalogue associated with the
          * specified devconfig device type and major version.  Returns null if
          * there is no such device type or major version.
          *
          * @param device_type Specifies the devconfig device type code to
          * search for.
          *
          * @param major_version Specifies the major version number for the
          * catalogue.
          */
         DeviceDesc::value_type get_catalog(uint2 device_type, byte major_version);

         //@group container access methods

         /**
          * @return Returns an iterator to the first device description.
          */
         iterator begin()
         { return descriptions.begin(); }

         /**
          * @return Returns an iterator beyond the last device description.
          */
         iterator end()
         { return descriptions.end(); }

         /**
          * @return Returns the number of device descriptions.
          */
         descriptions_type::size_type size() const
         { return descriptions.size(); }

         /**
          * @return Returns a reference to the first device description.
          */
         value_type const &front() const
         { return descriptions.front(); }

         /**
          * @return Returns a reference to the last device description.
          */
         value_type const &back() const
         { return descriptions.back(); }
         //@endgroup


         void add_description(            
            void const *dd_image,
            uint4 dd_image_len,
            StrAsc const &library_dir = default_library_dir());

         /**
          * @return Returns the devconfig library directory name.
          */
         StrAsc const &get_library_dir() const
         { return library_dir; }

         /**
          * @return Returns the list of device description file load errors.
          */
         typedef std::deque<StrAsc> load_errors_type;
         load_errors_type const &get_load_errors() const
         { return load_errors; }
         
      private:
         /**
          * Specifies the set of device descriptions.
          */
         descriptions_type descriptions;

         /**
          * Specifies the devconfug library directory.
          */
         StrAsc library_dir;

         /**
          * Specifies the set of errors encountered while loading device
          * descriptions.
          */
         load_errors_type load_errors;
      };
   };
};


#endif
