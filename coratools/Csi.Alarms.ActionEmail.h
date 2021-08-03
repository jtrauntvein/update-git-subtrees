/* Csi.Alarms.ActionEmail.h

   Copyright (C) 2012, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 26 September 2012
   Last Change: Monday 21 November 2016
   Last Commit: $Date: 2016-11-21 14:29:41 -0600 (Mon, 21 Nov 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Alarms_ActionEmail_h
#define Csi_Alarms_ActionEmail_h

#include "Csi.Alarms.ActionTemplateBase.h"


namespace Csi
{
   namespace Alarms
   {
      /**
       * Defines a template for an alarm action that will send an email when the alarm is triggered.
       */
      class ActionEmailTemplate: public ActionTemplateBase
      {
      protected:
         /**
          * Specifies the name of the email profile that will be used for this action.
          */
         StrUni profile;
         
         /**
          * Specifies the template for the email message body.
          */
         StrUni body_template;

         /**
          * Specifies the subject line for the alarm.
          */
         StrUni subject;

         /**
          * Specifies the name(s) of any files to be sent as a attachments.
          */
         StrAsc attachment;
         
      public:
         /**
          * Constructor
          *
          * @param condition Spercifies the condition that owns this action.
          */
         ActionEmailTemplate(Condition *condition);
         
         /**
          * Destructor
          */
         virtual ~ActionEmailTemplate();
         
         /**
          * @return Returns the type name for this action.
          */
         virtual StrUni get_type_name() const
         { return "email"; }

         /**
          * Specifies the expected names for attributes an elements.
          */
         static StrUni const body_name;
         static StrUni const subject_name;
         static StrUni const profile_name;
         static StrUni const attachment_name;
         
         /**
          * Reads the configuration for this action from the specified XML structure.
          */
         virtual void read(Xml::Element &elem);
         
         /**
          * Overloaded to read the configuration for this action from the specified XML structure.
          */
         virtual void write(Xml::Element &elem);

         /**
          * @return Returns the subject line for the email.
          */
         StrUni const &get_subject() const
         { return subject; }

         /**
          * @return Returns the template for formatting the email body.
          */
         StrUni const &get_body_template() const
         { return body_template; }

         /**
          * @return Returns the identifier profile for the email.
          */
         StrUni const &get_profile() const
         { return profile; }

         /**
          * @return Returns the file name(s) for the attachment(s) that should be sent with the
          * email.
          */
         StrAsc const &get_attachment() const
         { return attachment; }
         
      protected:
         /**
          * Overloads the base class version to carry out the action.
          */
         virtual void perform_action();
      };
   };
};


#endif
