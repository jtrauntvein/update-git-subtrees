/* Cora.Broker.ValueFactory.h

   Copyright (C) 1998, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 24 August 1999
   Last Change: Thursday 12 May 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Committed by: $Author: tmecham $
   
*/

#ifndef Cora_Broker_ValueFactory_h
#define Cora_Broker_ValueFactory_h

#include "Csi.SharedPtr.h"
#include "Cora.Broker.Value.h"
#include <exception>


namespace Cora
{
   namespace Broker
   {
      //@group class forward declarations
      class ValueDesc;
      //@endgroup

      ////////// class ValueFactory
      // Defines a class that is responsible for creating value objects based on the criteria specified
      // by a ValueDesc reference. This class can be used as a base class for other factories that
      // extend the types of value objects that are allocated beyond the default set used by this
      // factory. 
      class ValueFactory
      {
      public:
         ////////// class ExcUnsupportedValueType
         // Declares the type of exception thrown when the factory does not support the value data type
         // specified in the description
         class ExcUnsupportedValueType: public std::exception
         {
         public:
            char const *what() const throw ()
            { return "Cora::Broker::ValueFactory: Unsupported data type specified"; }
         };
      
         ////////// make_value
         virtual Csi::SharedPtr<Value> make_value(Csi::SharedPtr<ValueDesc> &description);
      };
   };
};

#endif
