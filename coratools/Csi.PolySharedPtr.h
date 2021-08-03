/* Csi.PolySharedPtr.h

   Copyright (C) 2000, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 13 January 2000
   Last Change: Tuesday 31 March 2015
   Last Commit: $Date: 2015-03-31 09:03:25 -0600 (Tue, 31 Mar 2015) $ 

*/

#ifndef Csi_PolySharedPtr_h
#define Csi_PolySharedPtr_h

#include "Csi.SharedPtr.h"
#include <typeinfo>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // template class PolySharedPtr
   //
   // A two parameter template that defines a class that encapsulates an instance of
   // Csi::SharedPtr<base_type> but makes pointers of type derived_type available. It is anticipated
   // that this class will be most useful in cases where a pointer to a base class is used
   // polymorphically.
   ////////////////////////////////////////////////////////////
   template <class base_type, class derived_type>
   class PolySharedPtr
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor from derived_type pointer
      ////////////////////////////////////////////////////////////
      typedef derived_type value_type;
      PolySharedPtr(derived_type *rep = 0):
         handle(rep)
      { }

      ////////////////////////////////////////////////////////////
      // copy constructor
      //////////////////////////////////////////////////////////// 
      PolySharedPtr(PolySharedPtr const &other):
         handle(other.handle)
      { }

      ////////////////////////////////////////////////////////////
      // constructor from base_type handle
      //
      // Checks to make sure that the object pointed to can be represented by a derived_type pointer
      // using dynamic cast. If the object is not representable, an object of class std::bad_cast
      // will be thrown.
      //////////////////////////////////////////////////////////// 
      PolySharedPtr(SharedPtr<base_type> const &base_ptr):
         handle(base_ptr)
      { verify_base_ptr(); }

      ////////////////////////////////////////////////////////////
      // copy operator
      //////////////////////////////////////////////////////////// 
      PolySharedPtr &operator =(PolySharedPtr const &other)
      { handle = other.handle; return *this; }

      ////////////////////////////////////////////////////////////
      // copy operator from base type handle
      //////////////////////////////////////////////////////////// 
      PolySharedPtr &operator =(SharedPtr<base_type> const &base_ptr)
      { handle = base_ptr; verify_base_ptr(); return *this; }

      ////////////////////////////////////////////////////////////
      // destructor
      //////////////////////////////////////////////////////////// 
      ~PolySharedPtr()
      { }

      //@group access operators
      // These access operators using a static cast to the derived type. This is OK because the
      // derived-base relationship was tested when the assignment was made.
      ////////////////////////////////////////////////////////////
      // pointer member
      //////////////////////////////////////////////////////////// 
      derived_type *operator ->()
      {
         if(handle == 0)
            throw SharedPtrException<derived_type>();
         return static_cast<derived_type *>(handle.get_rep());
      }

      ////////////////////////////////////////////////////////////
      // pointer const member
      //////////////////////////////////////////////////////////// 
      derived_type const *operator ->() const
      {
         if(handle == 0)
            throw SharedPtrException<derived_type>();
         return static_cast<derived_type const *>(handle.get_rep());
      }

      ////////////////////////////////////////////////////////////
      // pointer dereference
      //////////////////////////////////////////////////////////// 
      derived_type &operator *()
      {
         if(handle == 0)
            throw SharedPtrException<derived_type>();
         return static_cast<derived_type &>(*handle.get_rep());
      }
      
      ////////////////////////////////////////////////////////////
      // pointer const dereference
      //////////////////////////////////////////////////////////// 
      derived_type const &operator *() const
      {
         if(handle == 0)
            throw SharedPtrException<derived_type>();
         return static_cast<derived_type const &>(*handle.get_rep());
      }

      ////////////////////////////////////////////////////////////
      // get_handle
      //////////////////////////////////////////////////////////// 
      SharedPtr<base_type> &get_handle() { return handle; }
      //@endgroup

      ////////////////////////////////////////////////////////////
      // bind (derived_type)
      //////////////////////////////////////////////////////////// 
      void bind(derived_type *rep)
      { handle.bind(rep); }

      ////////////////////////////////////////////////////////////
      // bind (base_type)
      //////////////////////////////////////////////////////////// 
      void bind(base_type *rep)
      { handle.bind(rep); verify_base_ptr(); }

      ////////////////////////////////////////////////////////////
      // get_rep
      //////////////////////////////////////////////////////////// 
      derived_type *get_rep() { return static_cast<derived_type *>(handle.get_rep()); }
      derived_type const *get_rep() const { return static_cast<derived_type const *>(handle.get_rep()); }

      ////////////////////////////////////////////////////////////
      // get_reference_count
      //////////////////////////////////////////////////////////// 
      int get_reference_count() const { return handle.get_reference_count(); }

      ////////////////////////////////////////////////////////////
      // clear
      //////////////////////////////////////////////////////////// 
      void clear() { bind(static_cast<derived_type *>(0)); }

      ////////////////////////////////////////////////////////////
      // comparison (==) operator
      ////////////////////////////////////////////////////////////
      bool operator ==(PolySharedPtr<base_type,derived_type> const &other) const
      { return handle == other.handle; }
      bool operator ==(derived_type const *other_rep) const
      { return static_cast<derived_type const *>(handle.get_rep()) == other_rep; }
      
      ////////////////////////////////////////////////////////////
      // comparison (!= operator
      ////////////////////////////////////////////////////////////
      bool operator !=(PolySharedPtr<base_type,derived_type> const &other) const
      { return get_rep() != other.get_rep(); }
      bool operator !=(derived_type const *other_rep) const
      { return get_rep() != other_rep; }
      
   private:
      ////////////////////////////////////////////////////////////
      // verify_base_ptr
      //
      // Verifies that the handle, if it is not bound to a null pointer, can be cast to a pointer to
      // class derived_type. This is done using the dynamic_cast operator. If the cast fails, an
      // object of class std::bad_cast will be thrown
      //////////////////////////////////////////////////////////// 
      void verify_base_ptr() const
      {
         if(handle.get_rep() != 0 && dynamic_cast<derived_type const *>(handle.get_rep()) == 0)
            throw std::bad_cast();
      }
      
   private:
      ////////////////////////////////////////////////////////////
      // handle
      //////////////////////////////////////////////////////////// 
      SharedPtr<base_type> handle;
   };


   /**
    * Defines a functor that can be used to match a poly shared pointer.
    */
   template <class base_type, class derived_type>
   struct HasPolySharedPtr
   {
      derived_type const *ptr;
      HasPolySharedPtr(derived_type const *ptr_):
         ptr(ptr_)
      { }

      bool operator ()(PolySharedPtr<base_type, derived_type> const &p) const
      { return p.get_rep() == ptr; }
   };
};

#endif
