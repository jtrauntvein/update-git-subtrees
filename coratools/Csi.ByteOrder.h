/* Csi.ByteOrder.h

   Copyright (C) 2001, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Monday 05 March 2001
   Last Change: Monday 05 March 2001
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef Csi_ByteOrder_h
#define Csi_ByteOrder_h

namespace Csi
{
   ////////////////////////////////////////////////////////////
   // function is_big_endian
   //
   // Returns true if the native processor architecture is big endian (most significant byte comes
   // first).
   ////////////////////////////////////////////////////////////
   inline bool is_big_endian()
   {
      uint2 temp = 0x00FF;
      byte const *temp_ptr = reinterpret_cast<byte const *>(&temp);
      return temp_ptr[1] == 0xFF;
   } // is_big_endian 
};


#endif
