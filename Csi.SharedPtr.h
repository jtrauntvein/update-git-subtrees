/* Csi.SharedPtr.h

   Copyright (C) 1998, 2014 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Monday 08 June 1998
   Last Change: Monday 21 July 2014
   Last Commit: $Date: 2014-07-21 12:09:34 -0600 (Mon, 21 Jul 2014) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_SharedPtr_h
#define Csi_SharedPtr_h

#include "Csi.CriticalSection.h"
#include "Csi.SharedPtrException.h"
#include <assert.h>
#include "trace.h"
#include <typeinfo>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // template class SharedPtr
   //
   // Defines a template for what can be best viewed as a new kind of pointer
   // with reference counting and garbage collection. This code was mostly
   // adapted from the Handle template presented in section 25.7 of "The C++
   // Programming Language" third edition by Bjarne Stroustrup. I have added
   // the capability for the handle to be able to deal with null pointers as
   // well as valid heap objects.
   //
   // Limitations:
   //
   //    1 - The code is written with the understanding that once a pointer is
   //    assigned to a SharedPtr object, that object will claim ownership of
   //    that pointer. This implies that any pointer given to a SharedPtr
   //    object to manage must be allocated using new.
   //
   //    2 - It is possible for code using a SharedPtr object to obtain a copy
   //    of the pointer being managed by that SharedPtr. It should be
   //    remembered, however, that the SharedPtr object still maintains
   //    ownership of the object being pointed to. If the client deletes a
   //    pointer managed by a SharedPtr object, a severe memory error will
   //    occur when the SharedPtr object itself is destroyed (or a new pointer
   //    is assigned to it) and it attempts to delete the same pointer again.
   //
   //    3 - Objects generated from this template will occupy twice as much
   //    memory as a normal pointer would. This is due to the overhead imposed
   //    by keeping track of a reference count.
   //
   // The reference count pointers maintained by SharedPtr objects are
   // protected from multiple thread access using a Csi:;Protector (critical
   // section or mutex) object.  This is done to help alleviate problems that
   // can occur when SharedPtr copies are made in different threads.
   ////////////////////////////////////////////////////////////
   template <class T>
   class SharedPtr
   {
   public:
      ////////////////////////////////////////////////////////////
      // value_type
      ////////////////////////////////////////////////////////////
      typedef T value_type;
      
   private:
      ////////////////////////////////////////////////////////////
      // representation
      //
      // A pointer to the object that is being proxied
      ////////////////////////////////////////////////////////////
      T *representation;

      ////////////////////////////////////////////////////////////
      // reference_count
      //
      // Represents the number of times that a SharedPtr object has been created from the
      // original. This member is shared by all objects that reference the same representation. The
      // particular object that sets this count to zero will be responsible for deleting this count
      // as well as the representation.
      ////////////////////////////////////////////////////////////
      int *reference_count;

      ////////////////////////////////////////////////////////////
      // guard
      //
      // A critical section that guards the reference count so that it cannot be changed
      // simultaneously by multiple threads.  This member is declared as a pointer so that shared
      // pointers that reference the same pointer also reference the same guard.
      ////////////////////////////////////////////////////////////
      mutable CriticalSection *guard;

   public:
      ////////////////////////////////////////////////////////////
      // basic constructor
      //
      // Used to construct the first proxy to the representation. Subsequent copies should be made
      // using either the copy constructor or the copy operator
      ////////////////////////////////////////////////////////////
      SharedPtr(T *representation_ = 0):
         representation(representation_),
         reference_count(new int(1)),
         guard(new CriticalSection)
      { } 

      ////////////////////////////////////////////////////////////
      // copy constructor
      ////////////////////////////////////////////////////////////
      SharedPtr(SharedPtr const &other):
         representation(other.representation),
         reference_count(0),
         guard(0)
      {
         // update the reference count
         other.guard->lock();
         guard = other.guard;
         reference_count = other.reference_count;
         ++(*reference_count);
         other.guard->unlock();
      } // copy constructor

      ////////////////////////////////////////////////////////////
      // copy operator
      ////////////////////////////////////////////////////////////
      SharedPtr &operator =(SharedPtr const &other)
      {
         if(representation != other.representation)
         {
            guard->lock();
            if(*reference_count == 0 || --(*reference_count) == 0)
            {
               delete representation;
               delete reference_count;
               guard->unlock();
               delete guard;
            }
            else
               guard->unlock();
            other.guard->lock();
            guard = other.guard;
            representation = other.representation;
            reference_count = other.reference_count;
            ++(*reference_count);
            other.guard->unlock();
         }
         return *this;
      }

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      ~SharedPtr()
      {
         guard->lock();
         if(*reference_count == 0 || --(*reference_count) == 0)
         {
            delete representation;
            delete reference_count;
            guard->unlock();
            delete guard;
            representation = 0;
            reference_count = 0;
            guard = 0;
         }
         else
            guard->unlock();
      } // destructor

      ////////////////////////////////////////////////////////////
      // access operators
      //
      // Returns a pointer to the representation object
      ////////////////////////////////////////////////////////////
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

      ////////////////////////////////////////////////////////////
      // bind
      //
      // Replaces the current representation pointer with the one being passed
      // in. Will delete the current representation if the reference count
      // drops to zero
      ////////////////////////////////////////////////////////////
      void bind(T *representation_)
      {
         if(representation != representation_)
         {
            guard->lock();
            if(*reference_count == 0 || --(*reference_count) == 0)
            {
               delete representation;
               guard->unlock();
               *reference_count = 1;     // recycle the existing pointer
            }
            else
            {
               guard->unlock();
               guard = new CriticalSection;
               reference_count = new int(1);
            }
            representation = representation_;
         }
      }

      ////////////////////////////////////////////////////////////
      // get_rep
      //
      // Returns the pointer to the representation
      ////////////////////////////////////////////////////////////
      T *get_rep() { return representation; }
      T const *get_rep() const { return representation; }

      ////////////////////////////////////////////////////////////
      // get_reference_count
      //
      // Returns the number of SharedPtr objects that are sharing this reference
      ////////////////////////////////////////////////////////////
      int get_reference_count() const
      {
         int rtn = 0;
         guard->lock();
         rtn = *reference_count;
         guard->unlock();
         return rtn;
      } 

      ////////////////////////////////////////////////////////////
      // clear
      ////////////////////////////////////////////////////////////
      void clear() { bind(0); }

      ////////////////////////////////////////////////////////////
      // comparison (==) operator
      ////////////////////////////////////////////////////////////
      bool operator ==(SharedPtr<T> const &other) const
      { return representation == other.representation; }
      bool operator ==(T const *other_rep) const
      { return representation == other_rep; }

      ////////////////////////////////////////////////////////////
      // comparison (!= operator
      ////////////////////////////////////////////////////////////
      bool operator !=(SharedPtr<T> const &other) const
      { return representation != other.representation; }
      bool operator !=(T const *other_rep) const
      { return representation != other_rep; }

      // note: the other comparison operators (>, <, etc.) are not implemented because of lack of
      // relevance 
   };


   ////////////////////////////////////////////////////////////
   // template HasSharedPtr
   //
   // Defines a functor that can be used to compare a shared pointer with
   // another pointer.  This is very useful when searching a container of
   // shared pointers to determine if a pointer is present.  
   ////////////////////////////////////////////////////////////
   template <class T>
   struct HasSharedPtr
   {
      ////////////////////////////////////////////////////////////
      // ptr
      ////////////////////////////////////////////////////////////
      T const *ptr;

      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      HasSharedPtr(T const *ptr_):
         ptr(ptr_)
      { }

      ////////////////////////////////////////////////////////////
      // functor
      ////////////////////////////////////////////////////////////
      bool operator ()(SharedPtr<T> const &p)
      { return p == ptr; }
   };
};

#endif
