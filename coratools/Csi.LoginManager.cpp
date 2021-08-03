/* Csi.LoginManager.cpp

   Copyright (C) 2005, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 26 January 2005
   Last Change: Friday 02 August 2019
   Last Commit: $Date: 2019-08-02 12:00:58 -0600 (Fri, 02 Aug 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.LoginManager.h"
#include "Csi.Utils.h"
#include "Csi.BlowFish.h"
#include "Csi.Xml.Element.h"
#include "Csi.RegistryManager.h"
#include "Csi.SharedPtr.h"
#include "MsgExcept.h"
#include "Csi.fstream.h"
#include <sstream>
#include <iomanip>
#include <algorithm>


namespace Csi
{
   namespace
   {
      /**
       * Specifies the object that will be used to encrypt and decrypt passwords.
       */
      char const *encryptor_key = "0TOG799A"; 
      BlowFish encryptor(encryptor_key, (uint4)strlen(encryptor_key));
   
      /**
       * Defines a predicate that determines whether a given login is the same as another.
       */
      class login_eq
      {
      private:
         LoginManager::login_type login;

      public:
         typedef LoginManager::login_type login_type;
         login_eq(login_type const &login_):
            login(login_)
         { }

         bool operator () (login_type const &other) const
         { return login.server_name == other.server_name; }
      };
   };

   
   LoginManager::LoginManager(StrAsc const &app_name, StrAsc const &version):
      is_automatic(false),
      max_logins(30),
      locked(false)
   {
      // the location of the ini file path will depend on the application name and version
      get_work_dir(ini_file_path,app_name,version);
#ifdef _WIN32
      if(ini_file_path.last() != '\\')
         ini_file_path.append('\\');
      ini_file_path.append("sys\\inifiles\\");
#else
      if(ini_file_path.last() != '/')
         ini_file_path.append('/');
      ini_file_path.append("sys/inifiles/");
#endif
      login_file_path = ini_file_path + "login.xml";
      
      // we now need to attempt to read this file.  If there is nothing in the file, we will create
      // a record with a local default. 
      logins.push_back(login_type());
   } // constructor


   void LoginManager::set_last_login(
      StrAsc const &server_name,
      StrUni const &login_name,
      StrUni const &password,
      bool remembered,
      bool is_automatic)
   {
      // we will eliminate this login if it already matches one of the other logins
      login_type login(server_name,login_name,password,remembered);
      this->is_automatic = is_automatic;
      logins_type::iterator dupit = std::find_if(
         logins.begin(),
         logins.end(),
         login_eq(login));
      if(dupit != logins.end())
         logins.erase(dupit);
      logins.push_front(login);

      // we need to elminate any history over the past four logins
      if(!locked)
      {
         while(logins.size() > max_logins)
            logins.pop_back();
         save();
      }
   } // set_last_login


   void LoginManager::refresh()
   {
      logins.clear();
      try
      {
         Csi::ifstream file(login_file_path.c_str());
         Xml::Element root(L"");
         root.input(file);

         if(root.get_name() == L"login")
         {
            StrUni temp;

            temp = root.get_attribute(L"automatic");
            if(temp == L"true" || temp == L"1")
               is_automatic = true;
            else
               is_automatic = false;
            if(root.has_attribute(L"max-logins"))
               max_logins = root.get_attr_uint4(L"max-logins");
            if(root.has_attribute(L"locked"))
               locked = root.get_attr_bool(L"locked");
            for(Xml::Element::iterator ei = root.begin(); ei != root.end(); ++ei)
            {
               SharedPtr<Xml::Element> &child = *ei;
               if(child->get_name() == L"server")
               {
                  login_type login;
                  SharedPtr<Xml::Element> user_name(child->find_elem(L"username",0,true));
                  StrAsc decrypted_pass;

                  login.server_name = child->get_attr_str(L"name");
                  login.user_name = user_name->get_cdata_wstr();
                  if(login.user_name.length())
                  {
                     SharedPtr<Xml::Element> password(child->find_elem(L"password"));
                     login.password = decrypt_password(
                        decrypted_pass,
                        password->get_cdata_str().c_str());
                     login.remembered = true;
                  }
                  else
                     login.remembered = (is_automatic && ei == root.begin());
                  logins.push_back(login);
               }
            }
         }
      }
      catch(std::exception &)
      { }
      if(logins.empty())
         logins.push_back(login_type());
   } // refresh


   void LoginManager::save()
   {
      if(!locked)
      {
         // create the root element
         Xml::Element root(L"login");
         root.set_attr_bool(is_automatic,L"automatic");
         root.set_attr_bool(locked, L"locked");
         root.set_attr_uint4(max_logins, L"max_logins");
         
         // now save each of the server elements
         StrAsc temp;
         for(logins_type::const_iterator li = logins.begin(); li != logins.end(); ++li)
         {
            login_type const &login = *li;
            SharedPtr<Xml::Element> login_xml(root.add_element(L"server"));
            
            login_xml->set_attr_str(login.server_name,L"name");
            if(login.remembered && login.user_name.length() > 0)
            {
               SharedPtr<Xml::Element> user_name(login_xml->add_element(L"username"));
               SharedPtr<Xml::Element> password(login_xml->add_element(L"password"));
               user_name->set_cdata_wstr(login.user_name);
               password->set_cdata_str(encrypt_password(temp,login.password));
            }
         }
         
         // we now need to save the contents of the XML model to file.
         createNestedDir(get_path_from_file_name(temp,login_file_path.c_str()));
         Csi::ofstream out(login_file_path.c_str());
         if(!out)
            throw OsException("Failed to create the Login File");
         root.output(out,true);
         out << "\n\n";
      }
   } // save


   StrAsc LoginManager::get_ini_file_path()
   { 
      StrAsc rtn(ini_file_path);
      if(!logins.empty())
      {
         StrAsc address(get_server_address());
         if(address != "localhost" && address != "::1" && address != "127.0.0.1")
         {
            address.replace(":", "_");
            rtn += address;
#ifdef _WIN32
            rtn.append('\\');
#else
            rtn.append('/');
#endif
         }
      } 
      return rtn;
   } // get_ini_file_path


   char const *LoginManager::encrypt_password(
      StrAsc &dest,
      StrUni const &password)
   {
      // the unicode version needs to be converted into multi byte
      StrAsc multi = password.to_utf8();
      
      // we can now encrypt the multi-byte version of the password
      StrBin encrypted;
      encryptor.encrypt(encrypted,multi.c_str(), (uint4)multi.length());
      
      // we now need to write the encrypted bytes out as hex values
      std::ostringstream temp;
      for(uint4 i = 0; i < encrypted.length(); ++i)
      {
         temp << std::hex << std::setw(2) << std::setfill('0')
              << uint4(encrypted[i]);
      }
      dest = temp.str().c_str();
      return dest.c_str();
   }
   
   
   char const *LoginManager::decrypt_password(
      StrAsc &dest,
      StrAsc const &source)
   {
      // we need to accumulate the string binary string as hex encoded
      StrBin encrypted;
      char accum[3];
      int accum_len = 0;
      
      memset(accum,0,sizeof(accum));
      for(uint4 i = 0; i < source.length(); ++i)
      {
         if(isxdigit(source[i]))
            accum[accum_len++] = source[i];
         if(accum_len == 2)
         {
            encrypted.append(static_cast<byte>(strtoul(accum,0,16)));
            memset(accum,0,sizeof(accum));
            accum_len = 0;
         }
      }
      
      // we can now decrypt the buffer
      StrBin decrypted;
      encryptor.decrypt(decrypted, encrypted.getContents(), (uint4)encrypted.length());
      decrypted.append('\0');
      dest = decrypted.getContents();
      return dest.c_str();
   }
};

