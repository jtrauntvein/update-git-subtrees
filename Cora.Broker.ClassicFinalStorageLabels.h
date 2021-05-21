/* Cora.Broker.ClassicFinalStorageLabels.h

   Copyright (C) 2000, 2006 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 20 December 2006
   Last Change: Wednesday 20 December 2006
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ 
   Committed by: $Author: tmecham $
   
*/

#ifndef Cora_Broker_ClassicFinalStorageLabels_h
#define Cora_Broker_ClassicFinalStorageLabels_h

#include "Csi.SharedPtr.h"
#include "CsiTypeDefs.h"
#include <map>


namespace Cora
{
   namespace Broker
   {
      //@group class forward declarations
      class ClassicFinalStorageTable;
      //@endgroup
      

      ////////////////////////////////////////////////////////////
      // class ClassicFinalStorageLabels
      //
      // Contains all of the final storage labels specifications that are embedded in a DLD file
      //////////////////////////////////////////////////////////// 
      class ClassicFinalStorageLabels
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         ClassicFinalStorageLabels();

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         ~ClassicFinalStorageLabels();

         //@group tables iterator access methods
         typedef Csi::SharedPtr<ClassicFinalStorageTable> value_type;
         typedef std::map<uint4, value_type> tables_type;
         typedef tables_type::iterator iterator;
         typedef tables_type::const_iterator const_iterator;
         typedef tables_type::reverse_iterator reverse_iterator;
         typedef tables_type::const_reverse_iterator const_reverse_iterator;
         iterator begin() { return tables.begin(); }
         const_iterator begin() const { return tables.begin(); }
         reverse_iterator rbegin() { return tables.rbegin(); }
         const_reverse_iterator rbegin() const { return tables.rbegin(); }
         iterator end() { return tables.end(); }
         const_iterator end() const { return tables.end(); }
         reverse_iterator rend() { return tables.rend(); }
         const_reverse_iterator rend() const { return tables.rend(); }
         iterator find(uint4 key) { return tables.find(key); }
         const_iterator find(uint4 key) const { return tables.find(key); }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // extract_from_dld
         //
         // Extracts all final storage labels from the specified DLD stream
         //////////////////////////////////////////////////////////// 
         void extract_from_dld(std::istream &in);

         ////////////////////////////////////////////////////////////
         // extract_from_fsl
         //
         // Extracts all final stroage labels from the specified FSL stream
         //////////////////////////////////////////////////////////// 
         void extract_from_fsl(std::istream &in);
         
      private:
         ////////////////////////////////////////////////////////////
         // tables
         //
         // The list of tables known to this label set keyed by array id
         //////////////////////////////////////////////////////////// 
         tables_type tables;
      };
   };
};

#endif
