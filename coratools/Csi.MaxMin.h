/* Csi.MaxMin.h

   Copyright (C) 2001, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Friday 06 April 2001
   Last Change: Friday 06 April 2001
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef Csi_MaxMin_h
#define Csi_MaxMin_h


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // function csimax
   ////////////////////////////////////////////////////////////
   template <class T>
   T const &csimax(T const &t1, T const &t2)
   {
      if(t1 >= t2)
         return t1;
      else
         return t2;
   }


   ////////////////////////////////////////////////////////////
   // function csimin
   ////////////////////////////////////////////////////////////
   template <class T>
   T const &csimin(T const &t1, T const &t2)
   {
      if(t1 <= t2)
         return t1;
      else
         return t2;
   }
};

#endif
