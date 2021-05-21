/* CsiConstant.js

   Copyright (C) 2010, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Monday 28 March 2011
   Last Commit: $Date: 2011-08-22 10:25:18 -0600 (Mon, 22 Aug 2011) $
   Last Changed by: $Author: ken $

*/

////////////////////////////////////////////////////////////
// class CsiConstant
//
// Defines a constant value in an expression
////////////////////////////////////////////////////////////
function CsiConstant(value)
{ this.set_val(value, new CsiLgrDate()); }
CsiConstant.prototype = new CsiOperand();


CsiConstant.prototype.evaluate = function(operands, token)
{ operands.push(new CsiOperand(this)); };



