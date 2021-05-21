/* Cora.LgrNet.DirectoryLister.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 22 October 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_DirectoryLister_h
#define Cora_LgrNet_DirectoryLister_h

#include <list>
#include "Cora.ClientBase.h"
#include "Csi.Events.h"
#include "Csi.LgrDate.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class DirectoryLister;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class DirectoryElement
      ////////////////////////////////////////////////////////////
      struct DirectoryElement
      {
         ////////////////////////////////////////////////////////////
         // name
         ////////////////////////////////////////////////////////////
         StrAsc name;

         ////////////////////////////////////////////////////////////
         // type_code
         ////////////////////////////////////////////////////////////
         enum type_code_type
         {
            type_directory = 1,
            type_file = 2,
         } type_code;

         ////////////////////////////////////////////////////////////
         // is_read_only
         ////////////////////////////////////////////////////////////
         bool is_read_only;
         
         ////////////////////////////////////////////////////////////
         // is_hidden
         ////////////////////////////////////////////////////////////
         bool is_hidden;

         ////////////////////////////////////////////////////////////
         // is_system
         ////////////////////////////////////////////////////////////
         bool is_system;

         ////////////////////////////////////////////////////////////
         // is_temporary
         ////////////////////////////////////////////////////////////
         bool is_temporary;

         ////////////////////////////////////////////////////////////
         // creation_time
         ////////////////////////////////////////////////////////////
         Csi::LgrDate creation_time;

         ////////////////////////////////////////////////////////////
         // last_access_time
         ////////////////////////////////////////////////////////////
         Csi::LgrDate last_access_time;

         ////////////////////////////////////////////////////////////
         // last_write_time
         ////////////////////////////////////////////////////////////
         Csi::LgrDate last_write_time;

         ////////////////////////////////////////////////////////////
         // file_size
         ////////////////////////////////////////////////////////////
         int8 file_size;

         ////////////////////////////////////////////////////////////
         // default constructor
         ////////////////////////////////////////////////////////////
         DirectoryElement():
            type_code(type_directory),
            is_read_only(false),
            is_hidden(false),
            is_system(false),
            is_temporary(false),
            file_size(0)
         {  }

         ////////////////////////////////////////////////////////////
         // copy constructor
         ////////////////////////////////////////////////////////////
         DirectoryElement(DirectoryElement const &other):
            name(other.name),
            type_code(other.type_code),
            is_read_only(other.is_read_only),
            is_hidden(other.is_hidden),
            is_system(other.is_system),
            is_temporary(other.is_temporary),
            creation_time(other.creation_time),
            last_access_time(other.last_access_time),
            last_write_time(other.last_write_time),
            file_size(other.file_size)
         { }

         ////////////////////////////////////////////////////////////
         // copy operator
         ////////////////////////////////////////////////////////////
         DirectoryElement &operator =(DirectoryElement const &other)
         {
            name = other.name;
            type_code = other.type_code;
            is_read_only = other.is_read_only;
            is_hidden = other.is_hidden;
            is_system = other.is_system;
            is_temporary = other.is_temporary;
            creation_time = other.creation_time;
            last_access_time = other.last_access_time;
            last_write_time = other.last_write_time;
            file_size = other.file_size;
            return *this; 
         }
      };

      
      ////////////////////////////////////////////////////////////
      // class DirectoryListerClient
      ////////////////////////////////////////////////////////////
      class DirectoryListerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         typedef std::list<DirectoryElement> elements_type;
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_session_broken = 3,
            outcome_server_security_blocked = 4,
            outcome_unsupported = 5,
            outcome_invalid_path = 6,
            outcome_path_specifies_file = 7,
         };
         virtual void on_complete(
            DirectoryLister *lister,
            outcome_type outcome,
            StrAsc const &full_path,
            elements_type const &elements) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class DirectoryLister
      //
      // This class defines a component that can be used to get the contents of
      // a directory on the LoggerNet server's file system.  In order to use
      // this component, an application must provide an object derived from
      // class DirectoryListerClient.  It should then create an instance of
      // this class, invoke the appropriate methods to set properties including
      // set_path() and set_list_path_parent(), and then invoke the start()
      // method.  When the server transaction is complete, the component will
      // call the client object's on_complete() method.  The parameters of this
      // method will describe the results of the transaction. 
      ////////////////////////////////////////////////////////////
      class DirectoryLister:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
         } state;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         DirectoryListerClient *client;

         ////////////////////////////////////////////////////////////
         // path
         ////////////////////////////////////////////////////////////
         StrAsc path;

         ////////////////////////////////////////////////////////////
         // list_path_parent
         ////////////////////////////////////////////////////////////
         bool list_path_parent;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         DirectoryLister():
            state(state_standby),
            client(0),
            list_path_parent(false)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~DirectoryLister()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // set_path
         ////////////////////////////////////////////////////////////
         void set_path(StrAsc const &path_)
         {
            if(state == state_standby)
               path = path_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_path
         ////////////////////////////////////////////////////////////
         StrAsc const &get_path() const
         { return path; }

         ////////////////////////////////////////////////////////////
         // set_list_path_parent
         ////////////////////////////////////////////////////////////
         void set_list_path_parent(bool list_path_parent_)
         {
            if(state == state_standby)
               list_path_parent = list_path_parent_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_list_path_parent
         ////////////////////////////////////////////////////////////
         bool get_list_path_parent() const
         { return list_path_parent; }

         ////////////////////////////////////////////////////////////
         // start (using uninitialised router)
         ////////////////////////////////////////////////////////////
         typedef DirectoryListerClient client_type;
         void start(
            client_type *client_,
            router_handle &router)
         {
            if(state == state_standby)
            {
               if(client_type::is_valid_instance(client_))
               {
                  client = client_;
                  state = state_delegate;
                  ClientBase::start(router);
               }
               else
                  throw std::invalid_argument("Invalid client pointer");
            }
            else
               throw exc_invalid_state(); 
         }


         ////////////////////////////////////////////////////////////
         // start (using another component's router)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            ClientBase *other_component)
         {
            if(state == state_standby)
            {
               if(client_type::is_valid_instance(client_))
               {
                  client = client_;
                  state = state_delegate;
                  ClientBase::start(other_component);
               }
               else
                  throw std::invalid_argument("Invalid client pointer");
            }
            else
               throw exc_invalid_state(); 
         }


         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         {
            state = state_standby;
            client = 0;
            ClientBase::finish();
         }

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);
         
         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure();
      };
   };
};


#endif
