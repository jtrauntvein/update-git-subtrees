/* Csi.Passwd.cpp

   Copyright (C) 2010, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 04 August 2010
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Passwd.h"
#include "Csi.FileSystemObject.h"
#include "Csi.Utils.h"
#include "Csi.Base64.h"
#include "Csi.fstream.h"
#include <algorithm>


namespace Csi
{
   namespace PasswdHelpers
   {
      bool Account::check_lengths() const
      {
         bool rtn(true);
         if(name.length() + (4 * passwd.length() / 3) > Passwd::realm_max_length - 3)
            rtn = false;
         return rtn;
      } // check_lengths
   };

   
   ////////////////////////////////////////////////////////////
   // class Passwd definitions
   ////////////////////////////////////////////////////////////
   uint4 const Passwd::realm_max_length = 61;

   
   Passwd::Passwd(StrAsc const &file_name_):
      file_name(file_name_),
      realm("Default Realm"),
      using_default_accounts(true)
   { refresh(true); }


   Passwd::Passwd():
      realm("Default Realm"),
      using_default_accounts(true)
   {
      accounts.push_back(value_type());
      accounts.push_back(value_type("admin", "admin", PasswdHelpers::access_all));
   } // default constructor


   Passwd::~Passwd()
   { accounts.clear(); }


   namespace
   {
      ////////////////////////////////////////////////////////////
      // predicate account_has_name
      ////////////////////////////////////////////////////////////
      struct account_has_name
      {
         StrAsc const &name;
         account_has_name(StrAsc const &name_):
            name(name_)
         { }
         
         bool operator ()(PasswdHelpers::Account const &account) const
         { return name == account.get_name(); }
      };
      
   };

   
   Passwd::iterator Passwd::find(StrAsc const &name)
   {
      iterator rtn(
         std::find_if(begin(), end(), account_has_name(name)));
      return rtn;
   } // find


   Passwd::const_iterator Passwd::find(StrAsc const &name) const
   {
      const_iterator rtn(
         std::find_if(begin(), end(), account_has_name(name)));
      return rtn;
   } // find (const version)


   PasswdHelpers::access_level_type Passwd::get_access_level(
      StrAsc const &name, StrAsc const &passwd) const
   {
      using namespace PasswdHelpers;
      access_level_type rtn(access_none);
      const_iterator ai(end());
      if(name.length() == 0)
         ai = find("anonymous");
      else
         ai = find(name);
      if(ai != end() && ai->get_passwd() == passwd)
         rtn = ai->get_access_level();
      return rtn;
   } // get_access_level


   void Passwd::set_realm(StrAsc const &realm_)
   {
      // we want the realm to remain all on one line so we will trim out and line feeds
      realm = realm_;
      for(size_t i = 0; i < realm.length(); ++i)
      {
         if(realm[i] == '\n' || realm[i] == '\r')
            realm[i] = ' ';
      }
      if(realm.length() >= 64)
         realm.cut(63);
   } // set_realm


   bool Passwd::refresh(bool ignore_last_write)
   {
      // we need to determine whether the file has been updated since last we looked.
      bool needs_refresh(ignore_last_write);
      Csi::FileSystemObject file_info(file_name.c_str());
      bool rtn(false);
      
      if(file_info.get_is_valid())
      {
         if(ignore_last_write || last_write != file_info.get_last_write_date())
         {
            // we are now ready to read the file
            Csi::ifstream in(file_name.c_str(), std::ios::binary);
            rtn = refresh(in);
         }
      }
      else
      {
         using_default_accounts = true;
         accounts.clear();
         accounts.push_back(value_type());
         accounts.push_back(value_type("admin", "admin", PasswdHelpers::access_all));
      }
      if(rtn)
         last_write = file_info.get_last_write_date();
      return rtn;
   } // refresh


   bool Passwd::refresh(std::istream &in)
   {
      bool rtn = false;
      enum state_type
      {
         state_in_realm,
         state_before_record,
         state_in_comment,
         state_read_user_name,
         state_read_passwd,
         state_read_access,
         state_complete
      } state(state_in_realm);
      StrAsc user_name;
      StrAsc passwd;
      StrAsc access;
      char ch;
      bool read_next(true);
      
      realm.cut(0);
      accounts.clear();
      while(state != state_complete && in)
      {
         // read the next character from the file
         if(read_next)
         {
            in.get(ch);
            if(in.eof())
               state = state_complete;
         }
         else
            read_next = true;
         
         // now decide what to do with it
         switch(state)
         {
         case state_in_realm:
            if(ch == '\r' || ch == '\n')
               state = state_before_record;
            else
               realm.append(ch);
            break;
            
         case state_before_record:
            if(!isspace(ch) && ch != '#')
            {
               user_name.append(ch);
               state = state_read_user_name;
            }
            else if(ch == '#')
               state = state_in_comment;
            break;
            
         case state_in_comment:
            if(ch == '\n' || ch == '\r')
               state = state_before_record;
            break;
            
         case state_read_user_name:
            if(ch == ':')
               state = state_read_passwd;
            else if(ch != '\r' && ch != '\n')
               user_name.append(ch);
            else
            {
               user_name.cut(0);
               state = state_before_record;
            }
            break;
            
         case state_read_passwd:
            if(ch == ':')
               state = state_read_access;
            else if(ch != '\r' && ch != '\n')
               passwd.append(ch);
            else
            {
               access = "3";
               read_next = false;
               state = state_read_access;
            }
            break;
            
         case state_read_access:
            if(isdigit(ch))
               access.append(ch);
            else if(isspace(ch))
            {
               // we have now accumulated the information for one record.  We will now verify
               // that information and add it to the list of accounts
               StrBin decrypted_passwd;
               uint4 access_level_value(strtoul(access.c_str(), 0, 10));
               StrBin decoded_passwd;
               
               Csi::Base64::decode(decoded_passwd, passwd.c_str(), passwd.length());
               Csi::decrypt_sig(decrypted_passwd, decoded_passwd.getContents(), (uint4)decoded_passwd.length());
               decrypted_passwd.append(0);
               if(access_level_value >= 0 && access_level_value <= 3)
               {
                  accounts.push_back(
                     value_type(
                        user_name,
                        decrypted_passwd.getContents(),
                        static_cast<access_level_type>(access_level_value)));
               }
               
               // clear the accumulators and reset the state
               user_name.cut(0);
               passwd.cut(0);
               access.cut(0);
               state = state_before_record;
            }
            else
            {
               user_name.cut(0);
               passwd.cut(0);
               access.cut(0);
               state = state_before_record;
            }
            break;
            
         case state_complete:
            rtn = true;
            break;
         }
      }

      // the final step is to ensure that the anonymous account exists and that there is an
      // administrator account created.
      bool has_anonymous = false;
      bool has_admin = false;
      for(const_iterator ai = accounts.begin(); ai != accounts.end(); ++ai)
      {
         value_type const &account(*ai);
         if(!has_anonymous && account.get_name() == "anonymous")
            has_anonymous = true;
         if(account.get_access_level() == PasswdHelpers::access_all)
            has_admin = true;
      }
      if(!has_anonymous)
         accounts.push_front(value_type());
      if(!has_admin)
         accounts.push_back(value_type("admin", "admin", PasswdHelpers::access_all));
      using_default_accounts = (!has_anonymous || !has_admin);
      if(realm.length() == 0)
         realm = "Default Realm";
      return rtn;
   } // refresh (from stream)


   bool Passwd::write() const
   {
      bool rtn = false;
      Csi::ofstream out(file_name.c_str(), std::ios::binary);
      if(out)
      {
         write(out);
         rtn = true;
      }
      return rtn;
   } // write (cached file name)


   void Passwd::write(std::ostream &out) const
   {
      StrBin encrypted_passwd;
      out << realm << "\r\n";
      for(const_iterator ai = begin(); ai != end(); ++ai)
      {
         out << ai->get_name() << ":";
         encrypted_passwd.cut(0);
         Csi::encrypt_sig(encrypted_passwd, ai->get_passwd().c_str(), (uint4)ai->get_passwd().length());
         Csi::Base64::encode(out, encrypted_passwd.getContents(), encrypted_passwd.length());
         out << ":" << ai->get_access_level() << "\r\n";
      }
   } // write (to stream)
};

