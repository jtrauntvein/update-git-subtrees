/* Csi.DevConfig.SummaryConverterBase.h

   Copyright (C) 2007, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 19 April 2007
   Last Change: Thursday 16 April 2009
   Last Commit: $Date: 2010-10-01 15:42:12 -0600 (Fri, 01 Oct 2010) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_DevConfig_SummaryConverterBase_h
#define Csi_DevConfig_SummaryConverterBase_h

#include "Csi.DevConfig.ConfigSummary.h"


namespace Csi
{
   namespace DevConfig
   {
      ////////////////////////////////////////////////////////////
      // class SummaryConverterBase
      //
      // Defines a base class for an object that can convert device
      // configuration cummary files from one major version to another.  This
      // class also defines static methods that allow access to the set of
      // registered converters.  
      ////////////////////////////////////////////////////////////
      class SummaryConverterBase
      {
      private:
         ////////////////////////////////////////////////////////////
         // dest_version
         ////////////////////////////////////////////////////////////
         byte const dest_version;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SummaryConverterBase(byte dest_version_):
            dest_version(dest_version_)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SummaryConverterBase()
         { }
         
         ////////////////////////////////////////////////////////////
         // do_conversion
         //
         // Performs the conversion operation.  This default version assumes
         // that setting identifiers mean the same thing in both versions.  If
         // this is not the case, this method must be overloaded and a custom
         // policy instituted.
         ////////////////////////////////////////////////////////////
         virtual ConfigSummary *do_conversion(ConfigSummary *source);

         ////////////////////////////////////////////////////////////
         // convert_setting
         //
         // Can be overloaded to convert a particular setting
         ////////////////////////////////////////////////////////////
         virtual void convert_setting(
            ConfigSummary::value_type &dest_setting,
            ConfigSummary::value_type &source_setting);

         ////////////////////////////////////////////////////////////
         // after_convert
         //
         // Called by convert() after all of the settings have been
         // enumerated.  This gives the converter a chance to make decisions
         // once all of the settings have been considered.
         ////////////////////////////////////////////////////////////
         virtual void after_convert()
         { }
         
         ////////////////////////////////////////////////////////////
         // convert
         //
         // Looks for the registered converter and invokes its do_conversion()
         // method if found.  If no converter is found or the conversion fails,
         // an exception derived from std::exception will be thrown.  The
         // converted summary will be returned. 
         ////////////////////////////////////////////////////////////
         static ConfigSummary *convert(
            ConfigSummary *source,
            byte dest_version);
      };
   };
};


#endif
