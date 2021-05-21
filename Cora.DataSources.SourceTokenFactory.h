/* Cora.DataSources.SourceTokenFactory.h

   Copyright (C) 2008, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 01 November 2008
   Last Change: Thursday 29 July 2010
   Last Commit: $Date: 2010-07-29 13:17:32 -0600 (Thu, 29 Jul 2010) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_DataSources_SourceTokenFactory_h
#define Cora_DataSources_SourceTokenFactory_h

#include "Csi.Expression.TokenFactory.h"


namespace Cora
{
   namespace DataSources
   {
      ////////////////////////////////////////////////////////////
      // class SourceTokenFactory
      //
      // The functions that used to be defined in a specialised class are now a
      // part of the base class.
      ////////////////////////////////////////////////////////////
      typedef Csi::Expression::TokenFactory SourceTokenFactory;
   };
};


#endif
