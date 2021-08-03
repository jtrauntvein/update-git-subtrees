/* Csi.Passwd.h

   Copyright (C) 2010, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 04 August 2010
   Last Change: Wednesday 07 October 2020
   Last Commit: $Date: 2020-10-07 09:36:09 -0600 (Wed, 07 Oct 2020) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_Passwd_h
#define Csi_Passwd_h

#include "StrAsc.h"
#include "Csi.LgrDate.h"
#include <deque>


namespace Csi
{
   namespace PasswdHelpers
   {
      /**
       * Lists the possible access levels that can be assigned.
       */
      enum access_level_type
      {
         access_none = 0,
         access_read_only = 3,
         access_read_write = 2,
         access_all = 1
      };
      
      
      /**
       * Defines an object that represents an account described in the .csipasswd format.
       */
      class Account
      {
      public:
         /**
          * Constructor
          *
          * @param name_ Specifies the name of the account.
          *
          * @param passwd_ Specifies the password assigned to the account
          *
          * @param access_level_ Specifies the access level assigned to the account.
          */
         Account(
            StrAsc const &name_ = "anonymous",
            StrAsc const &passwd_ = "",
            access_level_type access_level_ = access_read_only):
            name(name_),
            passwd(passwd_),
            access_level(access_level_)
         { }
         
         /**
          * Copy Constructor
          */
         Account(Account const &other):
            name(other.name),
            passwd(other.passwd),
            access_level(other.access_level)
         { }
         
         /**
          * Copy operator
          */
         Account &operator =(Account const &other)
         {
            name = other.name;
            passwd = other.passwd;
            access_level = other.access_level;
            return *this;
         }
         
         /**
          * @return Returns the account name.
          */
         StrAsc const &get_name() const
         { return name; }
         
         /**
          * @return Returns the account password.
          */
         StrAsc const &get_passwd() const
         { return passwd; }
         
         /**
          * @param passwd_ Sets the account password.
          */
         void set_passwd(StrAsc const &passwd_)
         { passwd = passwd_; }
         
         /**
          * @return Returns the access level.
          */
         access_level_type get_access_level() const
         { return access_level; }
         
         /**
          * @param access_level_ Specifies the assigned access level.
          */
         void set_access_level(access_level_type access_level_)
         { access_level = access_level_; }

         /**
          * @return Returns false if the lengths of the user name and password strings are longer
          * than the space allotted within the datalogger.
          */
         bool check_lengths() const;
         
      private:
         /**
          * Specifies the account name.
          */
         StrAsc name;
         
         /**
          * Specifies the account password.
          */
         StrAsc passwd;

         /**
          * Specifies the assigned access level.
          */
         access_level_type access_level;
      };
   };


   /**
    * Defines an object that can parse or generate a .csipasswd structure.
    */
   class Passwd
   {
   public:
      /**
       * Constructor
       *
       * @param file_name_ Specifies the name of the file to read.
       */
      Passwd(StrAsc const &file_name_);
      
      /**
       * Default constructor
       */
      Passwd();

      /**
       * Destructor
       */
      virtual ~Passwd();
      
      /**
       * @return Returns an iterator to the first of the accounts.
       */
      typedef PasswdHelpers::Account value_type;
      typedef std::deque<value_type> accounts_type;
      typedef accounts_type::iterator iterator;
      typedef accounts_type::const_iterator const_iterator;
      iterator begin()
      { return accounts.begin(); }
      const_iterator begin() const
      { return accounts.begin(); }
      
      /**
       * @return Returns the iterator beyond the end of the accounts.
       */
      iterator end()
      { return accounts.end(); }
      const_iterator end() const
      { return accounts.end(); }
      
      /**
       * @return Returns a reference to the first account.
       */
      value_type &front()
      { return accounts.front(); }
      value_type const &front() const
      { return accounts.front(); }
      
      /**
       * Adds an account to the beginning of the list.
       */
      void push_front(value_type const &value)
      { accounts.push_front(value); }
      
      /**
       * @return Returns a reference to the last account.
       */
      value_type &back()
      { return accounts.back(); }
      value_type const &back() const
      { return accounts.back(); }
      
      /**
       * @param value Specifies an account to add at the end of the list.
       */
      void push_back(value_type const &value)
      { accounts.push_back(value); }
      
      /**
       * @return Returns the number of accounts.
       */
      typedef accounts_type::size_type size_type;
      size_type size() const
      { return accounts.size(); }
      
      /**
       * @return Returns true if there are no accounts.
       */
      bool empty() const
      { return accounts.empty(); }
      
      /**
       * Removes all accounts.
       */
      void clear()
      { accounts.clear(); }
      
      /**
       * @param ai Specifies an iterator for an account to remove.
       */
      void erase(iterator ai)
      { accounts.erase(ai); }
      
      /**
       * @return Returns an iterator to a matching account or the value of end() if there is no
       * matching account.
       *
       * @param name Specifies the name of the account for which we will search.
       */
      iterator find(StrAsc const &name);
      const_iterator find(StrAsc const &name) const;

      /**
       * @return Returns the access level assigned to the specified account name assuming that the
       * provided password matches the account password.  If no match is found or the password fails
       * to match, a value of access_none will be returned.
       *
       * @param name Specifies the name of the account for which we will search.
       *
       * @param passwd Specifies the password that will need to be matched.
       */
      typedef PasswdHelpers::access_level_type access_level_type;
      access_level_type get_access_level(StrAsc const &name, StrAsc const &passwd) const;

      /**
       * Specifies the maximum length that the realm string can allowed to be.
       */
      static uint4 const realm_max_length;
      
      /**
       * @return Returns the assigned realm string.
       */
      StrAsc const &get_realm() const
      { return realm; }
      
      /**
       * @param realm_ Specifies the realm string.
       */
      void set_realm(StrAsc const &realm_);

      /**
       * Can be called by the constructor to read or reread the source file.  If this object was
       * originally parsed from a stream, this method will have no effect.
       *
       * @return Returns true if the content has been updated.
       *
       * @param ignore_last_write Set to true if the file should be read regardless of whether it
       * has been thought to have changed.
       */
      bool refresh(bool ignore_last_write = false);

      /**
       * Reads the format from a stream rather than from a file.
       *
       * @param in Specifies the source of the format.
       */
      bool refresh(std::istream &in);

      /**
       * Overwrites the source file with the current contents.
       */
      bool write() const;
      
      /**
       * Writes the format to the specified stream.
       */
      void write(std::ostream &out) const;

      /**
       * @return Returns true if the set of accounts being used are the default set.
       */
      bool get_using_default_accounts() const
      { return using_default_accounts; }
      
   private:
      /**
       * Specifies the list of accounts.
       */
      accounts_type accounts;
      
      /**
       * Specifies the realm string.
       */
      StrAsc realm;
      
      /**
       * Specifies the path to the source file.
       */
      StrAsc const file_name;
      
      /**
       * Specifies the time that the source file was last written.
       */
      Csi::LgrDate last_write;

      /**
       * Set to true if all of the accounts are the default set of accounts.
       */
      bool using_default_accounts;
   };
};


#endif
