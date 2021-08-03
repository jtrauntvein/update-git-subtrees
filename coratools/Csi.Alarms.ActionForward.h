/* Csi.Alarms.ActionForward.h

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Monday 01 October 2012
   Last Change: Monday 01 October 2012
   Last Commit: $Date: 2012-10-02 11:47:16 -0600 (Tue, 02 Oct 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Alarms_ActionForward_h
#define Csi_Alarms_ActionForward_h

#include "Csi.Alarms.ActionTemplateBase.h"
#include "Csi.Expression.ExpressionHandler.h"


namespace Csi
{
   namespace Alarms
   {
      ////////////////////////////////////////////////////////////
      // class ActionForwardTemplate
      //
      // Defines a template for an action that will forward a value based upon
      // the source expression to a variable identified by a URI. 
      ////////////////////////////////////////////////////////////
      class ActionForwardTemplate: public ActionTemplateBase
      {
      private:
         ////////////////////////////////////////////////////////////
         // forward_expression_str
         ////////////////////////////////////////////////////////////
         StrAsc forward_expression_str;

         ////////////////////////////////////////////////////////////
         // forward_expression
         ////////////////////////////////////////////////////////////
         SharedPtr<Expression::ExpressionHandler> forward_expression;

         ////////////////////////////////////////////////////////////
         // dest_uri
         ////////////////////////////////////////////////////////////
         StrAsc dest_uri;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ActionForwardTemplate(Condition *condition);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ActionForwardTemplate();

         ////////////////////////////////////////////////////////////
         // get_type_name
         ////////////////////////////////////////////////////////////
         virtual StrUni get_type_name() const
         { return L"forward"; }

         ////////////////////////////////////////////////////////////
         // read
         ////////////////////////////////////////////////////////////
         virtual void read(Xml::Element &elem);

         ////////////////////////////////////////////////////////////
         // write
         ////////////////////////////////////////////////////////////
         virtual void write(Xml::Element &elem);

      protected:
         ////////////////////////////////////////////////////////////
         // perform_action
         ////////////////////////////////////////////////////////////
         virtual void perform_action();
      };
   };
};


#endif
