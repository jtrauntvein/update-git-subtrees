/* Csi.Alarms.EmailProfile.cpp

   Copyright (C) 2012, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 26 September 2012
   Last Change: Thursday 13 April 2017
   Last Commit: $Date: 2017-04-13 15:41:39 -0600 (Thu, 13 Apr 2017) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#define _SCL_SECURE_NO_WARNINGS
#include "Csi.Alarms.EmailProfile.h"
#include "Csi.LoginManager.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"
#include "boost/version.hpp"
#if BOOST_VERSION >= 104200
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#else
#include <uuid/uuid.h>
#endif


namespace Csi
{
   namespace Alarms
   {
      ////////////////////////////////////////////////////////////
      // class EmailProfile definitions
      ////////////////////////////////////////////////////////////
      EmailProfile::EmailProfile():
         smtp_server(L"mail.your-company.com"),
         from_address(L"your-name@your-company.com"),
         to_address(L"someone@your-company.com"),
         use_gateway(true)
      {
#if BOOST_VERSION >= 104200
         boost::uuids::random_generator uuid_gen;
         boost::uuids::uuid uuid(uuid_gen());
         OStrAscStream temp;
         temp << uuid;
         unique_id = temp.str();
#else
         uuid_t uuid;
         char temp[37];
         uuid_generate_random(uuid);
         uuid_unparse(uuid, temp);
         unique_id = temp;
#endif
      } // constructor


      EmailProfile::~EmailProfile()
      { }


      namespace
      {
         StrUni const name_name(L"name");
         StrUni const id_name(L"id");
         StrUni const use_gateway_name(L"use-gateway");
         StrUni const smtp_server_name(L"smtp_server");
         StrUni const smtp_user_name_name(L"smtp_username");
         StrUni const smtp_password_name(L"smtp_password");
         StrUni const from_address_name(L"from_address");
         StrUni const to_address_name(L"to_address");
         StrUni const cc_address_name(L"cc_address");
         StrUni const bcc_address_name(L"bcc_address");
      };

      
      void EmailProfile::read(Xml::Element &elem)
      {
         using namespace Xml;
         StrAsc decrypted_password;
         Xml::Element::iterator use_gateway_it(elem.find(use_gateway_name));
         Xml::Element::iterator smtp_server_it(elem.find(smtp_server_name));
         Xml::Element::iterator smtp_user_name_it(elem.find(smtp_user_name_name));
         Xml::Element::iterator smtp_password_it(elem.find(smtp_password_name));
         Xml::Element::iterator from_address_it(elem.find(from_address_name));

         unique_id = elem.get_attr_wstr(id_name);
         name = elem.get_attr_wstr(name_name);
         if(smtp_server_it != elem.end())
            smtp_server = (*smtp_server_it)->get_cdata_wstr();
         if(smtp_user_name_it != elem.end())
            smtp_user_name = (*smtp_user_name_it)->get_cdata_wstr();
         if(smtp_password_it != elem.end())
         {
            LoginManager::decrypt_password(decrypted_password, (*smtp_password_it)->get_cdata_str());
            smtp_password = decrypted_password;
         }
         if(from_address_it != elem.end())
            from_address = (*from_address_it)->get_cdata_wstr();
         to_address = elem.find_elem(to_address_name)->get_cdata_wstr();
         cc_address = elem.find_elem(cc_address_name)->get_cdata_wstr();
         bcc_address = elem.find_elem(bcc_address_name)->get_cdata_wstr();
         if(use_gateway_it != elem.end())
         {
            Xml::Element::value_type &use_gateway_xml(*use_gateway_it);
            use_gateway = use_gateway_xml->get_cdata_bool();
         }
         else
            use_gateway = false;
      } // read


      void EmailProfile::write(Xml::Element &elem)
      {
         using namespace Xml;
         StrAsc encrypted_password;
         LoginManager::encrypt_password(encrypted_password, smtp_password);
         elem.set_attr_wstr(unique_id, id_name);
         elem.set_attr_wstr(name, name_name);
         elem.add_element(smtp_server_name)->set_cdata_wstr(smtp_server);
         elem.add_element(smtp_user_name_name)->set_cdata_wstr(smtp_user_name);
         elem.add_element(smtp_password_name)->set_cdata_str(encrypted_password);
         elem.add_element(from_address_name)->set_cdata_wstr(from_address);
         elem.add_element(to_address_name)->set_cdata_wstr(to_address);
         elem.add_element(cc_address_name)->set_cdata_wstr(cc_address);
         elem.add_element(bcc_address_name)->set_cdata_wstr(bcc_address);
         elem.add_element(use_gateway_name)->set_cdata_bool(use_gateway);
      } // write
   };
};

