/* Csi.InstanceValidator.cpp

   Copyright (C) 2000, 2003 Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Thursday 01 June 2000
   Last Change: Tuesday 08 July 2003
   Last Commit: $Date: 2007-11-13 15:01:58 -0600 (Tue, 13 Nov 2007) $ (UTC)
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include <typeinfo>
#include <iostream>
#include "Csi.InstanceValidator.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class InstanceValidator definitions
   ////////////////////////////////////////////////////////////
   InstanceValidator::valid_instances_type InstanceValidator::valid_instances;


   InstanceValidator::InstanceValidator()
   {
      // It is possible that this constructor will be called several times for the same object
      // instance since a class could inherit from InstanceValidator multiple times. This kind of
      // inheritance does not really cost anything since there is no non-static class data. However,
      // we do need to guard against the pointer being placed in the valid_instances member multiple
      // times.
      valid_instances_type::key_type key(valid_instances);
      if(key->find(this) == key->end())
         key->insert(this);
   } // constructor


   void InstanceValidator::invalidate_this_instance()
   {
      // as mentioned in the constructor comments, this method may get called in the process of
      // destroying a high level object that inherits from this class through more than one base
      // class. Only one of these calls can be allowed to modify the container of valid
      // instances. This is all right since the object should not be considered to be valid while
      // any destructor associated with that object is valid.
      valid_instances_type::key_type key(valid_instances);
      valid_instances_set_type::iterator i = key->find(this);
      if(i != key->end())
         key->erase(i);
   } // invalidate_this_instance


   void InstanceValidator::print_remaining_instances(std::ostream &out)
   {
      valid_instances_type::key_type key(valid_instances);
      for(valid_instances_set_type::iterator ii = key->begin();
          ii != key->end();
          ++ii)
      {
         out << typeid(**ii).name() << "\n";
      }
   } // print_remaining_instances
};
