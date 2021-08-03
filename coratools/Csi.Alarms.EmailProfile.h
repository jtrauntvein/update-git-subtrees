/* Csi.Alarms.EmailProfile.h

   Copyright (C) 2012, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 26 September 2012
   Last Change: Saturday 01 April 2017
   Last Commit: $Date: 2017-04-03 16:25:43 -0600 (Mon, 03 Apr 2017) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Alarms_EmailProfile_h
#define Csi_Alarms_EmailProfile_h

#include "Csi.Xml.Element.h"


namespace Csi
{
   namespace Alarms
   {
      /**
       * Represents the information that controls the transmission of emails by alarms.
       */
      class EmailProfile
      {
      public:
         /**
          * Default constructor
          */
         EmailProfile();

         /**
          * Destructor
          */
         ~EmailProfile();
         
         /**
          * @return Returns the assigned name for this profile.
          */
         StrUni const &get_name() const
         { return name; }

         /**
          * @param name_ Specifies the assigned name for this profile.
          */
         EmailProfile &set_name(StrUni const &name_)
         {
            name = name_;
            return *this;
         }

         /**
          * @return Returns true if the profile is to use the CSI email gateway.
          */
         bool get_use_gateway() const
         { return use_gateway; }

         /**
          * @param value Set to true if the email gateway should be used rather than directly
          * contacting an SMTP server.
          */
         EmailProfile &set_use_gateway(bool value)
         {
            use_gateway = value;
            return *this;
         }
         
         /**
          * @return Returns the address of the SMTP server.
          */
         StrUni const &get_smtp_server() const
         { return smtp_server; }

         /**
          * @param smtp_server_ Specifies the address of the SMTP server that this profile will use.
          */
         EmailProfile &set_smtp_server(StrUni const &smtp_server_)
         {
            smtp_server = smtp_server_;
            return *this;
         }

         /**
          * @return Retuens the name of the account used for authorisation on the SMTP server.
          */
         StrUni const &get_smtp_user_name() const
         { return smtp_user_name; }

         /**
          * @param smtp_user_name_ Specifies the name of the account used for authorisation on the
          * SMTP server.
          */
         EmailProfile &set_smtp_user_name(StrUni const &smtp_user_name_)
         {
            smtp_user_name = smtp_user_name_;
            return *this;
         }

         /**
          * @return Returns the password of the account used for authorisation on the SMTP server.
          */
         StrUni const &get_smtp_password() const
         { return smtp_password; }

         /**
          * @param smtp_password_ Specifies the password of the account used for the authorisation
          * on the SMTP server.
          */
         EmailProfile &set_smtp_password(StrUni const &smtp_password_)
         {
            smtp_password = smtp_password_;
            return *this;
         }

         /**
          * @return Returns the source address for emails sent.
          */
         StrUni const &get_from_address() const
         { return from_address; }

         /**
          * @param from_address_ Specifies the source address for emails sent.
          */
         EmailProfile &set_from_address(StrUni const &from_address_)
         {
            from_address = from_address_;
            return *this;
         }

         /**
          * @return Returns the collection of address to which emails will be sent as a comma
          * separated list.
         */
         StrUni const &get_to_address() const
         { return to_address; }

         /**
          * @param to_address_ Specifies the collection of recipient addresses as a comma separated
          * list.
          */
         EmailProfile &set_to_address(StrUni const &to_address_)
         {
            to_address = to_address_;
            return *this;
         }

         /**
          * @return Returns the collection of addresses to which this email will be copied as a
          * comma-separated list.
          */
         StrUni const &get_cc_address() const
         { return cc_address; }

         /**
          * @param cc_address_ Specifies the collection of addresses to which the email will be
          * copied as a comma-separated list.
          */
         EmailProfile &set_cc_address(StrUni const &cc_address_)
         {
            cc_address = cc_address_;
            return *this;
         }

         /**
          * @return Returns the collection of addressses to which the email will be "blindly" copied
          * as comma-separated list.
          */
         StrUni const &get_bcc_address() const
         { return bcc_address; }

         /**
          * @param bcc_addresses_ Specifies the collection of addresses to which the email will be
          * "blindly" copied as a comma-separated list.
          */
         EmailProfile &set_bcc_address(StrUni const &bcc_address_)
         {
            bcc_address = bcc_address_;
            return *this;
         }

         /**
          * @return Return the unique identifier for this profile.  This value should remain
          * constant and be independent of user input.
          */
         StrUni const &get_unique_id() const
         { return unique_id; }

         /**
          * @param unique_id_ Specifies the unique identifier for this profile.
          */
         EmailProfile &set_unique_id(StrUni const &unique_id_)
         {
            unique_id = unique_id_;
            return *this;
         }

         /**
          * Reads the configuration of this profile from the specified XML element.
          */
         void read(Xml::Element &elem);

         /**
          * Writes the configuration of this profile to the specified XML element.
          */
         void write(Xml::Element &elem);
         
      private:
         StrUni name;
         bool use_gateway;
         StrUni smtp_server;
         StrUni smtp_user_name;
         StrUni smtp_password;
         StrUni from_address;
         StrUni to_address;
         StrUni cc_address;
         StrUni bcc_address;
         StrUni unique_id;
      };
   };
};


#endif

