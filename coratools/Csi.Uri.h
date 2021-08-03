/* Csi.Uri.h

   Copyright (C) 2007, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 23 February 2007
   Last Change: Wednesday 02 October 2019
   Last Commit: $Date: 2019-10-02 17:21:24 -0600 (Wed, 02 Oct 2019) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_Uri_h
#define Csi_Uri_h

#include "StrAsc.h"
#include "CsiTypeDefs.h"
#include <list>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class Uri
   //
   // Defines a class that is able to parse a well formed URI into its
   // components and is also able to build the URI string from its components.
   // Components include the protocol, server address, anchor name, and
   // arguments list.
   ////////////////////////////////////////////////////////////
   class Uri
   {
   public:
      ////////////////////////////////////////////////////////////
      // default constructor
      ////////////////////////////////////////////////////////////
      Uri();

      ////////////////////////////////////////////////////////////
      // string constructor
      ////////////////////////////////////////////////////////////
      Uri(StrAsc const &s);

      ////////////////////////////////////////////////////////////
      // copy constructor
      ////////////////////////////////////////////////////////////
      Uri(Uri const &other);

      ////////////////////////////////////////////////////////////
      // copy operator
      ////////////////////////////////////////////////////////////
      Uri &operator =(Uri const &other);

      ////////////////////////////////////////////////////////////
      // copy string operator
      ////////////////////////////////////////////////////////////
      Uri &operator =(StrAsc const &s); 

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      ~Uri();

      ////////////////////////////////////////////////////////////
      // get_protocol
      ////////////////////////////////////////////////////////////
      StrAsc const &get_protocol() const
      { return protocol; }

      ////////////////////////////////////////////////////////////
      // set_protocol
      ////////////////////////////////////////////////////////////
      void set_protocol(StrAsc const &protocol_)
      { protocol = protocol_; }

      ////////////////////////////////////////////////////////////
      // get_server_address
      ////////////////////////////////////////////////////////////
      StrAsc const &get_server_address() const
      { return server_address; }

      ////////////////////////////////////////////////////////////
      // set_server_address
      ////////////////////////////////////////////////////////////
      void set_server_address(StrAsc const &server_address_)
      { server_address = server_address_; }

      ////////////////////////////////////////////////////////////
      // get_path
      ////////////////////////////////////////////////////////////
      StrAsc const &get_path() const
      { return path; }

      ////////////////////////////////////////////////////////////
      // get_path_names
      //
      // Separates the path into the names given in the path.
      ////////////////////////////////////////////////////////////
      template <class container>
      void get_path_names(container &names) const
      {
         size_t first_slash_pos = path.find("/");
         StrAsc name;
         
         names.clear();
         if(first_slash_pos >= path.length() && path.length() > 0)
            names.push_back(path);
         if(first_slash_pos < path.length() && first_slash_pos > 0)
         {
            path.sub(name, 0, first_slash_pos);
            names.push_back(name);
         }
         while(first_slash_pos < path.length())
         {
            size_t second_slash_pos = path.find("/", first_slash_pos + 1);
            path.sub(name, first_slash_pos + 1, second_slash_pos - first_slash_pos - 1);
            names.push_back(name);
            first_slash_pos = second_slash_pos;
         }
      }

      ////////////////////////////////////////////////////////////
      // set_path
      ////////////////////////////////////////////////////////////
      void set_path(StrAsc const &path_)
      { path = path_; }

      ////////////////////////////////////////////////////////////
      // get_server_port
      ////////////////////////////////////////////////////////////
      uint2 get_server_port() const
      { return server_port; }

      ////////////////////////////////////////////////////////////
      // set_server_port
      ////////////////////////////////////////////////////////////
      void set_server_port(uint2 server_port_)
      { server_port = server_port_; }

      ////////////////////////////////////////////////////////////
      // get_anchor
      ////////////////////////////////////////////////////////////
      StrAsc const &get_anchor() const
      { return anchor; }

      ////////////////////////////////////////////////////////////
      // set_anchor
      ////////////////////////////////////////////////////////////
      void set_anchor(StrAsc const &anchor_)
      { anchor = anchor_; }

      // @group the following methods allow this class to act as a container
      // for the CGI parameters

      ////////////////////////////////////////////////////////////
      // begin
      ////////////////////////////////////////////////////////////
      typedef std::pair<StrAsc, StrAsc> value_type;
      typedef std::list<value_type> params_type;
      typedef params_type::iterator iterator;
      typedef params_type::const_iterator const_iterator;
      iterator begin()
      { return params.begin(); }
      const_iterator begin() const
      { return params.begin(); }

      ////////////////////////////////////////////////////////////
      // end
      ////////////////////////////////////////////////////////////
      iterator end()
      { return params.end(); }
      const_iterator end() const
      { return params.end(); }

      ////////////////////////////////////////////////////////////
      // insert
      ////////////////////////////////////////////////////////////
      void insert(StrAsc const &key, StrAsc const &value)
      { params.push_back(value_type(key,value)); }

      ////////////////////////////////////////////////////////////
      // front
      ////////////////////////////////////////////////////////////
      value_type &front()
      { return params.front(); }
      value_type const &front() const
      { return params.front(); }

      ////////////////////////////////////////////////////////////
      // back
      ////////////////////////////////////////////////////////////
      value_type &back()
      { return params.back(); }
      value_type const &back() const
      { return params.back(); }

      ////////////////////////////////////////////////////////////
      // empty
      ////////////////////////////////////////////////////////////
      bool empty() const
      { return params.empty(); }

      ////////////////////////////////////////////////////////////
      // size
      ////////////////////////////////////////////////////////////
      typedef params_type::size_type size_type;
      size_type size() const
      { return params.size(); }

      /**
       * Removes all parameters.
       */
      void clear()
      { params.clear(); }
      
      // @endgroup

      ////////////////////////////////////////////////////////////
      // parse
      ////////////////////////////////////////////////////////////
      void parse(StrAsc const &);

      ////////////////////////////////////////////////////////////
      // build
      ////////////////////////////////////////////////////////////
      StrAsc build();

      ////////////////////////////////////////////////////////////
      // has_param
      //
      // Determines whether the specified parameter is named as a parameter
      ////////////////////////////////////////////////////////////
      bool has_param(StrAsc const &name, StrAsc *value = 0) const;
      
      ////////////////////////////////////////////////////////////
      // get_param_value
      ////////////////////////////////////////////////////////////
      StrAsc get_param_value(StrAsc const &name) const;

      /**
       * Formats the URI to the specified stream.
       *
       * @param out Specifies the stream to write.
       *
       * @param address_only Set to true if only the protocol and address should be formatted.
       */
      void format(std::ostream &out, bool address_only = false) const;
      void format(std::wostream &out, bool address_only = false) const;

      ////////////////////////////////////////////////////////////
      // encode
      //
      // Responsible for encoding the provided string as a part of the URI
      ////////////////////////////////////////////////////////////
      static void encode(
         std::ostream &out,
         StrAsc const &s);

      ////////////////////////////////////////////////////////////
      // decode
      //
      // Inverse of the encode() function.  
      ////////////////////////////////////////////////////////////
      static StrAsc decode(StrAsc const &s);

   private:
      ////////////////////////////////////////////////////////////
      // protocol
      ////////////////////////////////////////////////////////////
      StrAsc protocol;

      ////////////////////////////////////////////////////////////
      // server_address
      ////////////////////////////////////////////////////////////
      StrAsc server_address;

      ////////////////////////////////////////////////////////////
      // server_port
      ////////////////////////////////////////////////////////////
      uint2 server_port;

      ////////////////////////////////////////////////////////////
      // path
      ////////////////////////////////////////////////////////////
      StrAsc path;

      ////////////////////////////////////////////////////////////
      // anchor
      ////////////////////////////////////////////////////////////
      StrAsc anchor;

      ////////////////////////////////////////////////////////////
      // params
      ////////////////////////////////////////////////////////////
      params_type params;
   };
};


#endif