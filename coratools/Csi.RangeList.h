/* Csi.RangeList.h

   Copyright (C) 2003, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 08 August 2003
   Last Change: Friday 18 December 2015
   Last Commit: $Date: 2015-12-18 17:50:05 -0600 (Fri, 18 Dec 2015) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_RangeList_h
#define Csi_RangeList_h

#include "CsiTypeDefs.h"
#include <list>


namespace Csi
{
   /**
    * This class defines a component that allows the application to keep track
    * of a list of number ranges.  The services provided by this component are
    * as follows:
    *
    *   - The list of number ranges are maintained so that there is no overlao
    *     between individual ranges.  Existing ranges will be expanded and/or
    *     merged to accomodate a new range.
    *
    *   - The set of ranges will be kept to a minimum.  For instance, if a new
    *     range is inserted that is adjacent to an existing range, the existing
    *     range will be expanded to include the new range rather than creating
    *     a new entry.
    *
    *   - Ranges can be removed from anywhere in the list.  Existing ranges
    *     will be divided as needed to accomodate resulting voids.
    *
    *   - If any range is added that has a beginning value greater than or
    *     equal to the ending value (as can happen with datalogger record
    *     numbers), two new ranges will be inserted so that simple numeric
    *     comparisons can be used.
    *
    *   - The application will be able to query the component for set
    *     membership of individual values as well as intersection of ranges.
    */
   class RangeList
   {
      /**
       * Specifies the list of ranges maintainedf by this component.
       */
   public:
      typedef std::list<std::pair<uint4,uint4> > ranges_type;
   private:
      ranges_type ranges;
      
   public:
      /**
       * Default constructor.
       */
      RangeList()
      { }

      /**
       * Constructor that initialises this component with a range of numbers.
       */
      RangeList(uint4 range_begin, uint4 range_end)
      { add_range(range_begin,range_end); }

      /**
       * Copy constructor.
       */
      RangeList(RangeList const &other):
         ranges(other.ranges)
      { }

      /**
       * Copy operator.
       */
      RangeList &operator =(RangeList const &other)
      {
         ranges = other.ranges;
         return *this;
      } 

      // @group: container access methods

      /**
       * @return Returns the first iterator for the set of ranges.
       */
      typedef ranges_type::const_iterator const_iterator;
      const_iterator begin() const
      { return ranges.begin(); }

      /**
       * @return Returns the last iterator for the set of ranges.
       */
      const_iterator end() const
      { return ranges.end(); }

      /**
       * Returns the first reverse iterator for the set of ranges.
       */
      typedef ranges_type::const_reverse_iterator const_reverse_iterator;
      const_reverse_iterator rbegin() const
      { return ranges.rbegin(); }

      /**
       * @return Returns the last reverse iterator for the set of ranges.
       */
      const_reverse_iterator rend() const
      { return ranges.rend(); }

      /**
       * @return Returns the number of ranges.
       */
      typedef ranges_type::size_type size_type;
      size_type size() const
      { return ranges.size(); }

      /**
       * @return Returns true if there are no ranges.
       */
      bool empty() const
      { return ranges.empty(); }

      /**
       * Removes all ranges.
       */
      void clear()
      { ranges.clear(); }

      /**
       * @return Returns the first range.
       */
      typedef ranges_type::value_type value_type;
      value_type const &front() const
      { return ranges.front(); }

      /**
       * @return Returns the last range.
       */
      value_type const &back() const
      { return ranges.back(); }
      
      // @endgroup:

      /**
       * Adds a new range.  If the begin is less than the end, two new ranges
       * will be added at each end of the number line.  If this range overlaps
       * any existing range, the set of ranges will be consolidated.
       *
       * @param range_begin Specifies the first value for the range to add.
       *
       * @param range_end  Specifies the end value for the range to add.
       */
      void add_range(uint4 range_begin, uint4 range_end);

      /**
       * Removes a range of numbers from the set maintained by this component.
       * If the specified range lies in the middle of an existing range, the
       * existing range will be divided into two subranges.
       *
       * @param range_begin Specifies the start of the range to remove.
       *
       * @param range_end Specifies the end of the range to remove.
       */
      void remove_range(uint4 range_begin, uint4 range_end);

      /**
       * Removes all ranges that overlap with the specified range.
       *
       * @param range_begin Specifies the start of the range to be removed.
       *
       * @param range_end Specifies the end of the range to be removed.
       */
      void remove_complete_ranges(uint4 range_begin, uint4 range_end);
      
      /**
       * @return Returns true if the specified number is within one of the
       * contained ranges.
       *
       * @param value Specifies the value to search for.
       */
      bool is_element(uint4 value) const;

      /**
       * @return Returns true if the specified range of values overlaps with
       * values contained within this set.
       *
       * @param range_begin Specifies the start of the test range.
       *
       * @param range_end Specifies the end of the test range.
       */
      bool overlaps(uint4 range_begin, uint4 range_end) const;

      /**
       * @return Returns the total number of values contained in this set of
       * ranges.
       */
      uint4 set_size() const;

      /**
       * Makes copies of all of the ranges in this set into the supplied list.
       *
       * @param output Reference to the container to which the ranges will be
       * written.
       *
       * @param wrap_value Specifies the number that orders the ranges that
       * will be output.  The first range output will be greater than the wrap.
       */
      void order_ranges(ranges_type &output, uint4 wrap_value) const;

   private:
#ifdef _DEBUG
      void check_invariant();
#endif
   };


   /**
    * @return Retuns the union of the two specified range lists.
    *
    * @param set1 Specifies the first set to union.
    *
    * @param set2 Specifies the second set to union.
    */
   RangeList ranges_union(RangeList const &set1, RangeList const &set2);


   /**
    * @return Returns the range of numbers that are in the first set that are
    * not in the second set.
    *
    * @param set1 Specifies the first set.
    *
    * @param set2 Specifies the second set.
    */
   RangeList ranges_difference(RangeList const &set1, RangeList const &set2);
   inline RangeList operator -(RangeList const &set1, RangeList const &set2)
   { return ranges_difference(set1,set2); }


   /**
    * @return Returns the set of numbers that are common between both specified
    * sets.
    *
    * @param set1 Specifies the first set.
    *
    * @param set2 Specifies the second set.
    */
   inline RangeList ranges_intersection(RangeList const &set1, RangeList const &set2)
   {
      return ranges_difference(
         set1,
         ranges_difference(set1,set2));
   }


   /**
    * @return Returns the set of numbers that are common between the first set
    * and the specified range of numbers.
    *
    * @param set1 Specifies the first set.
    *
    * @param range_begin Specifies the start of the second range.
    *
    * @param range_end Specifies the end of the second range.
    */
   inline RangeList ranges_intersection(
      RangeList const &set1,
      uint4 range_begin, uint4 range_end)
   {
      return ranges_intersection(
         set1,
         RangeList(
            range_begin,
            range_end));
   }
};


#endif
