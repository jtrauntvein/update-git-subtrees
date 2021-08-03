/* CsiEquivalence.js

   Copyright (C) 2010, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 18 August 2011
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiEquivalence()
{ }
CsiEquivalence.prototype = new CsiExprToken();
CsiExprToken.add_creator("EQV", function() { return new CsiEquivalence(); });

CsiEquivalence.prototype.is_operator = function()
{ return true; };

CsiEquivalence.prototype.get_priority = function()
{ return CsiExprToken.prec_comparator; };

CsiEquivalence.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 2)
      throw "EQV requires two operands";
   var op2 = operands.pop();
   var op1 = operands.pop();
   var a = op1.get_val_int();
   var b = op2.get_val_int();

   operands.push(
      new CsiOperand(
         (a & b) | (~a & ~b),
         CsiLgrDate.max(op1.timestamp, op2.timestamp)));
};

