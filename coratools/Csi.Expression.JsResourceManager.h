/* Csi.Expression.JsResourceManager.h

   Copyright (C) 2010, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Wednesday 28 July 2010
   Last Change: Wednesday 28 July 2010
   Last Commit: $Date: 2010-07-28 13:45:47 -0600 (Wed, 28 Jul 2010) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_Expression_JsResourceManager_h
#define Csi_Expression_JsResourceManager_h

#include "Csi.ResourceManager.h"


namespace Csi
{
   namespace Expression
   {
      ////////////////////////////////////////////////////////////
      // class JsResourceManager
      //
      // Defines an object that manages the JavaScript resources for the
      // Expression component.  
      ////////////////////////////////////////////////////////////
      class JsResourceManager: public ResourceManager
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         JsResourceManager();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~JsResourceManager();
      };
   };
};


#endif
