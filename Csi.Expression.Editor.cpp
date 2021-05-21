/* Csi.Expression.Editor.cpp

   Copyright (C) 2013, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 24 January 2013
   Last Change: Friday 25 January 2013
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Expression.Editor.h"


namespace Csi
{
   namespace Expression
   {
      ////////////////////////////////////////////////////////////
      // class Editor definitions
      ////////////////////////////////////////////////////////////
      Editor::Editor(StrUni const &source, TokenFactory *factory_):
         factory(factory_),
         owns_factory(false)
      {
         if(factory == 0)
         {
            factory = new TokenFactory;
            owns_factory = true;
         }
         if(source.length() > 0)
         {
            try
            {
               parse(source);
            }
            catch(std::exception &)
            {
               if(owns_factory)
                  delete factory;
               throw;
            }
         }
      } // constructor


      Editor::~Editor()
      {
         if(owns_factory)
            delete factory;
         setup_functions.clear();
      } // destructor


      void Editor::parse(StrUni const &source)
      {
         // we need to first parse the source into a collection of parsed tokens
         ExpressionHandler parser(factory);
         ExpressionHandler::parsed_tokens_type tokens;
         body.cut(0);
         setup_functions.clear();
         parser.make_tokens(tokens, source);

         // we now need to find all of the setup functions in the token list
         bool complete(false);
         while(!tokens.empty() && !complete)
         {
            ExpressionHandler::parsed_tokens_type::value_type &token(tokens.front());
            if(token.token->abort_after_eval())
            {
               value_type function(new setup_function_type(tokens, source));
               setup_functions.push_back(function);
            }
            else
               complete = true;
         }

         // the last thing to do is to extract the body from the source
         if(!tokens.empty())
            source.sub(body, tokens.front().begin_pos, source.length());
      } // parse


      void Editor::write(std::ostream &out)
      {
         for(iterator fi = begin(); fi != end(); ++fi)
         {
            value_type &f(*fi);
            f->write(out);
         }
         out << body;
      } // write


      void Editor::write(std::wostream &out)
      {
         for(iterator fi = begin(); fi != end(); ++fi)
         {
            value_type &f(*fi);
            f->write(out);
         }
         out << body;
      } // write
      
      
      namespace EditorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class SetupFunction definitions
         ////////////////////////////////////////////////////////////
         SetupFunction::SetupFunction(tokens_type &tokens, StrUni const &source)
         {
            // we will first grab the token from the start of the list
            tokens_type::value_type parsed = tokens.front();
            size_t offset(parsed.begin_pos);
            function = parsed.token;
            tokens.pop_front();

            // we now need to build up the list of arguments.  We will do so examining each token to
            // find the terminating semi-colon.
            int paren_nest_count(0);
            StrUni argument;
            StrUni fragment;
            StrUni argument_name;
            while(!tokens.empty())
            {
               tokens_type::value_type parsed(tokens.front());
               tokens.pop_front();
               if(parsed.token->is_lparen())
                  ++paren_nest_count;
               if(parsed.token->is_rparen())
                  --paren_nest_count;
               if(parsed.token->is_semi_colon())
                  break;
               if((parsed.token->is_comma() && paren_nest_count == 1) ||
                  (parsed.token->is_rparen() && paren_nest_count == 0))
               {
                  // we need to check to see if there are too many arguments
                  if(arguments.size() + 1 > function->get_max_arguments())
                     throw ExcParseError(parsed.begin_pos, "too many arguments specified");
                  argument_name = function->get_argument_name((uint4)arguments.size());
                  arguments.push_back(std::make_pair(argument_name, argument));
                  argument.cut(0);
               }
               else if(!(parsed.token->is_lparen() && paren_nest_count == 1))
               {
                  source.sub(fragment, parsed.begin_pos, parsed.length);
                  argument.append(fragment);
               }
            }
            if(arguments.size() < function->get_min_arguments())
               throw ExcParseError(offset, "too few arguments specified");
         } // constructor


         SetupFunction::~SetupFunction()
         {
            function.clear();
            arguments.clear();
         } // destructor


         void SetupFunction::write(std::ostream &out)
         {
            function->format(out);
            out << "(";
            for(iterator ai = begin(); ai != end(); ++ai)
            {
               if(ai != begin())
                  out << ", ";
               out << ai->second;
            }
            out << ");";
         } // write


         void SetupFunction::write(std::wostream &out)
         {
            OStrAscStream temp;
            function->format(temp);
            out << temp.str() << L"(";
            for(iterator ai = begin(); ai != end(); ++ai)
            {
               if(ai != begin())
                  out << L", ";
               out << ai->second;
            }
            out << L");";
         } // write
      };
   };
};

