/* Inline.h

   Declares inline containers based on map and set. The concept of inline containers comes from an
   article "Inline Containers for Variable Arguments" by Andrei Alexandrescu (C/C++ User's Journal
   September, 1998)

Copyright (C) 1998, Campbell Scientific, Inc

Version 1.00
  Written by: Jon Trauntvein
  Date Begun: Saturday 15 August 1998

*/

#ifndef Inline_h
#define Inline_h

#include <map>
#include <set>

////////// template inline_map
// Declares an inline version of a map or multimap container
template <class K, class T, class container = std::map<K, T> >
class inline_map: public container
{
public:
   ////////// default constructor
   inline_map()
   { }

   ////////// construct with a key,value pair
   explicit inline_map(K const &key, T const &value)
   { insert(std::pair<K,T>(key,value)); }

   ////////// insertion operator
   // Overloads the function call operator to accept key and value arguments
   inline_map &operator ()(K const &key, T const &value)
   { insert(std::pair<K,T>(key,value)); return *this; }
};

////////// template make_map
// Template function responsible for the creation of inline maps
template <class K, class T>
inline inline_map<K,T> make_map(K const &key, T const &value)
{ return inline_map<K,T>(key,value); }

////////// template inline_set
// Declares an inline version of the set container template
template <class T, class container = std::set<T> >
class inline_set: public container
{
public:
   ////////// default constructor
   inline_set()
   { }

   ////////// construct with a value
   explicit inline_set(T const &value)
   { insert(value); }

   ////////// insertion operator
   inline_set &operator ()(T const &value)
   { insert(value); return *this; }
};

////////// template make_set
// Template function that creates a inline_set object
template <class T>
inline inline_set<T> make_set(T const &value)
{ return inline_set<T>(value); }

#endif
