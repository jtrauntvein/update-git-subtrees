/* Csi.OrderedList.h

   Copyright (C) 1998, 2008 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 16 December 1999
   Last Change: Thursday 18 December 2008
   Last Commit: $Date: 2008-12-18 15:50:01 -0600 (Thu, 18 Dec 2008) $ 
   Committed by: $author:$
   
*/

#ifndef Csi_OrderedList_h
#define Csi_OrderedList_h

#include <list>
#include <algorithm>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // template class OrderedList
   //
   // This template defines an stl based container that attempts to maintain
   // order (defined by a supplied predicate template parameter) between nodes
   // in a stable way (i.e. original order of insertion is preserved when
   // possible). This is similar to the interface provided by the STL
   // priority_queue adapter only the interface to this class is more open in
   // that nodes in the middle of the queue can be deleted and the nodes can
   // also be iterated.
   ////////////////////////////////////////////////////////////
   template <class T, class Pred = std::less<T> >
   class OrderedList
   {
   public:
      //@group container, reference, and iterator type definitions
      typedef typename std::list<T> container_type;
      typedef typename container_type::reference reference;
      typedef typename container_type::const_reference const_reference;
      typedef typename container_type::size_type size_type;
      typedef typename container_type::iterator iterator;
      typedef typename container_type::const_iterator const_iterator;
      typedef typename container_type::reverse_iterator reverse_iterator;
      typedef typename container_type::const_reverse_iterator const_reverse_iterator;
      typedef typename container_type::value_type value_type;
      //@endgroup
      
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      OrderedList()
      { }

      ////////////////////////////////////////////////////////////
      // copy constructor
      ////////////////////////////////////////////////////////////
      OrderedList(OrderedList const &other):
         container(other.container)
      { }

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      ~OrderedList()
      { }

      ////////////////////////////////////////////////////////////
      // copy operator
      ////////////////////////////////////////////////////////////
      OrderedList &operator =(OrderedList const &other)
      { container = other.container; return *this; }
      
      //@group iterator access methods
      iterator begin() { return  container.begin(); }
      const_iterator begin() const { return container.begin(); }
      reverse_iterator rbegin() { return container.rbegin(); }
      const_reverse_iterator rbegin() const { return container.rbegin(); }
      iterator end() { return container.end(); }
      const_iterator end() const { return container.end(); }
      reverse_iterator rend() { return container.rend(); }
      const_reverse_iterator rend() const { return container.rend(); }
      //@endgroup

      ////////////////////////////////////////////////////////////
      // push
      //
      // Inserts an element into the conatiner. The element will be placed in
      // the order specified by the predicate while preserving existing order
      // in the list
      ////////////////////////////////////////////////////////////
      iterator push(const_reference element)
      {
         return container.insert(
            std::upper_bound(
               container.begin(),
               container.end(),
               element,
               Pred()),
            element);
      } // push

      ////////////////////////////////////////////////////////////
      // front
      //
      // Returns a reference to the first element in the container
      ////////////////////////////////////////////////////////////
      reference front() { return container.front(); }
      const_reference front() const { return container.front(); }

      ////////////////////////////////////////////////////////////
      // back
      //
      // Returns a reference to the last element in the container
      ////////////////////////////////////////////////////////////
      reference back() { return container.back(); }
      const_reference back() const { return container.back(); }

      ////////////////////////////////////////////////////////////
      // erase
      //
      // Methods for removing an element or a range of elements
      ////////////////////////////////////////////////////////////
      iterator erase(iterator pos) { return container.erase(pos); }
      void erase(iterator first, iterator last) { container.erase(first,last); }

      ////////////////////////////////////////////////////////////
      // remove
      //
      // The member function removes from the controlled sequence all elements, 
      // designated by the iterator P, for which *P == x.
      ////////////////////////////////////////////////////////////
      void remove(const T& x){ container.remove(x); }

      ////////////////////////////////////////////////////////////
      // size
      ////////////////////////////////////////////////////////////
      size_type size() const { return container.size(); }

      ////////////////////////////////////////////////////////////
      // empty
      ////////////////////////////////////////////////////////////
      bool empty() const { return container.empty(); }

      ////////////////////////////////////////////////////////////
      // clear
      ////////////////////////////////////////////////////////////
      void clear() { container.clear(); }

      ////////////////////////////////////////////////////////////
      // pop_front
      ////////////////////////////////////////////////////////////
      void pop_front() { container.pop_front(); }

   private:
      ////////// container
      container_type container; 
   };
};


#endif
