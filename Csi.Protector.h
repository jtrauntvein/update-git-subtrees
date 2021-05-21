/* Csi.Protector.h

   Copyright (C) 2000, 2006 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 14 March 2000
   Last Change: Tuesday 07 February 2006
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Committed by: $Author: tmecham $
   
*/

#ifndef Csi_Protector_h
#define Csi_Protector_h

#include "Csi.CriticalSection.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // template Protector
   //
   // A class template that encapsulates an instance of the specified class and
   // allows access to that instance only via methods that guard it with a
   // critical section. This is intended to make the object instance being
   // guarded thread-safe.
   //
   // The class <protege_type> must define a default constructor as well as a
   // copy constructor. Access to the protected object can be had by
   // constructing objects of class key_type or const_key_type. The
   // constructors and destructors for these classes will lock and unlock the
   // critical section guarding the protected object as needed.
   //
   // The classes, key_type and const_key_type behave much like smart pointers
   // in that they overload the pointer dereference operators ->() and
   // *(). These operators provide access to the protected object. key objects
   // cannot, however, be copied. 
   ////////////////////////////////////////////////////////////
   template <class protege_type>
   class Protector
   {
   public:
      ////////////////////////////////////////////////////////////
      // class key_type
      //
      // Provides access to the guarded element of a protector. In its
      // constructor, it obtains a lock on the guarded reference. The lock will
      // be released when the destructor for this object is run or when the
      // application invokes release()
      ////////////////////////////////////////////////////////////
      class key_type
      {
      private:
         ////////////////////////////////////////////////////////////
         // is_locked
         ////////////////////////////////////////////////////////////
         bool is_locked;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         key_type(Protector<protege_type> &protector_):
            protege(0),
            protector(&protector_),
            is_locked(false)
         {
            protector->guard.lock();
            is_locked = true;
            protege = &protector->protege;
         } // constructor

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         ~key_type()
         { release(); }

         ////////////////////////////////////////////////////////////
         // release
         ////////////////////////////////////////////////////////////
         void release()
         {
            if(is_locked)
            {
               protector->guard.unlock();
               is_locked = false;
            }
         }

         ////////////////////////////////////////////////////////////
         // access operators
         ////////////////////////////////////////////////////////////
         protege_type *operator ->() { return protege; }
         protege_type &operator *() { return *protege; }

      private:
         ////////////////////////////////////////////////////////////
         // protege
         //
         // The object being protected
         ////////////////////////////////////////////////////////////
         protege_type *protege;

         ////////////////////////////////////////////////////////////
         // protector
         ////////////////////////////////////////////////////////////
         Protector<protege_type> *protector;
      };


      ////////////////////////////////////////////////////////////
      // const_key_type
      //
      // Provides access to the guarded element of a protector. In its
      // constructor, it obtains a lock on the guarded reference. The lock will
      // be released when the destructor for this object is run or release() is
      // invoked.
      ////////////////////////////////////////////////////////////
      class const_key_type
      {
      private:
         ////////////////////////////////////////////////////////////
         // is_locked
         ////////////////////////////////////////////////////////////
         bool is_locked;
         
      public:
         ////////// constructor
         const_key_type(Protector<protege_type> const &protector_):
            protege(0),
            protector(&protector_),
            is_locked(false)
         {
            protector->guard.lock();
            is_locked = true;
            protege = &protector->protege;
         } // constructor

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         ~const_key_type()
         { release(); }

         ////////////////////////////////////////////////////////////
         // release
         ////////////////////////////////////////////////////////////
         void release()
         {
            if(is_locked)
            {
               is_locked = false;
               protector->guard.unlock();
            }
         }

         ////////////////////////////////////////////////////////////
         // access operators
         ////////////////////////////////////////////////////////////
         protege_type const *operator ->() const { return protege; }
         protege_type const &operator *() const { return *protege; }

      private:
         ////////////////////////////////////////////////////////////
         // protege
         ////////////////////////////////////////////////////////////
         protege_type const *protege;

         ////////////////////////////////////////////////////////////
         // protector
         ////////////////////////////////////////////////////////////
         Protector<protege_type> const *protector;
      };

   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      Protector(protege_type const &protege_): protege(protege_) { }
      Protector() { }

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      ~Protector() { }

   private:
      ////////////////////////////////////////////////////////////
      // guard
      ////////////////////////////////////////////////////////////
      mutable CriticalSection guard;

      ////////////////////////////////////////////////////////////
      // protege
      //
      // The object that is being guarded
      ////////////////////////////////////////////////////////////
      protege_type protege;

      friend class key_type;
      friend class const_key_type;
   };
};

#endif
