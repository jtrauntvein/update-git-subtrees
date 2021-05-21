/* Csi.Alarms.TestData.h

   Copyright (C) 2012, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 25 September 2012
   Last Change: Thursday 17 November 2016
   Last Commit: $Date: 2016-11-17 14:43:10 -0600 (Thu, 17 Nov 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Alarms_TestData_h
#define Csi_Alarms_TestData_h
#include "Csi.Alarms.TestBase.h"


namespace Csi
{
   namespace Alarms
   {
      class Alarm;

      
      /**
       * Defines a test for a condition that will evaluate the return value of the alarm's source
       * expression against a trigger expression to determine whether the condition should trigger.
       */
      class TestData: public TestBase
      {
      public:
         /**
          * Constructor
          *
          * @param alarm_ Specifies the alarm that owns this test.
          */
         TestData(Alarm *alarm_);

         /**
          * Destructor
          */
         virtual ~TestData();

         /**
          * Overloads the base class version as a no-op.
          */
         virtual bool on_record(record_handle const &record)
         { return false; }

         /**
          * Evaluates a new value from the alarm source expression.
          */
         virtual bool on_value(operand_handle &value_);

         /**
          * Returns true if this test is triggered.
          */
         virtual bool is_triggered();

         /**
          * Returns true if this test is triggered.
          */
         virtual bool has_on_condition() const;

         /**
          * Specifies the names of elements that configure this test.
          */
         static StrUni const on_expression_name;
         static StrUni const off_expression_name;

         /**
          * Reads the configuration for this test from the specified XML structure.
          */
         virtual void read(Xml::Element &elem);

         /**
          * Writes the configuration for this test to the specified XML structure.
          */
         virtual void write(Xml::Element &elem);

         /**
          * Formats the entrance condition for this test.
          */
         virtual void format_entrance(std::ostream &out);

         /**
          * Formats the exit condition for this test.
          */
         virtual void format_exit(std::ostream &out);

         /**
          * Overloads the base class version to format the value.
          */
         virtual void format_value(std::ostream &out);

         /**
          * Overloads the base class version to format the current value time stamp.
          */
         virtual void format_value_time(std::ostream &out);

         /**
          * Overloads the base class version to recognise when to start testing.
          */
         virtual void on_started();

      private:
         /**
          * Set to true if this test has been triggered.
          */
         bool was_triggered;

         /**
          * Specifies the source of the expression that evaluates whether this test should be
          * triggered.
          */
         StrUni on_expression_str;

         /**
          * Specifies the source of the expression that evaluates whether this test should return to
          * a non-triggered state.
          */
         StrUni off_expression_str;

         /**
          * Specifies the expression that evaluates the transition from off to on.
          */
         typedef Expression::ExpressionHandler expression_type;
         SharedPtr<expression_type> on_expression;

         /**
          * Specifies the expression that evaluates the transistion from on to off.
          */
         SharedPtr<expression_type> off_expression;

         /**
          * Specifies the last value evaluated from the on expresion.
          */
         operand_handle last_on_value;

         /**
          * Specifies the last value evaluated from the off expression.
          */
         operand_handle last_off_value;

         /**
          * Specifies the alarm that owns this test.
          */
         Alarm *alarm;
      };
   };
};


#endif
