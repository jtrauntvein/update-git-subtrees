/* Cora.DataSources.DataFileSymbol.cpp

   Copyright (C) 2008, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 20 August 2008
   Last Change: Friday 14 February 2020
   Last Commit: $Date: 2020-02-14 09:53:04 -0600 (Fri, 14 Feb 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.DataFileSymbol.h"
#include "Cora.DataSources.DataFileSource.h"
#include "Cora.DataSources.SymbolBrowser.h"


namespace Cora
{
   namespace DataSources
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class ValueSymbol
         ////////////////////////////////////////////////////////////
         class ValueSymbol: public SymbolBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            typedef Cora::Broker::RecordDesc::value_type desc_handle;
            ValueSymbol(SymbolBase *parent, StrUni const &name,  desc_handle &desc_):
               SymbolBase(parent->get_source(), name, parent),
               desc(desc_)
            { }

            ////////////////////////////////////////////////////////////
            // get_symbol_type
            ////////////////////////////////////////////////////////////
            virtual symbol_type_code get_symbol_type() const
            { return type_scalar; }
            
            ////////////////////////////////////////////////////////////
            // can_expand
            ////////////////////////////////////////////////////////////
            virtual bool can_expand() const
            { return false; }
            
            ////////////////////////////////////////////////////////////
            // get_data_type
            ////////////////////////////////////////////////////////////
            virtual bool has_data_type() const
            { return true; }
            virtual CsiDbTypeCode get_data_type() const
            { return desc->data_type; }

            ////////////////////////////////////////////////////////////
            // get_units
            ////////////////////////////////////////////////////////////
            virtual bool has_units() const
            { return true; }
            virtual StrUni get_units() const
            { return desc->units; }

            ////////////////////////////////////////////////////////////
            // get_process
            ////////////////////////////////////////////////////////////
            virtual bool has_process() const
            { return true; }
            virtual StrUni get_process() const
            { return desc->process; }

            ////////////////////////////////////////////////////////////
            // get_description
            ////////////////////////////////////////////////////////////
            virtual bool has_description() const
            { return true; }
            virtual StrUni get_description() const
            { return desc->description; }

         private:
            ////////////////////////////////////////////////////////////
            // desc
            ////////////////////////////////////////////////////////////
            desc_handle desc;
         };

         
         ////////////////////////////////////////////////////////////
         // class TableSymbol
         ////////////////////////////////////////////////////////////
         class TableSymbol: public SymbolBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            typedef Cora::Broker::Record::desc_handle record_desc_handle;
            TableSymbol(
               DataFileSymbol *parent,
               StrUni const &name,
               uint4 array_id_,
               record_desc_handle &record_desc_):
               SymbolBase(parent->get_source(), name, parent),
               array_id(array_id_),
               record_desc(record_desc_)
            {
               // add symbols for all of the record values
               using namespace Cora::Broker;
               Csi::OStrAscStream value_name;
               for(RecordDesc::iterator vi = record_desc->begin(); vi != record_desc->end(); ++vi)
               {
                  RecordDesc::value_type &value = *vi;
                  if(!value->should_be_merged())
                  {
                     typedef std::vector<uint4> subscripts_type;
                     subscripts_type subscripts(value->array_address);
                     
                     value_name.str("");
                     value_name << value->name; 
                     if(value->data_type == CsiAscii && !subscripts.empty())
                        subscripts.pop_back();
                     if(!subscripts.empty())
                     {
                        value_name << "(";
                        for(subscripts_type::iterator si = subscripts.begin(); si != subscripts.end(); ++si)
                        {
                           if(si != subscripts.begin())
                              value_name << ",";
                           value_name << *si;
                        }
                        value_name << ")";
                     }
                     children.push_back(new ValueSymbol(this, value_name.str(), value));
                  }
               }
            } // constructor

            ////////////////////////////////////////////////////////////
            // get_symbol_type
            ////////////////////////////////////////////////////////////
            virtual symbol_type_code get_symbol_type() const
            { return type_table; }

            ////////////////////////////////////////////////////////////
            // can_expand
            ////////////////////////////////////////////////////////////
            virtual bool can_expand() const
            { return true; }

            ////////////////////////////////////////////////////////////
            // get_array_id
            ////////////////////////////////////////////////////////////
            uint4 const get_array_id() const
            { return array_id; }

         private:
            ////////////////////////////////////////////////////////////
            // array_id
            ////////////////////////////////////////////////////////////
            uint4 const array_id;

            ////////////////////////////////////////////////////////////
            // desc
            ////////////////////////////////////////////////////////////
            record_desc_handle record_desc;
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class DataFileSymbol definitions
      ////////////////////////////////////////////////////////////
      DataFileSymbol::DataFileSymbol(DataFileSource *source_):
         SymbolBase(source_, source_->get_name()),
         source(source_)
      { }


      DataFileSymbol::~DataFileSymbol()
      {
         children.clear();
      } // destructor


      bool DataFileSymbol::is_connected() const
      { return source->is_connected(); }


      bool DataFileSymbol::is_enabled() const
      { return source->is_started(); }


      void DataFileSymbol::start_expansion()
      {
         if(empty() && source->is_connected())
            reload_symbols();
      } // start_expansion


      void DataFileSymbol::reload_symbols()
      {
         using namespace Cora::Broker;
         DataFileSource::reader_handle reader(source->get_reader(false));
         if(reader != 0)
         {
            StrUni table_name;
            for(DataFileReader::iterator ti = reader->begin(); ti != reader->end(); ++ti)
            {
               table_name = ti->second->table_name;
               children.push_back(new TableSymbol(this, table_name, ti->first, ti->second));
               browser->send_symbol_added(children.back());
            }
            browser->send_expansion_complete(this);
         }
      } // reload_symbols
   };
};

