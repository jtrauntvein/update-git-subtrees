/* CsiExprToken.js

   Copyright (C) 2018, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 08 May 2018
   Last Change: Monday 29 July 2019
   Last Commit: $Date: 2020-07-14 15:47:08 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

/**
 * Defines a base class for all expression tokens.
 */
function CsiExprToken()
{ }
CsiExprToken.prec_default = 0xffff;
CsiExprToken.prec_semi_colon = 11;
CsiExprToken.prec_paren = 10;
CsiExprToken.prec_comma = 9;
CsiExprToken.prec_max_operator = 8;
CsiExprToken.prec_function = 7;
CsiExprToken.prec_negation = 6;
CsiExprToken.prec_expon = 5;
CsiExprToken.prec_mult_div_mod = 4;
CsiExprToken.prec_add_subtr = 3;
CsiExprToken.prec_comparator = 2;
CsiExprToken.prec_logic_op = 1;
CsiExprToken.prec_bit_op = 0;


/**
 * Specifies a registry of creation methods for various types of tokens.
 */
CsiExprToken.creators = {};


/**
 * Registers a creator method that will generate a token type identified by the
 * specified name.
 *
 * @param {string} key Specifies the key for the token type.
 *
 * @param {function} creator Specifies the function that will generate the token
 * object being requested.
 */
CsiExprToken.add_creator = function(key, creator)
{
   CsiExprToken.creators[key.toUpperCase()] = creator;
};


/**
 * Must be overloaded to evaluate the token by pushing or popping values on the specified operand stack.
 *
 * @param {array} stack Specifies the operand stack.
 *
 * @param {array} tokens Specifies the operation tokens list.
 */
CsiExprToken.prototype.evaluate = function(stack, tokens)
{ };


/**
 * @return {number} Can be overloaded to return the priority for this token.
 */
CsiExprToken.prototype.get_priority = function()
{ return CsiExprToken.prec_default; };


/**
 * @return {boolean} Can be overloaded to return true if this token represents a left parenthese.
 */
CsiExprToken.prototype.is_lparen = function()
{ return false; };


/**
 * @return {boolean} Can be overloaded to return true if this token represents a right parenthese.
 */
CsiExprToken.prototype.is_rparen = function()
{ return false; };


/**
 * @return {boolean} Returns true if the token is a semi-colon.
 */
CsiExprToken.prototype.is_semi_colon = function()
{ return false; };


/**
 * @return {boolean} Returns true if this token is a function.
 */
CsiExprToken.prototype.is_function = function()
{ return false; };


/**
 * @return {boolean} Can be overloaded to return true if this token is an operator.
 */
CsiExprToken.prototype.is_operator = function()
{ return false; };


/**
 * @return {boolean} Can be overloaded to return true if this token is a variable.
 */
CsiExprToken.prototype.is_variable = function()
{ return false; };


/**
 * @return {boolean} Can be overloaded to return true if this token is an operand.
 */
CsiExprToken.prototype.is_operand = function()
{ return false; };


/**
 * @return {boolean} Can be overloaded to return true if this token is a comma.
 */
CsiExprToken.prototype.is_comma = function()
{ return false; };


/**
 * Can be overloaded to clear variable length argument counts.
 */
CsiExprToken.prototype.clear_args_count = function()
{ };


/**
 * Can be overloaded to increment the variable length argument count.
 */
CsiExprToken.prototype.increment_args_count = function()
{ };

   
/**
 * Defines a left parenthese token
 */
function CsiLeftParen()
{ }
CsiLeftParen.prototype = new CsiExprToken();
CsiExprToken.add_creator("(", function() { return new CsiLeftParen(); });

CsiLeftParen.prototype.is_lparen = function()
{ return true; };

CsiLeftParen.prototype.get_priority = function()
{ return CsiExprToken.prec_paren; };


/**
 * Defines a right parenthese token.
 */
function CsiRightParen()
{ }
CsiRightParen.prototype = new CsiExprToken();
CsiExprToken.add_creator(")", function() { return new CsiRightParen(); });


CsiRightParen.prototype.is_rparen = function()
{ return true; };

CsiRightParen.prototype.get_priority = function()
{ return CsiExprToken.prec_paren; };


/**
 * Defines a semicolon operator.
 */
function CsiSemicolon()
{ }
CsiSemicolon.prototype = new CsiExprToken();
CsiExprToken.add_creator(";", function() { return new CsiSemicolon(); });


CsiSemicolon.prototype.get_priority = function()
{ return CsiExprToken.prec_semi_colon; };

CsiSemicolon.prototype.is_semi_colon = function()
{ return true; };


/**
 * Defines a comma operator.
 */
function CsiComma()
{ }
CsiComma.prototype = new CsiExprToken();
CsiExprToken.add_creator(",", function() { return new CsiComma(); });

                         
CsiComma.prototype.is_comma = function()
{ return true; };

CsiComma.prototype.get_priority = function()
{ return CsiExprToken.prec_comma; };

CsiComma.prototype.is_operator = function()
{ return true; };


/**
 * Defines an object that represents an operand.
 */
function CsiOperand()
{
   this.value = Number(0);
   this.value_type = CsiOperand.value_double;
   this.timestamp = new CsiLgrDate();
   if(arguments.length === 1)
   {
      this.value = arguments[0].value;
      this.value_type = arguments[0].value_type;
      this.timestamp = arguments[0].timestamp;
   }
   else if(arguments.length >= 2)
      this.set_val(arguments[0], arguments[1]);
}
CsiOperand.prototype = new CsiExprToken();
CsiOperand.value_double = 0;
CsiOperand.value_int = 1;
CsiOperand.value_string = 2;
CsiOperand.value_date = 3;


CsiOperand.prototype.is_operand = function()
{ return true; };


CsiOperand.prototype.evaluate = function(stack)
{
   stack.push(this);
};


CsiOperand.prototype.get_val = function()
{
   var rtn = 0;
   var temp;
   
   switch(this.value_type)
   {
   case CsiOperand.value_double:
   case CsiOperand.value_int:
      rtn = this.value;
      break;
      
   case CsiOperand.value_string:
      temp = this.value.toUpperCase();
      if(temp === "INF" || temp === "+INF")
         rtn = Infinity;
      else if(temp === "-INF")
         rtn = -Infinity;
      else if(temp === "NAN")
         rtn = NaN;
      else
         rtn = parseFloat(temp);
      break;
      
   case CsiOperand.value_date:
      rtn = this.value.milliSecs * CsiLgrDate.nsecPerMSec;
      break;
   }
   return rtn;
};


CsiOperand.prototype.get_val_str = function()
{
   var rtn = "";
   switch(this.value_type)
   {
   case CsiOperand.value_double:
      rtn = sprintf(this.value, "%g");
      break;
      
   case CsiOperand.value_int:
      rtn = sprintf(this.value, "%d");
      break;
      
   case CsiOperand.value_string:
      rtn = this.value;
      break;
      
   case CsiOperand.value_date:
      rtn = this.value.format("%Y-%m-%d %H:%M:%S%x");
      break;
   }
   return rtn;
};

CsiOperand.prototype.get_val_date = function ()
{
   var rtn = 0;
   switch (this.value_type) {
   
   case CsiOperand.value_double:
   case CsiOperand.value_int:
      rtn = new CsiLgrDate(this.value / CsiLgrDate.nsecPerMSec);
      break;      
   case CsiOperand.value_string:
      rtn = CsiLgrDate.fromStr(this.value);
      break;
   case CsiOperand.value_date:
         rtn = this.value;
      break;
   }
   return rtn;
};


CsiOperand.prototype.get_val_int = function()
{
   var rtn = 0;
   switch(this.value_type)
   {
   case CsiOperand.value_double:
      rtn = Math.floor(this.value);
      break;
      
   case CsiOperand.value_int:
      rtn = this.value;
      break;
      
   case CsiOperand.value_string:
      // if the string consists of all hex digits, we will convert it as hex
      if(this.value.search(/[a-fA-F0-9]+^/) === 0)
      {
         rtn = parseInt(this.value, 16);
      }
      else if(this.value.search(/[0-9]+^/) >= 0)
      {
         rtn = parseInt(this.value, 10);
      }
      else
      {
         // in order to honour exponential notation, we will first convert the
         // value to floating point and then to an integer.
         rtn = Math.floor(parseFloat(this.value));
      }
      break;
      
   case CsiOperand.value_date:
      rtn = this.value.milliSecs * CsiLgrDate.nsecPerMSec;
      break;
   }
   return rtn;
};


CsiOperand.prototype.set_val = function(value, timestamp)
{
   var value_type = typeof(value);
   switch(value_type)
   {
   case "number":
      this.value = value;
      this.value_type = CsiOperand.value_double;
      break;
      
   case "string":
      this.value = value;
      this.value_type = CsiOperand.value_string;
      break;
      
   case "boolean":
      if(value)
         this.value = -1;
      else
         this.value = 0;
      this.value_type = CsiOperand.value_int;
      break;
      
   case "object":
      if(value instanceof Number)
      {
         this.value = value;
         this.value_type = CsiOperand.value_double;
      }
      else if(value instanceof String)
      {
         this.value = value;
         this.value_type = CsiOperand.value_string;
      }
      else if(value instanceof Boolean)
      {
         this.value = (value ? -1 : 0);
         this.value_type = CsiOperand.value_int;
      }
      else if(value instanceof CsiLgrDate)
      {
         this.value = new CsiLgrDate(value);
         this.value_type = CsiOperand.value_date;
      }
      else if(value instanceof CsiOperand)
      {
         this.value = value.value;
         this.value_type = value.value_type; 
      }
      else
      {
         this.value = NaN;
         this.value_type = CsiOperand.value_double;
      }
   }
   this.timestamp = timestamp; 
};


CsiOperand.prototype.set_val_int = function(value, timestamp)
{
   this.set_val(value, timestamp);
   this.value = this.get_val_int();
   this.value_type = CsiOperand.value_int;
};


CsiOperand.prototype.valueOf = function()
{
   var rtn = NaN;
   switch(this.value_type)
   {
   case CsiOperand.value_double:
   case CsiOperand.value_int:
   case CsiOperand.value_string:
      rtn = this.value;
      break;
      
   case CsiOperand.value_date:
      rtn = this.value.milliSecs;
      break;
   }
   return rtn;
};


CsiOperand.prototype.toString = function()
{ return this.get_val_str(); };


/**
 *  
 * @param {CsiToken} value Defines a constant value which is either pre-defined or specified in the expression.
 */
function CsiConstant(value)
{
   this.set_val(value, new CsiLgrDate());
}
CsiConstant.prototype = new CsiOperand();
CsiExprToken.add_creator("NOPLOT", function() {
   return new CsiConstant(NaN);
});
CsiExprToken.add_creator("NAN", function() {
   return new CsiConstant(NaN);
});
CsiExprToken.add_creator("INF", function() {
   return new CsiConstant(Infinity);
});
CsiExprToken.add_creator("TRUE", function() {
   return new CsiConstant(-1);
});
CsiExprToken.add_creator("FALSE", function() {
   return new CsiConstant(0);
});
CsiExprToken.add_creator("PI", function() {
   return new CsiConstant(3.14159265359);
});
CsiExprToken.add_creator("e", function() {
   return new CsiConstant(2.718282);
});
CsiExprToken.add_creator("nsecPerUSec", function() {
   return new CsiConstant(CsiLgrDate.nsecPerUSec);
});
CsiExprToken.add_creator("nsecPerMSec", function() {
   return new CsiConstant(CsiLgrDate.nsecPerMSec);
});
CsiExprToken.add_creator("nsecPerMin", function() {
   return new CsiConstant(CsiLgrDate.nsecPerMin);
});
CsiExprToken.add_creator("nsecPerHour", function() {
   return new CsiConstant(CsiLgrDate.nsecPerHour);
});
CsiExprToken.add_creator("nsecPerDay", function() {
   return new CsiConstant(CsiLgrDate.nsecPerDay);
});
CsiExprToken.add_creator("nsecPerWeek", function() {
   return new CsiConstant(CsiLgrDate.nsecPerWeek);
});
CsiExprToken.add_creator("RESET_HOURLY", function() {
   return new CsiConstant(1);
});
CsiExprToken.add_creator("RESET_DAILY", function() {
   return new CsiConstant(2);
});
CsiExprToken.add_creator("RESET_WEEKLY", function() {
   return new CsiConstant(5);
});
CsiExprToken.add_creator("RESET_MONTHLY", function() {
   return new CsiConstant(3);
});
CsiExprToken.add_creator("RESET_YEARLY", function() {
   return new CsiConstant(4);
});
CsiExprToken.add_creator("RESET_CUSTOM", function() {
   return new CsiConstant(6);
});


CsiConstant.create_from_token = function(token)
{
   var rtn = null;
   var integer_regex = /^(\+|-)?\d+$/;
   var float_regex = /^[-+]?\d*\.?\d*([eE][-+]?\d+)?$/;
   
   if(token.length > 0)
   {
      rtn = new CsiConstant(NaN);
      if(token.charAt(0) === '&' && token.length > 2)
      {
         if(token.charAt(1) === 'h' || token.charAt(1) === 'H')
         {
            rtn.value = Number.parseInt(token.substr(2), 16);
            rtn.value_type = CsiOperand.value_int;
         }
         else if(token.charAt(1) === 'b' || token.charAt(1) === 'B')
         {
            rtn.value = Number.parseInt(token.substr(2), 2);
            rtn.value_type = CsiOperand.value_int;
         }
      }
      else if(token.charAt(0) === '$' && token.length >= 3 && token.charAt(1) === '\"' && token.charAt(token.length - 1) === '\"')
      {
         rtn.value = token.substr(2, token.length - 3);
         rtn.value_type = CsiOperand.value_string;
      }
      else if(integer_regex.test(token))
      {
         rtn.value = Number.parseInt(token);
         rtn.value_type = CsiOperand.value_int;
      }
      else if(float_regex.test(token))
      {
         rtn.value = Number.parseFloat(token);
         rtn.value_type = CsiOperand.value_double;
      }
      else
         rtn = null;
   }
   return rtn;
};


CsiConstant.prototype.is_constant = function()
{ return true; };


/**
 * Defines a variable within an expression.
 *
 * @param {string} name Specifies the name of this variable.
 */
function CsiExprVariable(name)
{
   this.name = name;
   this.has_been_set = false;
}
CsiExprVariable.prototype = new CsiOperand();

CsiExprVariable.prototype.is_variable = function()
{ return true; };

CsiExprVariable.prototype.set_val = function(value, timestamp)
{
   this.has_been_set = true;
   CsiOperand.prototype.set_val.call(this, value, timestamp);
};

CsiExprVariable.prototype.evaluate = function(stack)
{
   if(this.has_been_set)
      stack.push(this);
   else
      throw "variable " + this.name + " not set";
};

CsiExprVariable.prototype.reset = function()
{ this.has_been_set = false; };


/**
 * Defines a base class for functions.
 */
function CsiExprFunction()
{ }
CsiExprFunction.prototype = new CsiExprToken();

CsiExprFunction.prototype.get_priority = function()
{ return CsiExprToken.prec_function; };

CsiExprFunction.prototype.is_operator = function()
{ return true; };

CsiExprFunction.prototype.is_function = function()
{ return true; };


/**
 * @return {CsiToken} Returns a token generated for the specified key.
 *
 * @param {CsiToken} prev_token Specifies the token that was allocated for this expression previously.
 *
 * @param {string} name Specifies the token key.
 */
CsiExprToken.make_token = function(prev_token, name)
{
   var key = name.toUpperCase();
   var rtn = null;
   
   if(CsiExprToken.creators.hasOwnProperty(key))
      rtn = CsiExprToken.creators[key].call(prev_token, name);
   else
   {
      rtn = CsiConstant.create_from_token(name);
      if(rtn === null)
         rtn = new CsiExprVariable(name);
   }

   // we need to check for a unary operator
   if(!prev_token ||
      (prev_token.is_operator() && !prev_token.is_function()) ||
      prev_token.is_lparen() ||
      prev_token.is_semi_colon())
   {
      // we will ignore a unary plus.
      if(name === "+")
         rtn = null;
      else if(name === "-")
         rtn = new CsiNegation("-");
   }
   return rtn;
};
