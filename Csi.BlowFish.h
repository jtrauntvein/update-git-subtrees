/* blowfish.h

   _THE BLOWFISH ENCRYPTION ALGORITHM_
   by Bruce Schneier
   Revised code--3/20/94
   Converted to C++ class 5/96, Jim Conger
   Adapted to work with StrBin by Jon Trauntvein
   
   Written by: Jon Trauntvein
   Date Begun: Wednesday 26 January 2005
   Last Change: Wednesday 24 October 2012
   Last Commit: $Date: 2012-10-24 16:54:27 -0600 (Wed, 24 Oct 2012) $ 
   Last Changed by: $Author: jon $ 
*/

#ifndef blowfish_h
#define blowfish_h

#include "CsiTypeDefs.h"
#include "StrBin.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class BlowFish
   //
   // Defines a class that implements the BlowFish encryption algorithm for a
   // specified key.  The key is specified i the constructor.
   ////////////////////////////////////////////////////////////
   class BlowFish
   {
   public:
      ////////////////////////////////////////////////////////////
      // Constructor
      ////////////////////////////////////////////////////////////
      BlowFish(char const *key, uint4 key_len);
      
      ////////////////////////////////////////////////////////////
      // Destructor
      ////////////////////////////////////////////////////////////
      ~BlowFish();
      
      ////////////////////////////////////////////////////////////
      // encrypt
      //
      // encrypts the content of memory stored in the source pointer and stores
      // the results in the dest parameter.  For best results, the length of
      // the source buffer must be evenly divisible by eight.  If it isn't, the
      // algorithm will pad the input with trailing null characters (0).
      //
      // If the force_big_endian flag is set to true, the encrypted output will
      // be reversed if necessary.
      ////////////////////////////////////////////////////////////
      void encrypt(
         StrBin &dest,
         void const *source,
         uint4 source_len,
         bool force_big_endian = false);
      
      ////////////////////////////////////////////////////////////
      // decrypt
      //
      // Performs the inverse function of encrypt.  Assumes that the contents of
      // the source buffer have been previously encrypted using the encrypt
      // function.  The dest parameter will be set to the decrypted content.
      //
      // If the force_big_endian flag is set to true, the encrypted output will
      // be reversed if necessary.
      ////////////////////////////////////////////////////////////
      void decrypt(
         StrBin &dest,
         void const *source,
         uint4 source_len,
         bool force_big_endian = false);
         
   private:
      ////////////////////////////////////////////////////////////
      // blowfish_encipher
      ////////////////////////////////////////////////////////////
      void blowfish_encipher(uint4 *xl, uint4 *xr);
      
      ////////////////////////////////////////////////////////////
      // blowfish_decipher
      ////////////////////////////////////////////////////////////
      void blowfish_decipher(uint4 *xl, uint4 *xr);
      
      ////////////////////////////////////////////////////////////
      // F
      ////////////////////////////////////////////////////////////
      uint4 F(uint4 x);
      
   private:
      ////////////////////////////////////////////////////////////
      // PArray
      ////////////////////////////////////////////////////////////
      uint4 *PArray;
      
      ////////////////////////////////////////////////////////////
      // SBoxes
      ////////////////////////////////////////////////////////////
      uint4 (*SBoxes)[256]; 
   };
};


#endif
