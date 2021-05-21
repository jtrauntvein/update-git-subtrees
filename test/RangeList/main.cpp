/* main.cpp

   Copyright (C) 2003, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Monday 11 August 2003
   Last Change: Monday 11 August 2003
   Last Commit: $Date: 2004/04/30 13:18:39 $ (UTC)
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include <iostream>
#include <algorithm>
#include "Csi.RangeList.h"


namespace
{
   class print_range
   {
   private:
      std::ostream &out;
      
   public:
      print_range(std::ostream &out_):
         out(out_)
      { }
      
      void operator ()(std::pair<uint4, uint4> const &range)
      { out << "[" << range.first << ", " << range.second << "]\n"; }
   };

   
   std::ostream &operator <<(std::ostream &out, Csi::RangeList const &ranges)
   {
      std::for_each(
         ranges.begin(),
         ranges.end(),
         print_range(out));
      return out;
   } // insertion operator


   ////////////////////////////////////////////////////////////
   // print_test_results
   ////////////////////////////////////////////////////////////
   void print_test_results(
      char const *test_name,
      Csi::RangeList const &original,
      Csi::RangeList const &result)
   {
      std::cout << test_name << "\n=========================\n"
                << original << "\nResult:\n"
                << result << "\noriginal - result:\n"
                << Csi::ranges_difference(original,result)
                << "\nresult - original\n" 
                << Csi::ranges_difference(result,original) << "\n\n"; 
   }
};


int main(int argc, char *argv[])
{
   // create the initial ranges
   Csi::RangeList ranges;
   ranges.add_range(0,10);
   ranges.add_range(15,20);
   ranges.add_range(25,30);

   // first test case
   Csi::RangeList ranges2(ranges);
   ranges2.add_range(11,14);
   print_test_results("add [11, 14]",ranges,ranges2);

   // second test case
   Csi::RangeList test3(ranges);
   test3.add_range(0,24);
   print_test_results("add [0, 24]",ranges,test3);

   // third test case
   Csi::RangeList test4(test3);
   test4.remove_range(0,10);
   print_test_results("remove [0, 10]",test3,test4);

   // fourth test case
   Csi::RangeList test5(test4);
   test5.add_range(0,10);
   print_test_results("add [0, 10]",test4,test5);

   // simulate an error encountered with CR10T data advise
   Csi::RangeList test6;
   test6.add_range(11776,15348);
   test6.add_range(15372,15373);
   test6.add_range(15374,15374);
   test6.add_range(15375,15375);
   test6.add_range(15372,15378);
   return 0;
} // main

