/* Csi.LoginManager.h

   Copyright (C) 2005, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 26 January 2005
   Last Change: Friday 21 May 2021
   Last Commit: $Date: 2019-08-02 12:00:58 -0600 (Fri, 02 Aug 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_LoginManager_h
#define Csi_LoginManager_h

#include "CsiTypeDefs.h"
#include "StrAsc.h"
#include "StrUni.h"
#include "Csi.Utils.h"
#include "LoggerNetBuild.h"
#include <list>
#include <stdlib.h>


namespace Csi
{
   namespace LoginManagerHelpers
   {
      /**
       * Defines an object that keeps track of the values needed for a connection to a LoggerNet
       * server.
       */
      class Login
      {
      public:
         /**
          * Specifies the DNS name or IP address of the LoggerNet server and, optionally, that
          * server's port.
          */
         StrAsc server_name;

         /**
          * Specifies the user name used when connecting to the LoggerNet server.
          */
         StrUni user_name;

         /**
          * Specifies the password used when connecting to the LoggerNet server.
          */
         StrUni password;

         /**
          * Set to true if the user name and password should be remembered.
          */
         bool remembered;

         /**
          * Constructor
          *
          * @param server_name_ Specifies the DNS name or IP address and, optionally, the TCP port
          * of the LoggerNet server.
          *
          * @param user_name_ Specifies the name of the LoggerNet user account.
          *
          * @param password_ Specifies the password for the LoggerNet user account.
          *
          * @param remembered_ Set to true if the user name and password should be remembered.
          */
         Login(
            StrAsc const &server_name_ = "localhost",
            StrUni const &user_name_ = L"",
            StrUni const &password_ = L"",
            bool remembered_ = false):
            server_name(server_name_),
            user_name(user_name_),
            password(password_),
            remembered(remembered_)
         { }

         /**
          * Copy Constructor
          *
          * @param other Specifies the login information to copy.
          */
         Login(Login const &other):
            server_name(other.server_name),
            user_name(other.user_name),
            password(other.password),
            remembered(other.remembered)
         { }

         /**
          * Copy operator
          *
          * @param other Specifies the login information to copy.
          */
         Login &operator =(Login const &other)
         {
            server_name = other.server_name;
            user_name = other.user_name;
            password = other.password;
            remembered = other.remembered;
            return *this;
         }

         /**
          * @return Parses the server name and returns the address portion.
          */
         StrAsc get_server_address() const
         {
            StrAsc rtn;
            uint2 port(0);
            parse_uri_address(rtn, port, server_name);
            return rtn;
         }

         /**
          * @return Parses the server name and returns the TCP port to be used.
          */
         uint2 get_server_port() const
         {
            StrAsc rtn;
            uint2 port(0);
            parse_uri_address(rtn, port, server_name);
            if(port == 0)
               port = 6789;
            return port;
         }
      };
   };


   /**
    * Defines a component that manages the shared login information for various clients on the same
    * computer.  This component will read and write from a shared login.xml file stored within the
    * application's working directory.
    */
   class LoginManager
   {
   private:
      /**
       * Specifies the path to the login.xml file.
       */
      StrAsc login_file_path;

      /**
       * Specifies the last set of logins read from the file.
       */
   public:
      typedef LoginManagerHelpers::Login login_type;
      typedef std::list<login_type> logins_type;
   private:
      logins_type logins;

      /**
       * Set to true if the login should be automatic.
       */
      bool is_automatic;

      /**
       * Specifies the path to the ini file.
       */
      StrAsc ini_file_path;

      /**
       * Specifies the limit of the number of passwords that can be stored at any one time.
       */
      uint4 max_logins;

      /**
       * Set to true if this manager is locked.
       */
      bool locked;
      
   public:
      /**
       * Constructor
       *
       * @param app_name Specifies the name of the application.  This is used to resolve the working
       * direcotry.
       *
       * @param version Specifies the version of the application.  This is also used to resolve the
       * working directory.
       */
      LoginManager(
         StrAsc const &app_name = LoggerNet_name, StrAsc const &version = LoggerNet_version);

      /**
       * Destructor
       */
      virtual ~LoginManager()
      { }

      // @group: logins access methods

      /**
       * @return Returns an iterator referencing the first login.
       */
      typedef logins_type::const_iterator const_iterator;
      const_iterator begin() const
      { return logins.begin(); }

      /**
       * @return Returns an iterator referencing beyond the end of the last login.
       */
      const_iterator end() const
      { return logins.end(); }

      /**
       * @return Returns the number of logins stored.
       */
      logins_type::size_type size() const
      { return logins.size(); }

      /**
       * @return Returns true if there are no logins stored.
       */
      bool empty() const
      { return logins.empty(); }

      /**
       * @return Returns a reference to the first login.
       */
      login_type const &front() const
      { 
         if(logins.empty())
            throw std::logic_error("No logins are available");
         return logins.front();
      }
      
      // @endgroup:

      /**
       * Updates the last login used by storing the information at the first of the list.
       *
       * @param server_name Specifies the IP or DNS address of the server and, optionally, the
       * server's TCP port.
       *
       * @param login_name Specifies the account name used to connect to the LoggerNet server.
       *
       * @param password Specifies the password used to connect to the LoggerNet server.
       *
       * @param remembered Set to true if the user name and password should be remembered.
       *
       * @param is_automatic_ Set to true if this login should be used automatically in the future.
       */
      void set_last_login(
         StrAsc const &server_name,
         StrUni const &login_name,
         StrUni const &password,
         bool remembered,
         bool is_automatic_);

      /**
       * Rereads the logins from the file.
       */
      void refresh();

      /**
       * Saves the current set of logins to the file.
       */
      void save();

      /**
       * @return Returns the IP or DNS address and, optionally, the TCP port of the last selected
       * server (the login stored at the head of the queue).
       */
      StrAsc const &get_server_name() const
      {
         if(logins.empty())
            throw std::logic_error("No logins available");
         return logins.front().server_name;
      }

      /**
       * @return Returns the IP or DNS address of the last selected server (the login at the head of
       * the queue).
       */
      StrAsc get_server_address() const
      {
         if(logins.empty())
            throw std::logic_error("No logins available");
         return logins.front().get_server_address();
      }

      /**
       * @return Returns the TCP port of the last selected server (the login at the head of the
       * queue.
       */
      uint2 get_server_port() const
      {
         if(logins.empty())
            throw std::logic_error("No logins available");
         return logins.front().get_server_port();
      }

      /**
       * @return Returns the user name of the last selected server (the login at the head of the
       * queue.)
       */
      StrUni const &get_user_name() const
      {
         if(logins.empty())
            throw std::logic_error("No logins available");
         return logins.front().user_name;
      }

      /**
       * @return Returns the password of the last selected server (the login at the head of the
       * queue).
       */
      StrUni const &get_password() const
      {
         if(logins.empty())
            throw std::logic_error("No logins available");
         return logins.front().password;
      }

      /**
       * @return Returns true if the last selected server was supposed to remember login details.
       */
      bool get_remembered() const
      {
         if(logins.empty())
            throw std::logic_error("No logins available");
         return logins.front().remembered;
      }

      /**
       * @return Returns true if the login at the head of the queue should be automatically used.
       */
      bool get_is_automatic() const
      { return is_automatic; }

      /**
       * @return Returns the expected INI file path for the applocation based upon the selected
       * server and application name.
       */
      StrAsc get_ini_file_path();

      /**
       * Used internally to encrypt passwords that will be written to the login.xml file.
       *
       * @param dest Specifies the string to which the encrypted password will be stored in HEX
       * notation.
       *
       * @param source Specifies the Specifies the password to be encrypted.
       */
      static char const *encrypt_password(StrAsc &dest, StrUni const &source);

      /**
       * Used internally to decrypt passwords read from the login.xml file.
       *
       * @param dest Specifies the string to which the decrypted password will be written.
       *
       * @param source Specifies the encrypted password encoded in HEX format.
       */
      static char const *decrypt_password(StrAsc &dest, StrAsc const &source);
   };
};


#endif
