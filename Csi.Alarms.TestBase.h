/* Csi.Alarms.TestBase.h

   Copyright (C) 2012, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 24 September 2012
   Last Change: Monday 19 September 2016
   Last Commit: $Date: 2016-09-19 09:25:11 -0600 (Mon, 19 Sep 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Alarms_TestBase_h
#define Csi_Alarms_TestBase_h

#include "Cora.Broker.Record.h"
#include "Csi.Xml.Element.h"
#include "Csi.Expression.ExpressionHandler.h"


namespace Csi
{
   namespace Alarms
   {
      ////////////////////////////////////////////////////////////
      // class TestBase
      //
      // Defines a class that is responsible for performing the test for an
      // alarm condition. 
      ////////////////////////////////////////////////////////////
      class TestBase
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         TestBase()
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~TestBase()
         { }

         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started()
         { }

         ////////////////////////////////////////////////////////////
         // on_stopped
         ////////////////////////////////////////////////////////////
         virtual void on_stopped()
         { }

         ////////////////////////////////////////////////////////////
         // on_record
         //
         // Called when a new record has become available.  If the return value
         // is true, the condition that invokes this test should consider
         // itself triggered.  If false, the condition should consider itself
         // not triggered.  
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Cora::Broker::Record> record_handle;
         virtual bool on_record(record_handle const &record) = 0;

         ////////////////////////////////////////////////////////////
         // is_triggered
         //
         // Called to determine whether this tesxt has been triggered.
         ////////////////////////////////////////////////////////////
         virtual bool is_triggered() = 0;

         ////////////////////////////////////////////////////////////
         // has_on_condition
         //
         // Called to determine whether the on condition is still asserted
         // while ignoring the off condition. 
         ////////////////////////////////////////////////////////////
         virtual bool has_on_condition() const = 0;

         ////////////////////////////////////////////////////////////
         // on_value
         //
         // Called when a new value from the alarm's source expression has been
         // evaluated.   Like on_record(), a return value of true will indicate
         // that the condition should be triggered and a value of false will
         // indicate that the condition should be not triggered.
         ////////////////////////////////////////////////////////////
         typedef Expression::ExpressionHandler::operand_handle operand_handle;
         virtual bool on_value(operand_handle &value) = 0;

         ////////////////////////////////////////////////////////////
         // read
         //
         // Must be overloaded to read parameters from the specified XML
         // element.
         ////////////////////////////////////////////////////////////
         virtual void read(Xml::Element &elem) = 0;

         ////////////////////////////////////////////////////////////
         // write
         //
         // Must be overloaded to write parameters for this test to the
         // specified XML element.
         ////////////////////////////////////////////////////////////
         virtual void write(Xml::Element &elem) = 0;

         ////////////////////////////////////////////////////////////
         // format_entrance
         ////////////////////////////////////////////////////////////
         virtual void format_entrance(std::ostream &out) = 0;

         ////////////////////////////////////////////////////////////
         // format_exit
         ////////////////////////////////////////////////////////////
         virtual void format_exit(std::ostream &out) = 0;

         ////////////////////////////////////////////////////////////
         // format_value
         ////////////////////////////////////////////////////////////
         virtual void format_value(std::ostream &out)
         { }

         /**
          * Formats the current value time stampt to the specified stream.
          */
         virtual void format_value_time(std::ostream &out)
         { }
      };
   };
};


#endif
