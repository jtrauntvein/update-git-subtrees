/* Csi.CsvRec.h

   Copyright (C) 2000, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 16 November 2000
   Last Change: Thursday 07 September 2017
   Last Commit: $Date: 2017-09-07 16:55:39 -0600 (Thu, 07 Sep 2017) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_CsvRec_h
#define Csi_CsvRec_h

#include <vector>
#include "StrAsc.h"
#include "StrUni.h"


namespace Csi
{
   /**
    * Defines an object that represents a set of strings processed from a
    * comma-separated values format coming from a file handle or an input
    * stream.
    */
   class CsvRec: public std::vector<StrAsc>
   {
   public:
      /**
       * Default constructor.
       *
       * @param json_strings_ Set to true if quoted strings are to be interpreted with JSON
       * encoding. 
       */
      CsvRec(bool json_strings_ = false);

      /**
       * Copy constructor
       *
       * @param other Specifies another record.
       */
      CsvRec(CsvRec const &other);

      /**
       * Destructor
       */
      virtual ~CsvRec();

      /**
       * Processes from the current file or stream location to the end of the line following that
       * location.  Values will be considered to be separated by unquoted commas.  When this method
       * has returned, any values will be stored in the container and will be accessable through
       * standard vector calls.
       *
       * @param in Specifies the file handle or input stream to parse.
       *
       * @param limit  Specifies the maximum number of values that should be stored.  All values
       * will be stored if this value is less than zero.  If this value is greater than
       * zero, any values encountered after this limit will be parsed and counted but not stored.
       *
       * @return Returns the total number of values that were parsed.  Note that if limit is greater
       * than or equal to zero, this value will differ from the number of values that were stored.
       */
      int read(std::istream &in, int limit = -1);
      int read(FILE *in, int limit = -1);

      /**
       * @return Returns true if the value specified by pos was part of a quoted string.
       *
       * @param pos Specifies a value iterator to check.
       */
      bool was_quoted(const_iterator pos);

   private:
      /**
       * Processes the next input character.
       *
       * @param ch  Specifies the character that was just read.
       *
       * @param limit Specifies the maximum number of values to parse.  If the line contains more
       * than this and this parameter is greater than zero, the values will be counted but not
       * stored.
       *
       * @return Returns true if the end of stream or end of the line was encountered.
       */
      bool process_next_character(char ch, int limit = -1);

      /**
       * Specifies the value of the last character that was processed.
       */
      char last_character;

      /**
       * Set to true if we are parsing between quoted values.
       */
      bool within_quotes;

      /**
       * Specifies the value that is currently being accumulated.
       */
      StrAsc current_value;

      /**
       * Set to true when the parser has moved beyond a comma.  This is used to determine if there
       * is a remnant that needs to be added when line processing is complete.
       */
      bool passed_comma;

      /**
       * Specifies the positions of fields that were quoted.
       */
      typedef std::vector<const_iterator> quoted_positions_type;
      quoted_positions_type quoted_positions;

      /**
       * Set to true if the current value being read contained a quotation mark.
       */
      bool value_was_quoted;

      /**
       * Specifies the number of values that have been parsed since the start.
       */
      int values_count;

      /**
       * Set to true if quoted strings are to be interpreted with JSON encoding rules.
       */
      bool json_strings;
   };


   /**
    * Defins an object that represents a set of unicode strings processed from
    * a comma-separated values format coming from a wide input stream.
    */
   class CsvRecUni: public std::vector<StrUni>
   {
   public:
      /**
       * Default constructor.
       *
       * @param json_strings_ Set to true if quoted strings are to be decoded as JSON strings.
       */
      CsvRecUni(bool json_strings_ = false);

      /**
       * Copy constructor
       *
       * @param other Specifies another record.
       */
      CsvRecUni(CsvRecUni const &other);

      /**
       * Destructor
       */
      virtual ~CsvRecUni();

      /**
       * Processes from the current file or stream location to the end of the line following that
       * location.  Values will be considered to be separated by unquoted commas.  When this method
       * has returned, any values will be stored in the container and will be accessable through
       * standard vector calls.
       *
       * @param in Specifies a wide input stream.
       *
       * @param limit  Specifies the maximum number of values that should be stored.  All values
       * will be stored if this value is less than zero.  If this value is greater than
       * zero, any values encountered after this limit will be parsed and counted but not stored.
       *
       * @return Returns the total number of values that were parsed.  Note that if limit is greater
       * than or equal to zero, this value will differ from the number of values that were stored.
       */
      int read(std::wistream &in, int limit = -1);

      /**
       * @return Returns true if the value specified by pos was part of a quoted string.
       *
       * @param pos Specifies a value iterator to check.
       */
      bool was_quoted(const_iterator pos);

   private:
      /**
       * Processes the next input character.
       *
       * @param ch  Specifies the character that was just read.
       *
       * @param limit Specifies the maximum number of values to parse.  If the line contains more
       * than this and this parameter is greater than zero, the values will be counted but not
       * stored.
       *
       * @return Returns true if the end of stream or end of the line was encountered.
       */
      bool process_next_character(wchar_t ch, int limit = -1);

      /**
       * Specifies the value of the last character that was processed.
       */
      wchar_t last_character;

      /**
       * Set to true if we are parsing between quoted values.
       */
      bool within_quotes;

      /**
       * Specifies the value that is currently being accumulated.
       */
      StrUni current_value;

      /**
       * Set to true when the parser has moved beyond a comma.  This is used to determine if there
       * is a remnant that needs to be added when line processing is complete.
       */
      bool passed_comma;

      /**
       * Specifies the positions of fields that were quoted.
       */
      typedef std::vector<const_iterator> quoted_positions_type;
      quoted_positions_type quoted_positions;

      /**
       * Set to true if the current value being read contained a quotation mark.
       */
      bool value_was_quoted;

      /**
       * Specifies the number of values that have been parsed since the start.
       */
      int values_count;

      /**
       * Set to true if quoted strings are to be interpreted with JSON encoding rules.
       */
      bool json_strings;
   };
};

#endif
