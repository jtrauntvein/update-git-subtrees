/* Csi.Expression.Editor.h

   Copyright (C) 2013, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 24 January 2013
   Last Change: Monday 21 July 2014
   Last Commit: $Date: 2014-07-21 12:09:34 -0600 (Mon, 21 Jul 2014) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Expression_Editor_h
#define Csi_Expression_Editor_h

#include "Csi.Expression.ExpressionHandler.h"
#include "Csi.Expression.TokenTypes.h"


namespace Csi
{
   namespace Expression
   {
      namespace EditorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class SetupFunction
         ////////////////////////////////////////////////////////////
         class SetupFunction
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            typedef ExpressionHandler::parsed_tokens_type tokens_type;
            SetupFunction(tokens_type &tokens, StrUni const &source);

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~SetupFunction();

            ////////////////////////////////////////////////////////////
            // get_function
            ////////////////////////////////////////////////////////////
            typedef Csi::LightPolySharedPtr<Token, Function> function_handle;
            function_handle &get_function()
            { return function; }

            // @group: container declarations

            ////////////////////////////////////////////////////////////
            // begin
            ////////////////////////////////////////////////////////////
            typedef std::pair<StrUni, StrUni> value_type;
            typedef std::vector<value_type> arguments_type;
            typedef arguments_type::iterator iterator;
            typedef arguments_type::const_iterator const_iterator;
            iterator begin()
            { return arguments.begin(); }
            const_iterator begin() const
            { return arguments.begin(); }

            ////////////////////////////////////////////////////////////
            // end
            ////////////////////////////////////////////////////////////
            iterator end()
            { return arguments.end(); }
            const_iterator end() const
            { return arguments.end(); }

            ////////////////////////////////////////////////////////////
            // empty
            ////////////////////////////////////////////////////////////
            bool empty() const
            { return arguments.empty(); }

            ////////////////////////////////////////////////////////////
            // size
            ////////////////////////////////////////////////////////////
            typedef arguments_type::size_type size_type;
            size_type size() const
            { return arguments.size(); }

            ////////////////////////////////////////////////////////////
            // subscript operator
            ////////////////////////////////////////////////////////////
            value_type &operator [](size_type index)
            { return arguments[index]; }
            value_type const &operator [](size_type index) const
            { return arguments[index]; }

            ////////////////////////////////////////////////////////////
            // at
            ////////////////////////////////////////////////////////////
            value_type &at(size_type index)
            { return arguments.at(index); }
            value_type const &at(size_type index) const
            { return arguments.at(index); }
            
            // @endgroup:

            ////////////////////////////////////////////////////////////
            // write
            ////////////////////////////////////////////////////////////
            void write(std::ostream &out);
            void write(std::wostream &out);

         private:
            ////////////////////////////////////////////////////////////
            // function
            ////////////////////////////////////////////////////////////
            function_handle function;

            ////////////////////////////////////////////////////////////
            // arguments
            ////////////////////////////////////////////////////////////
            arguments_type arguments;
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class Editor
      //
      // This class defines a component that an application can use to help
      // with editing an expression string.  It can break an expression with
      // aborting tokens into a set of sub-expressions for the aborting tokens
      // as well as a sub-expression for the body.  It will also break the
      // aborting token sub-expressions into lists of arguments.  
      ////////////////////////////////////////////////////////////
      class Editor
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Editor(StrUni const &source = L"", TokenFactory *factory_ = 0);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Editor();

         ////////////////////////////////////////////////////////////
         // get_body
         ////////////////////////////////////////////////////////////
         StrUni const &get_body() const
         { return body; }

         ////////////////////////////////////////////////////////////
         // set_body
         ////////////////////////////////////////////////////////////
         void set_body(StrUni const &body_)
         { body = body_; }

         // @group: declarations to act asa container for setup functions

         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef EditorHelpers::SetupFunction setup_function_type;
         typedef LightSharedPtr<setup_function_type> value_type;
         typedef std::list<value_type> setup_functions_type;
         typedef setup_functions_type::iterator iterator;
         typedef setup_functions_type::const_iterator const_iterator;
         iterator begin()
         { return setup_functions.begin(); }
         const_iterator begin() const
         { return setup_functions.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end()
         { return setup_functions.end(); }
         const_iterator end() const
         { return setup_functions.end(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return setup_functions.empty(); }

         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         typedef setup_functions_type::size_type size_type;
         size_type size() const
         { return setup_functions.size(); }
         
         // @endgroup:

         ////////////////////////////////////////////////////////////
         // parse
         ////////////////////////////////////////////////////////////
         void parse(StrUni const &source);

         ////////////////////////////////////////////////////////////
         // write
         ////////////////////////////////////////////////////////////
         void write(std::ostream &out);
         void write(std::wostream &out);
         
      private:
         ////////////////////////////////////////////////////////////
         // body
         ////////////////////////////////////////////////////////////
         StrUni body;

         ////////////////////////////////////////////////////////////
         // setup_functions
         ////////////////////////////////////////////////////////////
         setup_functions_type setup_functions;

         ////////////////////////////////////////////////////////////
         // factory
         ////////////////////////////////////////////////////////////
         TokenFactory *factory;
         bool owns_factory;
      };
   };
};


#endif
