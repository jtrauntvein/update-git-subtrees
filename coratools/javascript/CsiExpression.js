/* CsiExpression.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Friday 21 December 2018
   Last Commit: $Date: 2019-07-29 15:37:53 -0600 (Mon, 29 Jul 2019) $
   Last Changed by: $Author: jon $

*/

/**
 * Defines an object that can evaluate a CRBasic type expression.
 *
 * @param {array | string} tokens If passed as an array, sets the postfix stack of operations
 * that will be evaluated.  If specified as a string, specifies the expression in infix
 * form that needs to be parsed and converted.
 */
function CsiExpression(tokens)
{
   var expression = this;
   this.tokens = tokens;
   this.ownerComponent = null; //set when added to component
   this.has_table_ref = false;
   this.variables = null;
   this.tokens.forEach(function(token) {
      if(token && typeof token.set_owner_expression === "function")
      {
         token.set_owner_expression(expression);
         if(!expression.has_table_ref && token.hasOwnProperty("is_table"))
            expression.has_table_ref = token.is_table;
      }
   });
}


/**
 * @return {token} Evaluates the postfix expression and returns the final token.
 */
CsiExpression.prototype.evaluate = function()
{
   var data_stack = [];
   var expression = this;
   this.tokens.forEach(function(token) {
      token.evaluate(data_stack, expression.tokens);
   });
   return data_stack[0];
};


/**
 * Invokes reset on all tokens that support it.
 */
CsiExpression.prototype.reset = function()
{
   this.tokens.forEach(function(token) {
      if(token && typeof token.reset === "function")
         token.reset();
   });
};


/**
 * Lexically breaks down a source expression string into a collection of tokens.  This is the first stage of
 * converting an expression string into a postfix expression.
 *
 * @return {array} Generates an array of objects that represent the significant
 * tokens in the specified expression string along with their starting position
 * in the source and their length.
 *
 * @param {string} source_ Specifies the source string in infix notation.
 */
CsiExpression.make_string_tokens = function(source_)
{
   var is_operator = function(token) {
      var rtn = false;
      switch(token)
      {
      case '+':
      case '-':
      case '*':
      case '/':
      case '(':
      case ')':
      case '^':
      case ',':
      case '=':
      case '<':
      case '>':
      case ';':
         rtn = true;
         break;
      }
      return rtn;
   };
   var is_space = function(ch) { return /\s/.test(ch); };
   const state_between_tokens = 1;
   const state_in_name = 2;
   const state_quoted = 3;
   const state_after_number = 4;
   const state_after_decimal = 5;
   const state_after_exp = 6;
   const state_after_exp_sign = 7;
   const state_after_amp = 8;
   const state_after_amp_hex = 9;
   const state_after_amp_bin = 10;
   const state_after_lt = 11;
   const state_after_gt = 12;
   const state_after_dollar = 13;
   const state_in_string = 14;
   var state = state_between_tokens;
   var i;
   var source = source_.split("");
   var ch;
   var cur_word = "";
   var rtn = [];
   
   for(i = 0; i < source.length; ++i)
   {
      ch = source[i];
      if(state === state_between_tokens)
      {
         if(ch >= '0' && ch <= '9')
         {
            cur_word += ch;
            state = state_after_number;
         }
         else if(ch === '.')
         {
            cur_word += ch;
            state = state_after_decimal;
         }
         else if(ch === '&')
         {
            cur_word += ch;
            state = state_after_amp;
         }
         else if(ch === '>')
         {
            cur_word += ch;
            state = state_after_gt;
         }
         else if(ch === '<')
         {
            cur_word += ch;
            state = state_after_lt;
         }
         else if(is_operator(ch))
         {
            if(cur_word.length)
               rtn.push({ token: cur_word, start: i - cur_word.length });
            rtn.push({ token: ch, start: i });
            cur_word = "";
         }
         else if(ch === '\"')
         {
            cur_word += ch;
            state = state_quoted;
         }
         else if(ch === '$')
         {
            if(cur_word.length)
               rtn.push({ token: cur_word, start: i - cur_word.length });
            cur_word = ch;
            state = state_after_dollar;
         }
         else if(!is_space(ch))
         {
            cur_word += ch;
            state = state_in_name;
         }
      }
      else if(state === state_in_name)
      {
         if(is_space(ch) || is_operator(ch))
         {
            rtn.push({ token: cur_word, start: i - cur_word.length });
            cur_word = "";
            if(!is_space(ch))
               --i;
            state = state_between_tokens;
         }
         else if(ch === '\"')
         {
            cur_word += ch;
            state = state_quoted;
         }
         else
            cur_word += ch;
      }
      else if(state === state_quoted)
      {
         if(ch === '\"')
         {
            if(cur_word.length)
            {
               cur_word.append(ch);
               state = state_in_name;
            }
            else
               state = state_between_tokens;
         }
         else
            cur_word += ch;
      }
      else if(state === state_after_number)
      {
         if(ch >= '0' && ch <= '9')
            cur_word += ch;
         else if(ch === '.')
         {
            cur_word += ch;
            state = state_after_decimal;
         }
         else if(ch === 'e' || ch === 'E')
         {
            cur_word += ch;
            state = state_after_exp;
         }
         else if(is_operator(ch) || is_space(ch))
         {
            rtn.push({ token: cur_word, start: i - cur_word.length });
            cur_word = "";
            if(!is_space(ch))
               --i;
            state = state_between_tokens;
         }
         else
            throw { message: "unexpected character in number constant", pos: i };
      }
      else if(state === state_after_decimal)
      {
         if(ch >= '0' && ch <= '9')
            cur_word += ch;
         else if(ch === 'e' || ch === 'E')
         {
            cur_word += ch;
            state = state_after_exp;
         }
         else if(is_operator(ch) || is_space(ch))
         {
            rtn.push({ token: cur_word, start: i - cur_word.length });
            cur_word = "";
            if(!is_space(ch))
               --i;
            state = state_between_tokens;
         }
         else
            throw { message: "unexpected character in number constant", pos: i };
      }
      else if(state === state_after_exp)
      {
         if(ch === '+' || ch === '-' || (ch >= '0' && ch <= '9'))
         {
            cur_word += ch;
            state = state_after_exp_sign;
         }
         else
            throw { message: "unexpected character in number constant", pos: i };
      }
      else if(state === state_after_exp_sign)
      {
         if(ch >= '0' && ch <= '9')
            cur_word += ch;
         else if(is_space(ch) || is_operator(ch))
         {
            rtn.push({token: cur_word, start: i - cur_word.length });
            cur_word = "";
            if(!is_space(ch))
               --i;
            state = state_between_tokens;
         }
         else
            throw { message: "unexpected character in number constant", pos: i};
      }
      else if(state === state_after_amp)
      {
         if(ch === 'h' || ch === 'H')
         {
            cur_word += ch;
            state = state_after_amp_hex;
         }
         else if(ch === 'b' || ch === 'B')
         {
            cur_word += ch;
            state = state_after_amp_bin;
         }
         else
            throw { message: "invalid ampersand constant", pos: i };
      }
      else if(state === state_after_amp_hex)
      {
         if((ch >= '0' && ch <= '9') ||
            (ch >= 'a' && ch <= 'f') ||
            (ch >= 'A' && ch <= 'F'))
            cur_word += ch;
         else if(is_space(ch) || is_operator(ch))
         {
            rtn.push({ token: cur_word, start: i - cur_word.length });
            cur_word = "";
            if(!is_space(ch))
               --i;
            state = state_between_tokens;
         }
         else
            throw { message: "invalid hexadecimal constant", pos: i };
      }
      else if(state === state_after_amp_bin)
      {
         if(ch === '0' || ch === '1')
            cur_word += ch;
         else if(is_space(ch) || is_operator(ch))
         {
            rtn.push({token: cur_word, start: i - cur_word.length});
            cur_word = "";
            if(!is_space(ch))
               --i;
            state = state_between_tokens;
         }
         else
            throw {message: "invalid binary constant", pos: i};
      }
      else if(state === state_after_lt)
      {
         if(ch === '>' || ch === '=')
            cur_word += ch;
         else
            --i;
         rtn.push({token: cur_word, start: i - cur_word.length});
         state = state_between_tokens;
      }
      else if(state === state_after_gt)
      {
         if(ch === '=')
            cur_word += ch;
         else
            --i;
         cur_word = "";
         state = state_between_tokens;
      }
      else if(state === state_after_dollar)
      {
         if(ch === '\"')
         {
            cur_word += ch;
            state = state_in_string;
         }
         else
            throw {message: "double quotes expected after an unquoted dollar sign", pos: i};
      }
      else if(state === state_in_string)
      {
         cur_word += ch;
         if(ch === '\"')
         {
            rtn.push({token: cur_word, start: i - cur_word.length});
            cur_word = "";
            state = state_between_tokens;
         }
      }
   }
   if(cur_word.length > 0)
      rtn.push({token: cur_word, start: i - cur_word.length});
   if(state === state_quoted)
      throw {message: "unbalanced quotes in expression", pos: i};
   return rtn;
};


/**
 * Implements the second stage of the expressions parser.  This stage will convert the specified
 * array of string tokens into expression token objects.
 *
 * @param {array} string_tokens Specifies the collection of string tokens.  This array is generated
 * by calling CsiExpression.make_string_tokens.
 *
 * @return {object} Returns an object that contains two array properties: tokens and variables.
 */
CsiExpression.make_tokens = function(string_tokens)
{
   var rtn = { tokens: [], variables: {} };
   var prev_token = null;
   string_tokens.forEach(function(string_token) {
      var current_token;
      var token_name = string_token.token.toUpperCase();
      
      if(rtn.variables.hasOwnProperty(token_name))
      {
         current_token = rtn.variables[token_name];
         rtn.tokens.push({token: current_token, start: string_token.start});
      }
      else
      {
         current_token = CsiExprToken.make_token(prev_token, token_name);
         if(current_token)
         {
            if(current_token.is_variable())
               rtn.variables[token_name] = current_token;
            rtn.tokens.push({token: current_token, start: string_token.start});
         }
         else
            throw {message: "unrecognised token name", pos: string_token.pos};
         prev_token = current_token;
      }
   });
   return rtn;
};


/**
 * Implements the third stage of parsing an expression string.  This method will
 * converts an infix collection of tokens and converts it to a postfix collection.
 *
 * @param {object} tokens Specifies an object that has a property that specifies the
 * collection of parsed tokens and another property that specifies the variables
 * in the expression.
 *
 * @return {object} Returns an object that has properties that specify the postfix
 * operations stack and the variables in the expression.
 */
CsiExpression.infix_to_postfix = function(tokens)
{
   var op_stack = [];
   var rtn = { tokens: [], variables: tokens.variables };
   var popped_token;
   tokens.tokens.forEach(function(current) {
      var paren_token;
      var cont;
      
      if(current.token.is_comma())
      {
         if(op_stack.length > 0)
         {
            // we need to pop off everything off the op stack that has lesser priority
            // then the current token.
            popped_token = op_stack[op_stack.length - 1];
            while(op_stack.length > 0 &&
                  popped_token.token.get_priority() < current.token.get_priority() &&
                  !popped_token.token.is_lparen())
            {
               op_stack.pop();
               rtn.tokens.push(popped_token.token);
               if(op_stack.length > 0)
                  popped_token = op_stack[op_stack.length - 1];
            }
            if(!popped_token.token.is_lparen())
               throw { message: "comma must appear within parentheses", pos: popped_token.start };

            // we need to increment the argument count, if any, of the token in the front of the
            // left parenthese.
            paren_token = popped_token;
            op_stack.pop();
            if(op_stack.length > 0)
            {
               popped_token = op_stack[op_stack.length - 1];
               popped_token.token.increment_args_count();
            }
         }
      }
      else if(current.token.is_rparen())
      {
         if(op_stack.length > 0)
         {
            // we need to pop ops from the stack until we find the matching left parenthese.
            popped_token = op_stack[op_stack.length - 1];
            while(op_stack.length > 0 && !popped_token.token.is_lparen())
            {
               op_stack.pop();
               rtn.tokens.push(popped_token.token);
               if(op_stack.length > 0)
                  popped_token = op_stack[op_stack.length - 1];
            }
            if(!popped_token.token.is_lparen())
               throw {message: "Mismatched parentheses", pos: popped_token.start};
            else
               op_stack.pop();
         }
      }
      else if(current.token.is_lparen())
      {
         // always push left parens on the op stack.
         if(op_stack.length > 0)
            op_stack[op_stack.length - 1].token.clear_args_count();
         op_stack.push(current);
      }
      else if(current.token.is_operator())
      {
         if(op_stack.length === 0)
            op_stack.push(current);
         else
         {
            cont = true;
            while(cont)
            {
               if(op_stack.length === 0)
                  cont = false;
               else
               {
                  popped_token = op_stack[op_stack.length - 1];
                  if(popped_token.token.is_lparen() ||
                     popped_token.token.get_priority() < current.token.get_priority())
                  {
                     cont = false;
                  }
                  else if(popped_token.token.get_priority() === current.token.get_priority() &&
                     current.token.get_priority() >= CsiExprToken.prec_max_operator)
                  {
                     cont = false;
                  }
               }
               if(cont)
               {
                  popped_token = op_stack.pop();
                  rtn.tokens.push(popped_token.token);
               }
            }
            op_stack.push(current);
         }
      }
      else if(current.token.is_semi_colon())
      {
         while(op_stack.length > 0)
         {
            popped_token = op_stack.pop();
            if(!popped_token.token.is_lparen())
               rtn.tokens.push(popped_token.token);
         }
      }
      else // we assume an operand
         rtn.tokens.push(current.token);
   });

   // we have finished going through the original stack.  We need to pop off any remaining
   // operators on the op stack
   while(op_stack.length > 0)
   {
      popped_token = op_stack.pop();
      if(popped_token.token.is_lparen() || popped_token.token.is_rparen())
         throw {message: "mismatched parentheses", pos: popped_token.start};
      rtn.tokens.push(popped_token.token);
   }
   return rtn;
};   


/**
 * @return Returns an expression object that has been parsed from a string source.
 *
 * @param {string} source Specifies the source for the string.
 */
CsiExpression.parse = function(source)
{
   var string_tokens = CsiExpression.make_string_tokens(source);
   var tokens = CsiExpression.make_tokens(string_tokens);
   var postfix = CsiExpression.infix_to_postfix(tokens);
   var rtn = new CsiExpression(postfix.tokens);
   rtn.variables = postfix.variables;
   return rtn;
};
