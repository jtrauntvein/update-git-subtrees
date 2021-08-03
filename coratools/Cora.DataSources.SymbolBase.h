/* Cora.DataSources.SymbolBase.h

   Copyright (C) 2008, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 August 2008
   Last Change: Monday 03 October 2016
   Last Commit: $Date: 2016-10-17 17:02:49 -0600 (Mon, 17 Oct 2016) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_DataSources_SymbolBase_h
#define Cora_DataSources_SymbolBase_h

#include "StrUni.h"
#include "Csi.SharedPtr.h"
#include "CsiTypes.h"
#include <ostream>
#include <list>


namespace Cora
{
   ////////////////////////////////////////////////////////////
   // class ClientBase forward declaration
   //
   // This declaration provided for symbols that deal with the LoggerNet
   // server.
   //////////////////////////////////////////////////////////// 
   class ClientBase;

   
   namespace DataSources
   {
      ////////////////////////////////////////////////////////////
      // class SymbolBase
      //
      // Defines a "symbol" that can be used to view the data values, tables,
      // etc that are available from a given data source.  This class is
      // recursive meaning that it is able to act as a conatiner for "lower
      // level" symbols.
      ////////////////////////////////////////////////////////////
      class SourceBase;
      class SymbolBrowser;
      class SymbolBase
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SymbolBase(
            SourceBase *source_,
            StrUni const &name_,
            SymbolBase *parent_ = 0):
            source(source_),
            name(name_),
            parent(parent_),
            browser(0)
         {
            if(parent != 0)
               browser = parent->browser;
         } //  constructor

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SymbolBase()
         { children.clear(); }

         ////////////////////////////////////////////////////////////
         // get_source
         ////////////////////////////////////////////////////////////
         virtual SourceBase *get_source()
         { return source; }
         virtual SourceBase const *get_source() const
         { return source; }

         ////////////////////////////////////////////////////////////
         // set_source
         ////////////////////////////////////////////////////////////
         virtual void set_source(SourceBase *source_)
         { source = source_; }
         
         ////////////////////////////////////////////////////////////
         // get_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_name() const
         { return name; }

         ////////////////////////////////////////////////////////////
         // get_data_type
         //
         // Can be overloaded to return the data type for scalars and arrays
         ////////////////////////////////////////////////////////////
         virtual CsiDbTypeCode get_data_type() const
         { return CsiUnknown; }

         ////////////////////////////////////////////////////////////
         // has_data_type
         ////////////////////////////////////////////////////////////
         virtual bool has_data_type() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // get_units
         //
         // Can be overloaded to return the units string specified by the data
         // source. 
         ////////////////////////////////////////////////////////////
         virtual StrUni get_units() const
         { return L""; }

         ////////////////////////////////////////////////////////////
         // has_units
         ////////////////////////////////////////////////////////////
         virtual bool has_units() const
         { return false; }
         
         ////////////////////////////////////////////////////////////
         // get_process
         //
         // Can be overloaded to return a process string that was recorded by
         // the source.   
         ////////////////////////////////////////////////////////////
         virtual StrUni get_process() const
         { return L""; }

         ////////////////////////////////////////////////////////////
         // has_process
         //
         // Can be overloaded to indicate that this symbol has a process
         // string. 
         ////////////////////////////////////////////////////////////
         virtual bool has_process() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // get_description
         //
         // can be overloaded to return the user defined description for
         // columns.
         ////////////////////////////////////////////////////////////
         virtual StrUni get_description() const
         { return L""; }

         ////////////////////////////////////////////////////////////
         // virtual bool has_description
         ////////////////////////////////////////////////////////////
         virtual bool has_description() const
         { return false; }
         
         ////////////////////////////////////////////////////////////
         // get_parent
         ////////////////////////////////////////////////////////////
         virtual SymbolBase *get_parent() const
         { return parent; }

         ////////////////////////////////////////////////////////////
         // format_uri
         //
         // Formats the URI string that will locate this symbol uniquely in the
         // hierarchy represented by the data source manager.  
         ////////////////////////////////////////////////////////////
         virtual void format_uri(std::ostream &out) const
         {
            if(parent != 0)
            {
               parent->format_uri(out);
               if(parent->is_source_level())
                  out << ':';
               else
                  out << '.';
            }
            out << name;
         }
         virtual void format_uri(std::wostream &out) const
         {
            if(parent != 0)
            {
               parent->format_uri(out);
               if(parent->is_source_level())
                  out << L":";
               else
                  out << L".";
            }
            out << name;
         }

         ////////////////////////////////////////////////////////////
         // get_symbol_type
         ////////////////////////////////////////////////////////////
         enum symbol_type_code
         {
            type_lgrnet_source = 1,
            type_file_source = 2,
            type_db_source = 3,
            type_http_source = 9,
            type_virtual_source = 10,
            type_konect_source = 11,
            type_bmp5_source = 12,
            type_station = 4,
            type_statistics_broker = 5,
            type_table = 6,
            type_array = 7,
            type_scalar = 8
         };
         virtual symbol_type_code get_symbol_type() const = 0;
         
         ////////////////////////////////////////////////////////////
         // is_source_level
         //
         // Can be overloaded by to return true for symbols that are "source
         // level" symbols.  The default definitoion, which examines the value
         // of the parent pointer, should be sufficient for most, if not all,
         // purposes. 
         ////////////////////////////////////////////////////////////
         virtual bool is_source_level() const
         { return parent == 0; }

         ////////////////////////////////////////////////////////////
         // get_loggernet_component
         //
         // Must be overloaded for symbols that deal with LoggerNet server
         // objects to return the base component
         ////////////////////////////////////////////////////////////
         virtual Cora::ClientBase *get_loggernet_component()
         { return 0; }

         ////////////////////////////////////////////////////////////
         // is_connected
         //
         // Can be called by the application to determin whether the symbol is "connected" to its
         // data source.  The meaning of this varies according to the type of the associated data
         // source.  With LoggerNet symbols, this will indicate the connection state of the
         // loggernet symbol browser. 
         ////////////////////////////////////////////////////////////
         virtual bool is_connected() const
         {
            bool rtn = false;
            if(parent)
               rtn = parent->is_connected();
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // is_enabled
         //
         // Can be called by the application to determine whether this symbol is "enabled" for
         // collection.  This meaning of this state is dependent on the data source with which this
         // symbol is associated.  With LoggerNet symbols, for instance, this state will indicate
         // whether the symbol (or its parent) is enabled for scheduled collection.
         ////////////////////////////////////////////////////////////
         virtual bool is_enabled() const
         {
            bool rtn = false;
            if(parent)
               rtn = parent->is_enabled();
            return rtn; 
         }

         ////////////////////////////////////////////////////////////
         // is_read_only
         //
         // Can be overloaded to indicate that this value associated with this
         // symbol can be changed.  
         ////////////////////////////////////////////////////////////
         virtual bool is_read_only() const
         { return true; }

         ////////////////////////////////////////////////////////////
         // can_expand
         //
         // Must be to indicate whether this symbol is a parent to lower level
         // symbols.  Note that this condition can exist even if the children
         // container is empty due to this symbol not having been previously
         // expanded by the application.  
         ////////////////////////////////////////////////////////////
         virtual bool can_expand() const = 0;

         ////////////////////////////////////////////////////////////
         // start_expansion
         //
         // Must be overloaded by symbols that support expansion to begin the
         // expansion process if it has not already been started.  
         ////////////////////////////////////////////////////////////
         virtual void start_expansion()
         { }

         ////////////////////////////////////////////////////////////
         // refresh
         //
         // can be called to determine if new symbols are available beneath the
         // level of this symbol.  Some data source types, like LgrNet, will
         // maintain an advise transaction to list symbols but others,
         // particularly the database related symbols, need to be polled to
         // refresh.
         ////////////////////////////////////////////////////////////
         virtual void refresh()
         { }

         // @group: act as a container for other symbols

         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<SymbolBase> value_type;
         typedef std::list<value_type> children_type;
         typedef children_type::iterator iterator;
         typedef children_type::const_iterator const_iterator;
         iterator begin()
         { return children.begin(); }
         const_iterator begin() const
         { return children.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end()
         { return children.end(); }
         const_iterator end() const
         { return children.end(); }

         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         typedef children_type::size_type size_type;
         size_type size() const
         { return children.size(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return children.empty(); }

         ////////////////////////////////////////////////////////////
         // find
         ////////////////////////////////////////////////////////////
         iterator find(StrUni const &name);
         const_iterator find(StrUni const &name) const;

         ////////////////////////////////////////////////////////////
         // push_back
         ////////////////////////////////////////////////////////////
         void push_back(value_type value)
         { children.push_back(value); }
         
         // @endgroup

         ////////////////////////////////////////////////////////////
         // set_browser
         ////////////////////////////////////////////////////////////
         void set_browser(SymbolBrowser *browser_);

         ////////////////////////////////////////////////////////////
         // get_browser
         ////////////////////////////////////////////////////////////
         SymbolBrowser *get_browser()
         { return browser; }

         ////////////////////////////////////////////////////////////
         // predicate symbol_has_name
         ////////////////////////////////////////////////////////////
         struct symbol_has_name
         {
            StrUni const &name;
            symbol_has_name(StrUni const &name_):
               name(name_)
            { }

            bool operator ()(value_type const &symbol) const
            { return symbol->get_name() == name; }
         };

         ////////////////////////////////////////////////////////////
         // has_parent
         ////////////////////////////////////////////////////////////
         bool has_parent(SymbolBase *other)
         {
            bool rtn = false;
            SymbolBase *candidate = this;
            while(!rtn && candidate != 0)
            {
               rtn = (candidate == other);
               candidate = candidate->parent;
            }
            return rtn;
         }
               
         ////////////////////////////////////////////////////////////
         // source
         ////////////////////////////////////////////////////////////
         SourceBase *source;

         ////////////////////////////////////////////////////////////
         // name
         ////////////////////////////////////////////////////////////
         StrUni name;

         ////////////////////////////////////////////////////////////
         // parent
         ////////////////////////////////////////////////////////////
         SymbolBase *parent;

         ////////////////////////////////////////////////////////////
         // children
         ////////////////////////////////////////////////////////////
         children_type children;

         ////////////////////////////////////////////////////////////
         // browser
         ////////////////////////////////////////////////////////////
         SymbolBrowser *browser;
      };


      ////////////////////////////////////////////////////////////
      // predicate symbol_name_less
      ////////////////////////////////////////////////////////////
      struct symbol_name_less
      {
         bool operator ()(SymbolBase::value_type const &s1, SymbolBase::value_type const &s2)
         { return s1->name < s2->name; } 
      };
   };
};


#endif
