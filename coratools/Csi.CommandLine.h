/* Csi.CommandLine.h

   Copyright (C) 2000, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 13 January 2000
   Last Change: Tuesday 27 November 2018
   Last Commit: $Date: 2018-11-27 11:18:29 -0600 (Tue, 27 Nov 2018) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_CommandLine_h
#define Csi_CommandLine_h

#include <map>
#include <vector>
#include <set>
#include "StrUni.h"
#include "Csi.MsgExcept.h"


namespace Csi
{
   /**
    * Defines an object that assits in parsing command line strings.  This parser assumes that
    * command lines are laid out using the following syntax:
    *
    *    command_line = { option | argument }.
    *    option = "--" option_name [("="|":") option_value ].
    *    argument = string.
    *    option_name = string.
    *    option_value = string.
    *
    * A string may contain a sequence of quote characters '"' that cause the parser to include white
    * space in the string contents until a matching quote is found. If literal quotes are to be
    * included, it needs to be preceded by a backslash character as in '\"'.
    *
    * When the class is created, the client should invoke add_expected_option() as many times as
    * needful to describe the valid set of options that can be included in the command line. After
    * doing this, parse_command_line() can be invoked. Following this, options can be accessed by
    * name or by iterator and arguments can be accessed using iterators.
    *
    * This class is not intended to cover all possible command line uses but rather to simplify the
    * command line parsing process for a majority of applications.
    */
   class CommandLine
   {
   public:
      /**
       * Defines the class of exception that will be thrown when an unregistered option name is
       * encountered.
       */
      class ExcUnknownOption: public std::exception
      {
      public:
         ExcUnknownOption(StrAsc const &option_name_):
            option_name(option_name_)
         { }
         
         virtual ~ExcUnknownOption() throw ()
         { }
         
         virtual char const *what() const throw ()
         { return "Unrecognised command line option"; }
         
         StrAsc option_name;
      };

   public:
      //@group useful typedefs
      typedef std::map<StrAsc,StrAsc> options_type;
      typedef std::vector<StrAsc> arguments_type;
      //@endgroup

      /**
       * Constructor
       */
      CommandLine();

      /**
       * Destructor
       */
      ~CommandLine();

      /**
       * Registers an expected option name that can be expected on the command line.   All such
       * names must be registered before the command line is parsed.
       *
       * @param option_name Specifies the name of the option.
       */
      void add_expected_option(StrAsc const &option_name);

      /**
       * Attempts to parse the specified command line.  This method will look for the program name
       * as the first argument unlesss the skip_program_name property is set to true.
       *
       * @param command_line Specifies the command loine to parse.
       */
      void parse_command_line(wchar_t const *command_line);
      void parse_command_line(char const *command_line)
      {
         StrUni temp(command_line);
         parse_command_line(temp.c_str());
      }

      /**
       * Searches for the option specified by its name and returns its value, if found in the
       * supplied buffer.
       *
       * @return Returns true if the option was found.
       *
       * @param option_name Specifies the option name to search for.
       *
       * @param value_buffer Specifies the string to which the option value, if found, will be
       * written.
       */
      bool get_option_value(StrAsc const &option_name, StrAsc &value_buffer) const;
      bool get_option_value(StrAsc const &option_name, StrUni &value_buffer) const;

      /**
       * @return Returns the iterator to the first argument.
       */
      typedef arguments_type::const_iterator args_iterator;
      args_iterator begin() const { return arguments.begin(); }

      /**
       * @return Returns the iterator beyond the last argument.
       */
      args_iterator end() const { return arguments.end(); }

      /**
       * @return Returns the number of arguments.
       */
      arguments_type::size_type args_size() const { return arguments.size(); }

      /**
       * @return Returns true if the argument in the specified position is present.
       *
       * @param @buffer Specifies the buffer that will receive the argument value, if present.
       *
       * @param pos Specifies the zero based position of the argument.
       */
      bool get_argument(StrAsc &buffer, arguments_type::size_type pos);
      bool get_argument(StrUni &buff, arguments_type::size_type pos);

      /**
       * @return Overloads the subscript operator to return the argument at the specified position.
       *
       * @throws Throws an exception if the specified position is greater than or equal to the
       * argument list's size.
       *
       * @param pos Specifies the argument position.
       */
      StrAsc const &operator [](arguments_type::size_type pos)
      { return arguments.at(pos); }
      StrAsc const &operator [](arguments_type::size_type pos) const
      { return arguments.at(pos); } 
      
   private:
      /**
       * Specifies the set of option names that will be accepted when the command line is parsed.
       */
      typedef std::set<StrAsc> expected_options_type;
      expected_options_type expected_options;

      /**
       * Specifies the collection of options that were found when the command line was parsed.
       */
      options_type options;

      /**
       * Specifies the set of arguments that were found when the command line was parsed.
       */
      arguments_type arguments;
   };
};

#endif
