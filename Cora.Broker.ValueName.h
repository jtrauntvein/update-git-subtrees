/* Cora.Broker.ValueName.h

   Copyright (C) 2003, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 08 January 2003
   Last Change: Friday 14 March 2014
   Last Commit: $Date: 2014-03-14 09:55:32 -0600 (Fri, 14 Mar 2014) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Broker_ValueName_h
#define Cora_Broker_ValueName_h

#include "StrUni.h"
#include "CsiTypeDefs.h"
#include <vector>
#include <stdexcept>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class ValueName
      //
      // Defines a concrete class that can be used to help interpret a value name string.  A value
      // name string should conform to the following syntax:
      //
      //  name-string := [ [ station-name "." ] table-name "." ]
      //                 column-name [ "(" subscript { "," subscript } ")" ].
      //
      // The station name and table name considered optional because some uses of this class might
      // not consider this level of identification.  
      ////////////////////////////////////////////////////////////
      class ValueName
      {
      public:
         ////////////////////////////////////////////////////////////
         // class exc_invalid_syntax
         //
         // Specifies the type of exception that will be thrown from construction attempts using a
         // string with invalid syntax.
         ////////////////////////////////////////////////////////////
         class exc_invalid_syntax: public std::exception
         {
         public:
            ////////////////////////////////////////////////////////////
            // what
            ////////////////////////////////////////////////////////////
            virtual char const *what() const throw ()
            { return "Invalid value name syntax"; }
         };

         
         ////////////////////////////////////////////////////////////
         // default constructor
         ////////////////////////////////////////////////////////////
         ValueName();

         ////////////////////////////////////////////////////////////
         // string constructors
         //
         // Will attempt to parse the value name from the specified strings.
         ////////////////////////////////////////////////////////////
         ValueName(char const *s);
         ValueName(wchar_t const *s);
         ValueName(StrUni const &s);

         ////////////////////////////////////////////////////////////
         // copy constructor
         ////////////////////////////////////////////////////////////
         ValueName(ValueName const &other);

         ////////////////////////////////////////////////////////////
         // copy operator
         ////////////////////////////////////////////////////////////
         ValueName &operator =(ValueName const &other);
         
         ////////////////////////////////////////////////////////////
         // string copy operator
         ////////////////////////////////////////////////////////////
         ValueName &operator =(char const *s);
         ValueName &operator =(wchar_t const *s);
         ValueName &operator =(StrUni const &s);

         ////////////////////////////////////////////////////////////
         // get_full_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_full_name() const
         { return full_name; }

         ////////////////////////////////////////////////////////////
         // get_station_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_station_name() const
         { return station_name; }

         ////////////////////////////////////////////////////////////
         // get_table_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_table_name() const
         { return table_name; }

         ////////////////////////////////////////////////////////////
         // get_full_column_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_full_column_name() const
         { return full_column_name; }

         ////////////////////////////////////////////////////////////
         // get_column_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_column_name() const
         { return column_name; }

         //@group subscripts access members
         ////////////////////////////////////////////////////////////
         // begin
         //////////////////////////////////////////////////////////// 
         typedef std::vector<uint4> subscripts_type;
         typedef subscripts_type::iterator iterator;
         typedef subscripts_type::const_iterator const_iterator;
         iterator begin() { return subscripts.begin(); }
         const_iterator begin() const { return subscripts.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end() { return subscripts.end(); }
         const_iterator end() const { return subscripts.end(); }

         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         subscripts_type::size_type size() const { return subscripts.size(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const { return subscripts.empty(); }

         ////////////////////////////////////////////////////////////
         // get_subscripts
         ////////////////////////////////////////////////////////////
         subscripts_type &get_subscripts()
         { return subscripts; }
         subscripts_type const &get_subscripts() const
         { return subscripts; }

         ////////////////////////////////////////////////////////////
         // push_back
         ////////////////////////////////////////////////////////////
         void push_back(uint4 subscript)
         { subscripts.push_back(subscript); }
         
         //@endgroup

         ////////////////////////////////////////////////////////////
         // compare
         //
         // Compares this value name with another ValueName object and returns an integer coded to
         // mean the following:
         //
         //   rtn <  0 => this < other
         //   rtn == 0 => this == other
         //   rtn >  0 => this > other
         ////////////////////////////////////////////////////////////
         int compare(ValueName const &other) const;

         //@group comparison operators
         ////////////////////////////////////////////////////////////
         // equality operator
         ////////////////////////////////////////////////////////////
         bool operator ==(ValueName const &other) const
         { return compare(other) == 0; }

         ////////////////////////////////////////////////////////////
         // less than operator
         ////////////////////////////////////////////////////////////
         bool operator <(ValueName const &other) const
         { return compare(other) < 0; }

         ////////////////////////////////////////////////////////////
         // less than or equal operator
         ////////////////////////////////////////////////////////////
         bool operator <=(ValueName const &other) const
         { return compare(other) <= 0; }

         ////////////////////////////////////////////////////////////
         // greater than operator
         ////////////////////////////////////////////////////////////
         bool operator >(ValueName const &other) const
         { return compare(other) > 0; }

         ////////////////////////////////////////////////////////////
         // greater than or equal
         ////////////////////////////////////////////////////////////
         bool operator >=(ValueName const &other) const
         { return compare(other) >= 0; }

         ////////////////////////////////////////////////////////////
         // inequality
         ////////////////////////////////////////////////////////////
         bool operator !=(ValueName const &other) const
         { return compare(other) != 0; }

      private:
         ////////////////////////////////////////////////////////////
         // parse
         ////////////////////////////////////////////////////////////
         void parse(wchar_t const *s);
         
      private:
         ////////////////////////////////////////////////////////////
         // full_name
         //
         // Keep the original full tag name that was passed in.
         ////////////////////////////////////////////////////////////
         StrUni full_name;

         ////////////////////////////////////////////////////////////
         // station_name
         //
         // Specifies the station-name component of the construct.  Will be an empty string if the
         // station name was not specified in the source
         ////////////////////////////////////////////////////////////
         StrUni station_name;

         ////////////////////////////////////////////////////////////
         // table_name
         //
         // Specifies the table-name component of the construct.  Will be an empty string if the
         // table name was not specified in the source.
         ////////////////////////////////////////////////////////////
         StrUni table_name;

         ///////////////////////////////////////////////////////////
         // full_column_name
         //
         // Specifies the full column name including the subscript
         ///////////////////////////////////////////////////////////
         StrUni full_column_name;

         ////////////////////////////////////////////////////////////
         // column_name
         //
         // Specifies the column-name component.
         ////////////////////////////////////////////////////////////
         StrUni column_name;

         ////////////////////////////////////////////////////////////
         // subscripts
         ////////////////////////////////////////////////////////////
         subscripts_type subscripts;
      };
   };
};


#endif
