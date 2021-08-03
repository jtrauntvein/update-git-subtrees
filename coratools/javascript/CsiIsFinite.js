/* CsiIsFinite.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiIsFinite()
{ }
CsiIsFinite.prototype = new CsiExprFunction();
CsiExprToken.add_creator("IsFinite", function() { return new CsiIsFinite(); });

CsiIsFinite.prototype.evaluate = function(operands, stack)
{
   if(operands.length < 1)
      throw "IsFinite requires one parameter";
   var op1 = operands.pop();
   var v1 = op1.get_val();
   var rtn = -1;
   if(v1 == -Infinity || v1 == Infinity || isNaN(v1))
      rtn = 0;
   operands.push(new CsiOperand(rtn, op1.timestamp));
};

