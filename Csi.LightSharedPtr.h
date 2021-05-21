/* Csi.LightSharedPtr.h

   Copyright (C) 1998, 2014 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Monday 08 June 1998
   Last Change: Monday 21 July 2014
   Last Commit: $Date: 2014-07-29 12:16:18 -0600 (Tue, 29 Jul 2014) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_LightSharedPtr_h
#define Csi_LightSharedPtr_h

#include "Csi.SharedPtrException.h"
#include <assert.h>
#include "trace.h"
#include <typeinfo>


namespace Csi
{
   /**
    * Defines a template for what can be best viewed as a new kind of pointer
      with reference counting and garbage collection. This code was mostly
      adapted from the Handle template presented in section 25.7 of "The C++
      Programming Language" third edition by Bjarne Stroustrup. I have added
      the capability for the handle to be able to deal with null pointers as
      well as valid heap objects.
     
      Limitations:
     
         1 - The code is written with the understanding that once a pointer is
         assigned to a LightSharedPtr object, that object will claim ownership of
         that pointer. This implies that any pointer given to a LightSharedPtr
         object to manage must be allocated using new.
     
         2 - It is possible for code using a LightSharedPtr object to obtain a copy
         of the pointer being managed by that LightSharedPtr. It should be
         remembered, however, that the LightSharedPtr object still maintains
         ownership of the object being pointed to. If the client deletes a
         pointer managed by a LightSharedPtr object, a severe memory error will
         occur when the LightSharedPtr object itself is destroyed (or a new pointer
         is assigned to it) and it attempts to delete the same pointer again.
     
         3 - Objects generated from this template will occupy twice as much
         memory as a normal pointer would. This is due to the overhead imposed
         by keeping track of a reference count.
   */
   template <class T>
   class LightSharedPtr
   {
   public:
      /**
       * Creates the shared pointer from a raw pointer.  This version will claim ownership of the
       * raw pointer.
       *
       * @param representation_  Specifies the raw pointer
       */
      typedef T value_type;
      LightSharedPtr(T *representation_ = 0):
         representation(representation_),
         reference_count(new int(1))
      { } 

      /**
       * Creates a copy of another shared pointer and shares that object's representation and
       * reference count.
       *
       * @param Another shared pointer with which this object will shared the representation and
       * reference count
       */
      LightSharedPtr(LightSharedPtr const &other):
         representation(other.representation),
         reference_count(0)
      {
         // update the reference count
         reference_count = other.reference_count;
         ++(*reference_count);
      } // copy constructor

      /**
       * Copies the representation and reference count from the other shared pointer.  This will
       * first release owned by this shared pointer.
       *
       * @param other  Specifies another shared pointer that we will copy.
       */
      LightSharedPtr &operator =(LightSharedPtr const &other)
      {
         if(representation != other.representation)
         {
            if(*reference_count == 0 || --(*reference_count) == 0)
            {
               delete representation;
               delete reference_count;
            }
            representation = other.representation;
            reference_count = other.reference_count;
            ++(*reference_count);
         }
         return *this;
      }

      /**
       * Destructor will destroy the representation if the reference count drops to zero.
       */
      ~LightSharedPtr()
      {
         if(*reference_count == 0 || --(*reference_count) == 0)
         {
            delete representation;
            delete reference_count;
            representation = 0;
            reference_count = 0;
         }
      } // destructor

      /**
       * Overloaded pointer dereference operator
       */
      T *operator ->()
      {
         if(representation == 0)
            throw SharedPtrException<T>();
         return representation;
      }
      
      T &operator *()
      {
         if(representation == 0)
            throw SharedPtrException<T>();
         return *representation;
      }

      T const *operator ->() const
      {
         if(representation == 0)
            throw SharedPtrException<T>();
         return representation;
      }
      
      T const &operator *() const
      {
         if(representation == 0)
            throw SharedPtrException<T>();
         return *representation;
      }

      /**
       * Replaces the current representation pointer with the one being passed
       * in. Will delete the current representation if the reference count
       * drops to zero
       */
      void bind(T *representation_)
      {
         if(representation != representation_)
         {
            if(*reference_count == 0 || --(*reference_count) == 0)
            {
               delete representation;
               *reference_count = 1;     // recycle the existing pointer
            }
            else
               reference_count = new int(1);
            representation = representation_;
         }
      }

      /**
       * @return returns the variable reference.
       */
      T *get_rep() { return representation; }
      T const *get_rep() const { return representation; }

      /**
       * @return Returns the number of shared pointers that share this reference.
       */
      int get_reference_count() const
      { return *reference_count; }

      /**
       * Clears any reference owned by this shared pointer.
       */
      void clear() { bind(0); }

      /**
       * Compares this shared pointer representation with another shared pointer or raw pointer.
       */
      bool operator ==(LightSharedPtr<T> const &other) const
      { return representation == other.representation; }
      bool operator ==(T const *other_rep) const
      { return representation == other_rep; }

      bool operator !=(LightSharedPtr<T> const &other) const
      { return representation != other.representation; }
      bool operator !=(T const *other_rep) const
      { return representation != other_rep; }

   private:
      /**
       * Reference to the raw pointer managed by this object.
       */
      T *representation;

      /**
       * Reference to a variable that keeps track of the number of shared references.
       */
      int *reference_count;
   };


   /**
    * Defines a functor that can be used to compare a shared pointer with
    * another pointer.  This is very useful when searching a container of
    * shared pointers to determine if a pointer is present.
    */
   template <class T>
   struct HasLightSharedPtr
   {
      T const *ptr;
      HasLightSharedPtr(T const *ptr_):
         ptr(ptr_)
      { }

      bool operator ()(LightSharedPtr<T> const &p)
      { return p == ptr; }
   };


   /**
    * A two parameter template that defines a class that encapsulates an instance of
    * Csi::SharedPtr<base_type> but makes pointers of type derived_type available. It is anticipated
    * that this class will be most useful in cases where a pointer to a base class is used
    * polymorphically.
    */
   template <class base_type, class derived_type>
   class LightPolySharedPtr
   {
   public:
      typedef derived_type value_type;
      LightPolySharedPtr(derived_type *rep = 0):
         handle(rep)
      { }

      LightPolySharedPtr(LightPolySharedPtr const &other):
         handle(other.handle)
      { }


      /**
       * Checks to make sure that the object pointed to can be represented by a derived_type pointer
       * using dynamic cast. If the object is not representable, an object of class std::bad_cast
       * will be thrown.
       */
      LightPolySharedPtr(LightSharedPtr<base_type> const &base_ptr):
         handle(base_ptr)
      { }


      LightPolySharedPtr &operator =(LightPolySharedPtr const &other)
      {
         handle = other.handle;
         return *this;
      }

      LightPolySharedPtr &operator =(LightSharedPtr<base_type> const &base_ptr)
      {
         handle = base_ptr;
         return *this;
      }

      ~LightPolySharedPtr()
      { handle.clear(); }

      //@group access operators
      // These access operators using a static cast to the derived type. This is OK because the
      // derived-base relationship was tested when the assignment was made.
      derived_type *operator ->()
      {
         if(handle == 0)
            throw SharedPtrException<derived_type>();
         return static_cast<derived_type *>(handle.get_rep());
      }

      derived_type const *operator ->() const
      {
         if(handle == 0)
            throw SharedPtrException<derived_type>();
         return static_cast<derived_type const *>(handle.get_rep());
      }

      derived_type &operator *()
      {
         if(handle == 0)
            throw SharedPtrException<derived_type>();
         return static_cast<derived_type &>(*handle.get_rep());
      }
      
      derived_type const &operator *() const
      {
         if(handle == 0)
            throw SharedPtrException<derived_type>();
         return static_cast<derived_type const &>(*handle.get_rep());
      }

      LightSharedPtr<base_type> &get_handle()
      { return handle; }
      
      //@endgroup

      void bind(derived_type *rep)
      { handle.bind(rep); }

      void bind(base_type *rep)
      { handle.bind(rep); }

      derived_type *get_rep()
      { return static_cast<derived_type *>(handle.get_rep()); }
      derived_type const *get_rep() const
      { return static_cast<derived_type const *>(handle.get_rep()); }

      int get_reference_count() const
      { return handle.get_reference_count(); }

      void clear()
      { bind(static_cast<derived_type *>(0)); }

      bool operator ==(LightPolySharedPtr<base_type,derived_type> const &other) const
      { return handle == other.handle; }
      bool operator ==(derived_type const *other_rep) const
      { return static_cast<derived_type const *>(handle.get_rep()) == other_rep; }
      
      bool operator !=(LightPolySharedPtr<base_type,derived_type> const &other) const
      { return get_rep() != other.get_rep(); }
      bool operator !=(derived_type const *other_rep) const
      { return get_rep() != other_rep; }
      
   private:
      LightSharedPtr<base_type> handle;
   };
};

#endif
