/* Csi.RangeList.cpp

   Copyright (C) 2003, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 08 August 2003
   Last Change: Friday 18 December 2015
   Last Commit: $Date: 2015-12-18 17:50:05 -0600 (Fri, 18 Dec 2015) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include <algorithm>
#include <assert.h>
#include "Csi.RangeList.h"
#include "Csi.MaxMin.h"
using namespace std;


namespace Csi
{
   namespace
   {
      ////////////////////////////////////////////////////////////
      // class first_value_less
      //
      // Defines a predicate that evaluates whether the first value of the
      // first range is less than the first value of the second range
      ////////////////////////////////////////////////////////////
      typedef std::pair<uint4, uint4> range_type;
      class first_value_less
      {
      public:
         bool operator ()(range_type const &p1, range_type const &p2)
         { return p1.first < p2.first; }
      };


      ////////////////////////////////////////////////////////////
      // ranges_overlap
      ////////////////////////////////////////////////////////////
      bool ranges_overlap(
         uint4 begin1, uint4 end1,
         uint4 begin2, uint4 end2)
      {
         return
            (begin1 >= begin2 && begin1 <= end2) ||
            (end1 >= begin2 && end1 <= end2) ||
            (begin2 >= begin1 && begin2 <= end1) ||
            (end2 >= begin1 && end2 <= end1); 
      } // ranges_overlap


      ////////////////////////////////////////////////////////////
      // ranges_are_adjacent
      ////////////////////////////////////////////////////////////
      bool ranges_are_adjacent(
         uint4 begin1, uint4 end1,
         uint4 begin2, uint4 end2)
      {
         return
            (begin2 == end1 + 1 && begin2 != 0 && end1 != UInt4_Max) ||
            (begin1 == end2 + 1 && begin1 != 0 && end2 != UInt4_Max);
      } // ranges_are_adjacent
      
      
      ////////////////////////////////////////////////////////////
      // function ranges_meet
      //
      // Predicate evaluates whether the two specified ranges are adjacent to
      // or overlap one another. 
      ////////////////////////////////////////////////////////////
      bool ranges_meet(
         uint4 begin1, uint4 end1,
         uint4 begin2, uint4 end2)
      {
         return
            ranges_overlap(begin1,end1,begin2,end2) ||
            ranges_are_adjacent(begin1,end1,begin2,end2);
      } // ranges_meet


      ////////////////////////////////////////////////////////////
      // is_element_of
      ////////////////////////////////////////////////////////////
      class is_element_of
      {
      private:
         uint4 value;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         is_element_of(uint4 value_):
            value(value_)
         { }

         ////////////////////////////////////////////////////////////
         // evaluator
         ////////////////////////////////////////////////////////////
         bool operator() (range_type const &range)
         { return value >= range.first && value <= range.second; }
      };


      ////////////////////////////////////////////////////////////
      // class intersects_with
      ////////////////////////////////////////////////////////////
      class intersects_with
      {
      private:
         uint4 range_begin, range_end;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         intersects_with(uint4 range_begin_, uint4 range_end_):
            range_begin(range_begin_),
            range_end(range_end_)
         { }

         ////////////////////////////////////////////////////////////
         // evaluator
         ////////////////////////////////////////////////////////////
         bool operator ()(range_type const &range)
         { return ranges_overlap(range_begin,range_end,range.first,range.second); }
      };
   };

   
   ////////////////////////////////////////////////////////////
   // class RangeList definitions
   ////////////////////////////////////////////////////////////
   void RangeList::add_range(uint4 range_begin, uint4 range_end)
   {
      // normal ranges should be configured such that the begin is less than or
      // equal to the end.  If this isn't the case, we will deal with it by
      // adding two sub-ranges
      if(range_begin > range_end)
      {
         add_range(range_begin,UInt4_Max);
         add_range(0,range_end);
         return;
      }

      // if the list of ranges is empty, we're done
      if(ranges.empty())
      {
         ranges.push_back(make_pair(range_begin,range_end));
         return;
      }

      // search for the first range that has a start >= the range start.  We
      // will then position at the range previous to that if available
      ranges_type::iterator ri = lower_bound(
         ranges.begin(),
         ranges.end(),
         make_pair(range_begin,range_begin),
         first_value_less());
      ranges_type::iterator previous = ri;
      if(previous != ranges.begin())
         --previous;

      // if there is no overlap in this range, we need to insert a new range at
      // after this position
      assert(previous != ranges.end());
      if(!ranges_meet(range_begin,range_end,previous->first,previous->second))
         previous = ranges.insert(ri,make_pair(range_begin,range_end));
      else
      {
         // otherwise we absorb the new range into the previous
         previous->first = csimin(previous->first,range_begin);
         previous->second = csimax(previous->second,range_end);
      }

      // we now need to loop from the insertion position toward the end of the
      // list and adjust each range that meets as we go down.  This will go on
      // until we encounter a range that does not meet with the previous or
      // until we reach the end of the list
      while(previous != ranges.end())
      {
         ri = previous;
         if(++ri == ranges.end())
            break;
         if(ranges_meet(previous->first,previous->second,ri->first,ri->second))
         {
            previous->first = csimin(previous->first,ri->first);
            previous->second = csimax(previous->second,ri->second);
            ranges.erase(ri);
         }
         else
            break;
      }
   } // add_range


   void RangeList::remove_range(uint4 range_begin, uint4 range_end)
   {
      // we need to do the same sort of translation for the inputs of this
      // function as we did for add_range().  That is, we need to convert an
      // inverted range into two ranges and deal with each recursively.
      if(range_end < range_begin)
      {
         remove_range(range_begin,UInt4_Max);
         remove_range(0,range_end);
         return;
      }
      
      // the algorithm for removing a range is to iterate through the ranges
      // until we meet the end of the list or until the ranges are larger than
      // the specified range.  Any range that overlaps the specified range will
      // be either deleted or split as needed.
      ranges_type::iterator ri = ranges.begin();
      while(ri != ranges.end())
      {
         if(ranges_overlap(range_begin,range_end,ri->first,ri->second))
         {
            if(range_begin <= ri->first && range_end >= ri->second)
            {
               ranges_type::iterator dri = ri++;
               ranges.erase(dri);
            }
            else
            {
               bool range_changed = false; 
               uint4 first = ri->first, second = ri->second;

               if(first < range_begin)
               {
                  ri->second = range_begin - 1;
                  range_changed = true;
               }
               if(second > range_end)
               {
                  if(range_changed)
                  {
                     ++ri;
                     ri = ranges.insert(
                        ri,
                        make_pair(range_end + 1,second));
                  }
                  else
                     ri->first = range_end + 1;
               }
               ++ri;
            }
         }
         else if(ri->first <= range_begin)
            ++ri;
         else
            break;
      }
   } // remove_range


   void RangeList::remove_complete_ranges(uint4 range_begin, uint4 range_end)
   {
      ranges_type::iterator ri(ranges.begin());
      while(ri != ranges.end())
      {
         if(ranges_overlap(range_begin, range_end, ri->first, ri->second))
         {
            ranges_type::iterator dri(ri++);
            ranges.erase(dri);
         }
         else
            ++ri;
      }
   } // remove_complete_ranges


   bool RangeList::is_element(uint4 value) const
   {
      ranges_type::const_iterator ri = std::find_if(
         ranges.begin(),
         ranges.end(),
         is_element_of(value));
      return ri != ranges.end();
   } // is_element


   bool RangeList::overlaps(uint4 range_begin, uint4 range_end) const
   {
      bool rtn = false;
      for(ranges_type::const_iterator ri = ranges.begin();
          !rtn && ri != ranges.end();
          ++ri)
      {
         rtn = ranges_overlap(
            ri->first, ri->second,
            range_begin, range_end);
      }
      return rtn;
   } // overlaps


   uint4 RangeList::set_size() const
   {
      uint4 rtn = 0;
      for(ranges_type::const_iterator ri = ranges.begin();
          ri != ranges.end();
          ++ri)
         rtn += ri->second - ri->first + 1;
      return rtn;
   } // set_size


   void RangeList::order_ranges(ranges_type &output, uint4 wrap_value) const
   {
      // find the raange that begins in [wrap_value, UInt4_max]
      ranges_type::const_iterator oldest = ranges.begin();
      
      while(oldest != ranges.end() && oldest->first < wrap_value)
         ++oldest;

      // we now start at the oldest and output each range from that value on to the range just
      // preceding the oldest.
      output.clear();
      if(!ranges.empty())
      {
         ranges_type::const_iterator current = oldest;
         uint4 output_count = 0;

         do
         {
            if(current == ranges.end())
               current = ranges.begin();
            ++output_count;
            if(!output.empty() && output.back().second == current->first - 1)
               output.back().second = current->second;
            else
               output.push_back(*current);
            ++current;
         }
         while(output_count < ranges.size() && current != oldest);
      }
   } // order_ranges


#ifdef _DEBUG
   void RangeList::check_invariant()
   {
      ranges_type::iterator present = ranges.begin();
      if(present != ranges.end())
      {
         ranges_type::iterator next = present;
         ++next;
         while(next != ranges.end())
         {
            assert(
               !ranges_meet(
                  present->first,present->second,
                  next->first,next->second) &&
               next->first > present->second);
            ++present;
            ++next;
         }
      }
   } // check_invariant
#endif


   ////////////////////////////////////////////////////////////
   // function ranges_union definition
   ////////////////////////////////////////////////////////////
   RangeList ranges_union(RangeList const &set1, RangeList const &set2)
   {
      RangeList rtn(set1);
      for(RangeList::const_iterator ri = set2.begin();
          ri != set2.end();
          ++ri)
         rtn.add_range(ri->first,ri->second);
      return rtn;
   } // ranges_union


   ////////////////////////////////////////////////////////////
   // function ranges_difference
   ////////////////////////////////////////////////////////////
   RangeList ranges_difference(RangeList const &set1, RangeList const &set2)
   {
      RangeList rtn(set1);
      for(RangeList::const_iterator ri = set2.begin();
          ri != set2.end();
          ++ri)
         rtn.remove_range(ri->first,ri->second);
      return rtn;
   } // ranges_difference
};
