/* Csi.InstanceValidator.h

   Copyright (C) 2000, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 24 May 2000
   Last Change: Monday 11 December 2017
   Last Commit: $Date: 2017-12-11 16:56:59 -0600 (Mon, 11 Dec 2017) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_InstanceValidator_h
#define Csi_InstanceValidator_h

#ifdef _WIN32
#include <unordered_set>
#else
#include <set>
#endif
#include <iosfwd>
#include "Csi.Protector.h"


namespace Csi
{
   /**
    * Defines a component that keeps a static container of all instances (this pointers) of itself
    * and defines static methods that will allow the application to test for object validity.  This
    * set of instances is maintained usingthe default constructor and virtual destructor.
    */
   class InstanceValidator
   {
   private:
      /**
       * Specifies the references to all valid objects throughout the application that are derived
       * from this class.  This container is maintained by the constructor and destructor and is
       * bound within a protector so that it can be accessed from multiple threads.
       */
#ifdef _WIN32
      typedef std::unordered_set<InstanceValidator const *> valid_instances_set_type;
#else
      typedef std::set<InstanceValidator const *> valid_instances_set_type;
#endif
      typedef Csi::Protector<valid_instances_set_type> valid_instances_type;
      static valid_instances_type valid_instances;
      
   public:
      /**
       * Default constructor will be called automatically as a part of the derived class.  This
       * version will register the class instances with the valid_instances container.
       */
      InstanceValidator();

      /**
       * This method can be invokeed explicitly by the application or will be invoked automatically
       * by the destructor.  It will remove this object's pointer from the valid_instances container
       * so that future calls to is_valid_instance() will return false.
       */
      void invalidate_this_instance();

      /**
       * Destructor
       */
      virtual ~InstanceValidator()
      { invalidate_this_instance(); }

      /**
       * @return Returns true if the specified pointer can be found in the valid_instances container
       * and can be interpreted as class T.
       *
       * @param i Specifies the instance to test.
       */
      template <class T>
      static bool is_valid_instance(T const *i)
      {
         bool rtn(false);
         if(i != 0)
         {
            InstanceValidator const *val = static_cast<InstanceValidator const *>(i);
            valid_instances_type::const_key_type key(valid_instances);
            rtn = (key->find(val) != key->end());
            if(rtn && dynamic_cast<T const *>(val) == 0)
               rtn = false;
         }
         return rtn;
      }

      /**
       * @return Returns the total number of instances tracked.
       */
      static valid_instances_set_type::size_type remaining_instances_count()
      {
         valid_instances_type::const_key_type key(valid_instances);
         return key->size();
      }

      /**
       * Prins out descriptions of each remaining instance.
       */
      static void print_remaining_instances(std::ostream &out);
   };
};

#endif
