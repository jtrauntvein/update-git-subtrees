/* Csi.ResourceManager.cpp

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Saturday 13 January 2018
   Last Commit: $Date: 2018-11-16 09:52:52 -0600 (Fri, 16 Nov 2018) $
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.ResourceManager.h"
#include "Csi.Digest.h"
#include "Csi.Base64.h"
#include <iostream>
#include <deque>


namespace Csi
{
   ResourceManager::ResourceManager()
   { }


   ResourceManager::~ResourceManager()
   { }


   void ResourceManager::add_resources(ResourceInit const init[])
   {
      for(int i = 0; init[i].name.length() > 0; ++i)
      {
         ResourceInit const &res(init[i]);
         resources[res.name] = Resource(res);
      }
   } // add_resources


   void ResourceManager::generate(
      std::ostream &out, StrAsc const &key, used_keys_type &used_keys) const
   {
      // we will not generate this resource if it has already been generated
      if(used_keys.find(key) == used_keys.end())
      {
         const_iterator ri = resources.find(key);
         if(ri != resources.end())
         {
            // generate any dependencies for this resource
            Resource const &resource = ri->second;
            for(int i = 0; i < MAX_RESOURCE_DEPENDENCIES; ++i)
            {
               if(resource.dependencies[i].length() > 0)
                  generate(out, resource.dependencies[i], used_keys);
            }
            if(resource.content_len > 0 && resource.content != 0)
            {
               out.write(static_cast<char const *>(resource.content), resource.content_len);
               out << std::endl;
            }
            used_keys.insert(key);
         }
      }
   } // generate


   StrAsc ResourceManager::get_resource_date(StrAsc const &key) const
   {
      StrAsc rtn;
      std::set<StrAsc> checked;
      std::deque<StrAsc> needed;

      needed.push_back(key);
      while(!needed.empty())
      {

         StrAsc temp(needed.front());
         const_iterator ri(resources.find(temp));

         checked.insert(temp);
         needed.pop_front();
         if(ri != resources.end())
         {
            Resource const &resource(ri->second);
            for(int i = 0; i < MAX_RESOURCE_DEPENDENCIES; ++i)
            {
               StrAsc const &dependency(resource.dependencies[i]);
               if(dependency.length() == 0)
                  break;
               if(checked.find(dependency) == checked.end())
               {
                  needed.push_back(dependency);
                  checked.insert(dependency);
               }
            }
            if(resource.content_date > rtn)
               rtn = resource.content_date;
         }
      }
      return rtn;
   } // get_resource_date


   StrAsc ResourceManager::get_resource_sig(StrAsc const &key) const
   {
      // we will start by forming a list of files so that dependencies are arranged first.
      typedef std::deque<StrAsc> files_type;
      std::set<StrAsc> checked;
      files_type needed;
      files_type used;
      
      needed.push_back(key);
      while(!needed.empty())
      {
         StrAsc temp(needed.front());
         const_iterator ri(resources.find(temp));
         needed.pop_front();
         if(ri != resources.end())
         {
            Resource const &resource(ri->second);
            used.push_front(temp);
            for(int i = 0; i < MAX_RESOURCE_DEPENDENCIES; ++i)
            {
               StrAsc const &dependency(resource.dependencies[i]);
               if(dependency.length() == 0)
                  break;
               if(checked.find(dependency) == checked.end())
               {
                  needed.push_back(dependency);
                  checked.insert(dependency);
               }
            }
         }
      }

      // now that we have that list, we can iterate through it and add the signature of each file.
      StrAsc rtn;
      if(!used.empty())
      {
         Csi::Sha1Digest digest;
         
         for(files_type::const_iterator fi = used.begin(); fi != used.end(); ++fi)
         {
            const_iterator ri(resources.find(*fi));
            if(ri != resources.end())
            {
               Resource const &resource(ri->second);
               digest.add(resource.content, resource.content_len);
            }
         }
         Csi::Base64::encode(rtn, digest.final(), Csi::Sha1Digest::digest_size);
      }
      return rtn;
   } // get_resource_sig
};

