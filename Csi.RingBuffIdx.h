/* Csi.RingBuffIdx.h

   Copyright (C) 1998, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 19 January 1998
   Last Change: Friday 25 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_RingBuffIdx_h
#define Csi_RingBuffIdx_h


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // template class RingBuffIdx
   //
   // Defines a template class that performs calculations on an index of the type named by the template
   // parameter that is intended to wrap.
   ////////////////////////////////////////////////////////////
   template<class T>
   class RingBuffIdx
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      //
      // The mIdx_ parameter initialises the point at which the index is expected to wrap. 
      ////////////////////////////////////////////////////////////
      RingBuffIdx(T mIdx_):
         mIdx(mIdx_)
      { }

      ////////////////////////////////////////////////////////////
      // within
      //
      // Returns true if the specified position is considered to be "within" the closed interval of beg
      // and end.  If beg is greater than end, the function will evaluate whether the position lies in
      // the intervals of [beg, mIdx] or [0, end].  The return value will be true if the position is
      // determined to be in the interval.
      ////////////////////////////////////////////////////////////
      bool within(T pos, T beg, T end)
      {
         bool rtn = false;
         if(beg <= end)
            rtn = (beg <= pos && pos <= end);
         else
            rtn = ((beg <= pos && pos <= mIdx) || (0 <= pos && pos <= end));
         return rtn;
      }

      ////////////////////////////////////////////////////////////
      // overlap
      //
      // Calculates the size of the overlap region between two closed intervals, [b1, e1] and
      // [b2, e2].  If either of these intervals crosses the mIdx boundary, they will be divided into
      // two sub intervals and the sum of the overlap on those sub-intervals will be used. 
      ////////////////////////////////////////////////////////////
      T overlap(T b1, T e1, T b2, T e2)
      {
         T rtn;

         // check for the case where both regions are normal
         if(b1 <= e1 && b2 <= e2)
            rtn = normalOverlap(b1,e1,b2,e2);
         // check for both regions crossing
         else if(b1 > e1 && b2 > e2)
            rtn = normalOverlap(b1,mIdx,b2,mIdx) + normalOverlap(0,e1,0,e2);
         // check for region1 crossing
         else if(b1 > e1)
            rtn = normalOverlap(b1,mIdx,b2,e2) + normalOverlap(0,e1,b2,e2);
         // if we made it here, than region2 must be crossing
         else
            rtn = normalOverlap(b1,e1,b2,mIdx) + normalOverlap(b1,e1,0,e2);
         return rtn;
      }

      ////////////////////////////////////////////////////////////
      // normalOverlap
      //
      // Calculates the size of the overlap between two closed intervals [b1, e1] and [b2, e2] where it
      // is assumed b1 <= e1 and b2 <= e2.  The overlap method should be used when it is possible for
      // b1 > e1 or b2 > e2.
      ////////////////////////////////////////////////////////////
      T normalOverlap(T b1, T e1, T b2, T e2)
      {
         // depending on the relative positions of the two regions, the overlap is
         // calculated in varying ways
         T rtn;
      
         if(b2 >= b1 && e2 <= e1)
            rtn = e2 - b2 + 1;        // 2a encompassed by 1a
         else if(b1 >= b2 && e1 <= e2)
            rtn = e1 - b1 + 1;        // 1a encompassed by 2a
         else if(b1 <= b2 && b2 <= e1 && e1 <= e2)
            rtn = e1 - b2 + 1;        // overlap with 1a first
         else if(b2 <= b1 && b1 <= e2 && e2 <= e1)
            rtn = e2 - b1 + 1;        // overlap with 2a first
         else
            rtn = 0;                  // no overlap
         return rtn; 
      } 
   
      ////////////////////////////////////////////////////////////
      // diff
      //
      // Returns the difference between the begin point and the end point.  
      ////////////////////////////////////////////////////////////
      T diff(T beg, T end)
      {
         T rtn;
         if(end >= beg)
            rtn = end - beg;
         else
            rtn = end + (mIdx - beg);
         return rtn;
      }

      ////////////////////////////////////////////////////////////
      // fromPos
      //
      // Calculates a position in the buffer that is offset values behind the specified anchor in the
      // index. 
      ////////////////////////////////////////////////////////////
      T fromPos(T anchor, T offset)
      {
         T rtn;
         if(anchor >= offset)
            rtn = anchor - offset;
         else
            rtn = mIdx - (offset - anchor);
         return rtn;
      }
   
   private:
      ////////// mIdx
      // The largest index in the buffer
      T mIdx; 
   }; 
};


#endif
