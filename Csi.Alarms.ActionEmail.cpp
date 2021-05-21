/* Csi.Alarms.ActionEmail.cpp

   Copyright (C) 2012, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 26 September 2012
   Last Change: Wednesday 13 May 2020
   Last Commit: $Date: 2020-05-13 13:50:49 -0600 (Wed, 13 May 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Alarms.ActionEmail.h"
#include "Csi.Alarms.Manager.h"
#include "Csi.SmtpSender.h"
#include "trace.h"


namespace Csi
{
   namespace Alarms
   {
      /**
       * Defines an object that will carry out the email action.
       */
      class ActionEmail:
         public ActionBase,
         public SmtpSenderClient
      {
      public:
         /**
          * @param action_template Specifies the template from which this action was created.
          */
         ActionEmail(ActionEmailTemplate *action_template);

         /**
          * Destructor
          */
         virtual ~ActionEmail()
         { }

         /**
          * Overloaded to start the process of sending the email.
          */
         virtual void execute();

         /**
          * @return Returns the error description for the last error encountered by this action.
          */
         StrAsc const &get_last_error()
         { return last_error; }

         /**
          * Overloaded to describe this action to the specified XML structure for logging purposes.
          */
         virtual void describe_log(Xml::Element &elem);

         /**
          * Overloads the base class version to handle the completion of the SMTP transaction.
          */
         virtual void on_complete(SmtpSender *sender, outcome_type outcome);

         /**
          * Overloads the base class version to handle logging from the SMTP sender.
          */
         virtual void on_log(SmtpSender *sender, StrAsc const &log);

         /**
          * Overloads the base class version to handle low level logging from the sender.
          */
         virtual void on_bytes_sent(SmtpSender *sender, void const *buff, uint4 buff_len);

         /**
          * Overloads the base class versio to handle low level logging from the sender.
          */
         virtual void on_bytes_received(SmtpSender *sender, void const *buff, uint4 buff_len);

      private:
         /**
          * Specifies the component used for sending the email.
          */
         SharedPtr<SmtpSender> sender;

         /**
          * Specifies the last error encountered with the server.
          */
         StrAsc last_error;

         /**
          * Specifies the content of the email to be sent.
          */
         StrAsc message;
      };


      ActionEmail::ActionEmail(ActionEmailTemplate *action_template):
         ActionBase(action_template)
      {
         OStrAscStream temp;
         action_template->get_condition()->format_desc(
            temp, action_template->get_body_template().to_utf8());
         message = temp.str();
         sender.bind(new SmtpSender(action_template->get_timer()));
      } // constructor


      void ActionEmail::execute()
      {
         ActionEmailTemplate *properties(static_cast<ActionEmailTemplate *>(action_template));
         Manager *manager(properties->get_condition()->get_manager());
         try
         {
            Manager::profile_handle profile(manager->find_profile_id(properties->get_profile()));
            if(profile == nullptr || profile->get_to_address().length() == 0)
               throw std::invalid_argument("no destination address");
            sender->set_use_gateway(profile->get_use_gateway());
            sender->set_server_address(profile->get_smtp_server());
            sender->set_user_name(profile->get_smtp_user_name());
            sender->set_password(profile->get_smtp_password());
            sender->set_from_address(profile->get_from_address());
            sender->set_to_addresses(profile->get_to_address());
            sender->set_cc_addresses(profile->get_cc_address());
            sender->set_bcc_addresses(profile->get_bcc_address());
            sender->set_subject(properties->get_subject());
            sender->set_message(message);
            if(properties->get_attachment().length() > 0)
               sender->add_attachment(properties->get_attachment());
            sender->start(this);
         }
         catch(std::exception &e)
         {
            last_error = e.what();
            report_complete();
         }
      } // execute


      void ActionEmail::describe_log(Xml::Element &elem)
      {
         ActionEmailTemplate *properties(static_cast<ActionEmailTemplate *>(action_template));
         Manager *manager(properties->get_condition()->get_alarm()->get_manager());
         Manager::profile_handle profile(
            manager->find_profile_id(properties->get_profile()));
         Xml::Element::value_type message_xml(elem.add_element(L"message"));

         if(profile != 0)
            elem.set_attr_wstr(profile->get_name(), L"profile");
         else
            elem.set_attr_wstr(L"unrecognised profile", L"profile");
         message_xml->set_cdata_str(message);
         if(sender == 0)
         {
            Xml::Element::value_type outcome_xml(elem.add_element(L"outcome"));
            if(last_error.length() == 0)
               outcome_xml->set_cdata_wstr(L"success");
            else
               outcome_xml->set_cdata_str(last_error);
         }
      } // describe_log


      void ActionEmail::on_complete(SmtpSender *sender_, outcome_type outcome)
      {
         sender.clear();
         if(outcome != outcome_success)
         {
            OStrAscStream temp;
            format_outcome(temp, outcome);
            last_error = temp.str();
         }
         report_complete();
      } // on_complete


      void ActionEmail::on_log(SmtpSender *sender, StrAsc const &log)
      {
         trace("Alarm email: %s", log.c_str());
      } // on_log


      void ActionEmail::on_bytes_sent(
         SmtpSender *sender, void const *buff, uint4 buff_len)
      {
         StrAsc temp;
         temp.append(static_cast<char const *>(buff), buff_len);
         trace(temp.c_str());
      } // on_bytes_sent


      void ActionEmail::on_bytes_received(
         SmtpSender *sender, void const *buff, uint4 buff_len)
      {
         StrAsc temp;
         temp.append(static_cast<char const *>(buff), buff_len);
         trace(temp.c_str());
      } // on_bytes_received

      
      ActionEmailTemplate::ActionEmailTemplate(Condition *condition):
         ActionTemplateBase(condition)
      { }


      ActionEmailTemplate::~ActionEmailTemplate()
      { }


      StrUni const ActionEmailTemplate::body_name(L"body");
      StrUni const ActionEmailTemplate::subject_name(L"subject");
      StrUni const ActionEmailTemplate::profile_name(L"profile");
      StrUni const ActionEmailTemplate::attachment_name(L"attachment");
      

      void ActionEmailTemplate::read(Xml::Element &element)
      {
         using namespace Xml;
         Element::value_type body_xml(element.find_elem(body_name));
         Element::value_type subject_xml(element.find_elem(subject_name));
         Element::iterator ai(element.find(attachment_name));
         profile = element.get_attr_wstr(profile_name);
         body_template = body_xml->get_cdata_wstr();
         subject = subject_xml->get_cdata_wstr();
         if(ai != element.end())
            attachment = (*ai)->get_cdata_str();
         ActionTemplateBase::read(element);
      } // read


      void ActionEmailTemplate::write(Xml::Element &element)
      {
         using namespace Xml;
         Element::value_type body_xml(element.add_element(body_name));
         Element::value_type subject_xml(element.add_element(subject_name));
         Element::value_type attachment_xml(element.add_element(attachment_name));
         element.set_attr_wstr(profile, profile_name);
         body_xml->set_cdata_wstr(body_template);
         subject_xml->set_cdata_wstr(subject);
         attachment_xml->set_cdata_str(attachment);
         ActionTemplateBase::write(element);
      } // write


      void ActionEmailTemplate::perform_action()
      {
         condition->get_alarm()->add_action(new ActionEmail(this));
      } // perform_action
   };
};

