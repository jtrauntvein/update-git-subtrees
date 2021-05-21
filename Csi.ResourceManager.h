/* Csi.ResourceManager.h

   Copyright (C) 2010, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Friday 17 May 2019
   Last Commit: $Date: 2019-05-17 14:33:23 -0600 (Fri, 17 May 2019) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_ResourceManager_h
#define Csi_ResourceManager_h

#include "StrAsc.h"
#include "Csi.Digest.h"
#include <map>
#include <set>


#define MAX_RESOURCE_DEPENDENCIES 16


namespace Csi
{
   /**
    * Defines a sturcture that can be used to initialise a resource.
    */
   struct ResourceInit
   {
      /**
       * Specifies the file name for the resource.
       */
      StrAsc const name;

      /**
       * Specifies the content for the resource.
       */
      void const *content;

      /**
       * Specifies the size of the resource.
       */
      size_t const content_len;

      /**
       * Specifies the time stamp for the resource.
       */
      char const *content_date;

      /**
       * Specifies the files on which this resource depends.
       */
      StrAsc const dependencies[MAX_RESOURCE_DEPENDENCIES];
   };


   /**
    * Defines a resource object managed by the ResourceManager class.
    */
   class Resource
   {
   public:
      /**
       * Pointer to the resource content.
       */
      void const *content;

      /**
       * Specifies the length of the content.
       */
      size_t content_len;

      /**
       * Specifies the dependencies for the resource.
       */
      StrAsc dependencies[MAX_RESOURCE_DEPENDENCIES];

      /**
       * Specifies the date when the resource was generated.
       */
      StrAsc content_date;

      /**
       * Construct the resource from the init structure.
       */
      Resource(ResourceInit const &init):
         content(init.content),
         content_len(init.content_len),
         content_date(init.content_date)
      {
         for(int i = 0; i < MAX_RESOURCE_DEPENDENCIES; ++i)
            dependencies[i] = init.dependencies[i];
      }

      ////////////////////////////////////////////////////////////
      // copy constructor
      ////////////////////////////////////////////////////////////
      Resource(Resource const &other):
         content(other.content),
         content_len(other.content_len)
      {
         for(int i = 0; i < MAX_RESOURCE_DEPENDENCIES; ++i)
            dependencies[i] = other.dependencies[i];
      }

      /**
       * Default constructor
       */
      Resource():
         content(0),
         content_len(0)
      { }

      /**
       * Copy operator
       */
      Resource &operator =(Resource const &other)
      {
         content = other.content;
         content_len = other.content_len;
         for(int i = 0; i < MAX_RESOURCE_DEPENDENCIES; ++i)
            dependencies[i] = other.dependencies[i];
         content_date = other.content_date;
         return *this;
      }
   };
   

   /**
    * Defines an object that maintains a collection of resources and their dependencies and provides
    * methods for writing those resources to a stream.
    */
   class ResourceManager
   {
   public:
      /**
       * Default Constructor
       */
      ResourceManager();

      /**
       * Destructor
       */
      virtual ~ResourceManager();

      /**
       * Adds the resources from the specified array of init structures.
       *
       * @param init Specifies an array of initialisation structures.
       */
      void add_resources(ResourceInit const init[]);

      // @group: resource container definitions

      /**
       * @return Returns an iterator to the first resource.
       */
      typedef std::map<StrAsc, Resource> resources_type;
      typedef resources_type::iterator iterator;
      typedef resources_type::const_iterator const_iterator;
      iterator begin()
      { return resources.begin(); }
      const_iterator begin() const
      { return resources.begin(); }

      /**
       * @return Returns an iterator beyond the last resource.
       */
      iterator end()
      { return resources.end(); }
      const_iterator end() const
      { return resources.end(); }

      /**
       * @return Returns true if there are no resources.
       */
      bool empty() const
      { return resources.empty(); }

      /**
       * Removes all resources.
       */
      void clear()
      { resources.clear(); }

      /**
       * Associative subscript operator.
       */
      Resource const &operator [](StrAsc const &key)
      { return resources[key]; }

      /**
       * @return Returns an iterator for the specified key.
       */
      iterator find(StrAsc const &key)
      { return resources.find(key); }
      const_iterator find(StrAsc const &key) const
      { return resources.find(key); }
      
      // @endgroup

      /**
       * Generates the resource associated with the specified key along wirth all of its preceding
       * depencies to the specified stream.
       *
       * @param out Specifies the stream to which the resource will be generated.
       *
       * @param key Specifies the name of the resource to generate.
       *
       * @param used_keys Specifies the collection of rssources that have already been generated.
       * This provides a mechanmism to avoid generating the same dependency more than once.
       */
      typedef std::set<StrAsc> used_keys_type;
      void generate(
         std::ostream &out, StrAsc const &key, used_keys_type &used_keys) const;

      /**
       * @return Returns the resource date for the specified key.  This value will be the greater of
       * all the resource and all of its dependencies.  Returns an empty string if the resource was
       * not found.
       *
       * @param key Specifies the resource to look up.
       */
      StrAsc get_resource_date(StrAsc const &key) const;

      /**
       * @return Returns the SHA1 checksum for the specified resource key encoded in base64.
       * Returns an empty string if the resource does not exist.
       *
       * @param key Specifies the name of the resource.
       */
      StrAsc get_resource_sig(StrAsc const &key) const;
      
   private:
      /**
       * Specifies the set of resources managed.
       */
      resources_type resources;
   };


   /**
    * Defines an object that, given a resource manager and a set of keys, will generate the
    * resources to a stream along with their required dependencies.
    */
   class ResourceGenerator
   {
   public:
      /**
       * Constructor
       *
       * @param manager_ Specifies the manager for the set of resources.
       */
      ResourceGenerator(ResourceManager *manager_):
         manager(manager_)
      { }

      /**
       * Generates the resource along with its dependencies.
       *
       * @param out Specifies the output stream.
       *
       * @param key Specifies the name of the resource that should be generated.
       *
       * @param ignore_past Set to true if previously used keys should be ignored.
       */
      void generate(std::ostream &out, StrAsc const &key, bool ignore_past = false)
      {
         if(ignore_past)
            used_keys.clear();
         manager->generate(out, key, used_keys);
      }

      /**
       * @return Returns true if the specified key has been used.
       *
       * @param key Specififies the key to check.
       */
      bool is_key_used(StrAsc const &key) const
      {
         return (used_keys.find(key) != used_keys.end());
      }
            
private:
      /**
       * Specifies the collection of resources.
       */
      ResourceManager const *manager;

      /**
       * Specifies the collection of keys that have been used.
       */
      ResourceManager::used_keys_type used_keys;
   };
};


#endif
