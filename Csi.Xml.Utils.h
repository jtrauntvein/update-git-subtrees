/* Csi.Xml.Utils.h

   Copyright (C) 2016, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 01 March 2016
   Last Change: Tuesday 01 March 2016
   Last Commit: $Date: 2016-03-03 15:24:10 -0600 (Thu, 03 Mar 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Xml_Utils_h
#define Csi_Xml_Utils_h

#include "StrBin.h"
#include "StrAsc.h"
#include "StrUni.h"
#include "MsgExcept.h"
#include <iostream>


namespace Csi
{
   namespace Xml
   {
      /**
       * Defines a function that implements standard policy for writing an element or attribute
       * value to an output stream.  This policy basically dictates that all control codes or
       * character codes outside of the ASCII range are represented as numeric entities in order to
       * avoid character encoding confusion.
       *
       * @param out Specifies the output stream.
       *
       * @param data Specifies the data to be written.
       */
      void output_xml_data(std::ostream &out, StrUni const &data);
      void output_xml_data(std::wostream &out, StrUni const &data);

      /**
       * Defines a function that reads XML data (cdata or attribute value) into a wide string and
       * converts all standard entities in the process.
       *
       * @param dest Specifies the destination buffer.
       *
       * @param buff Specifies the input buffer.
       *
       * @param buff_len Specifies the input buffer length.
       */
      void input_xml_data(StrUni &dest, char const *buff, size_t buff_len);
      void input_xml_data(StrAsc &dest, char const *buff, size_t buff_len);


      class ParsingError: public MsgExcept
      {
      private:
         ////////////////////////////////////////////////////////////
         // row_no
         ////////////////////////////////////////////////////////////
         size_t row_no;

         ////////////////////////////////////////////////////////////
         // column_no
         ////////////////////////////////////////////////////////////
         size_t column_no;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ParsingError(
            char const *message_,
            size_t row_no_,
            size_t column_no_);

         ////////////////////////////////////////////////////////////
         // get_row_no
         ////////////////////////////////////////////////////////////
         size_t get_row_no() const
         { return row_no; }

         ////////////////////////////////////////////////////////////
         // get_column_no
         ////////////////////////////////////////////////////////////
         size_t get_column_no() const
         { return column_no; }
      };
   };
};


#endif
