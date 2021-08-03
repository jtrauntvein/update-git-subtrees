/* Cora.Broker.ClassicFinalStorageTable.h

   Copyright (C) 2000, 2006 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 07 March 2000
   Last Change: Thursday 01 June 2006
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ 
   Committed by: $Author: tmecham $
   
*/

#ifndef Cora_Broker_ClassicFinalStorageTable_h
#define Cora_Broker_ClassicFinalStorageTable_h

#include <list>
#include "Cora.Broker.RecordDesc.h"


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class ClassicFinalStorageTable
      //
      // Describes a table (output array) in a classic datalogger's final
      // storage. Since input tables area not accessible to the computer, we
      // will only concern ourseleves with tables that are marked for output
      //////////////////////////////////////////////////////////// 
      class ClassicFinalStorageTable
      {
      public:
         ////////////////////////////////////////////////////////////
         // default constructor, copy constructor
         //////////////////////////////////////////////////////////// 
         ClassicFinalStorageTable();
         ClassicFinalStorageTable(ClassicFinalStorageTable const &other);

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~ClassicFinalStorageTable();

         ////////////////////////////////////////////////////////////
         // copy operator
         //////////////////////////////////////////////////////////// 
         ClassicFinalStorageTable &operator =(ClassicFinalStorageTable const &other);

         //@group access methods
         ////////////////////////////////////////////////////////////
         // get_name
         //////////////////////////////////////////////////////////// 
         StrUni const &get_name() const { return name; }

         ////////////////////////////////////////////////////////////
         // get_array_id
         //////////////////////////////////////////////////////////// 
         uint4 get_array_id() const { return array_id; }

         ////////////////////////////////////////////////////////////
         // extract
         //////////////////////////////////////////////////////////// 
         bool extract(std::istream &in);

         ////////////////////////////////////////////////////////////
         // make_table_def
         //
         // Creates a table definition object based on what was extracted
         //////////////////////////////////////////////////////////// 
         Csi::SharedPtr<RecordDesc> make_table_def();

         //@group column names iteration
         typedef std::list<StrUni> column_names_type;
         typedef column_names_type::iterator iterator;
         typedef column_names_type::const_iterator const_iterator;
         typedef column_names_type::reverse_iterator reverse_iterator;
         typedef column_names_type::const_reverse_iterator const_reverse_iterator;
         iterator begin() { return column_names.begin(); }
         const_iterator begin() const { return column_names.end(); }
         reverse_iterator rbegin() { return column_names.rbegin(); }
         const_reverse_iterator rbegin() const { return column_names.rbegin(); }
         iterator end() { return column_names.end(); }
         const_iterator end() const { return column_names.end(); }
         reverse_iterator rend() { return column_names.rend(); }
         const_reverse_iterator rend() const { return column_names.rend(); }
         column_names_type::size_type size() const { return column_names.size(); }
         //@endgroup
         //@endgroup

      protected:
         ////////////////////////////////////////////////////////////
         // name
         //////////////////////////////////////////////////////////// 
         StrUni name;

         ////////////////////////////////////////////////////////////
         // array_id
         //////////////////////////////////////////////////////////// 
         uint4 array_id;

         ////////////////////////////////////////////////////////////
         // column_names
         //////////////////////////////////////////////////////////// 
         column_names_type column_names;
      };
   };
};

#endif
