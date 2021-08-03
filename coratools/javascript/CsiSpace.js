/* CsiSpace.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiSpace()
{ }
CsiSpace.prototype = new CsiExprFunction();
CsiExprToken.add_creator("Space", function() {return new CsiSpace(); });

CsiSpace.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 1)
      throw "Space requires one operand";
   var v1 = operands.pop();
   var rtn = new CsiOperand("", v1.timestamp);
   for(var i = 0; i < v1.value; ++i)
      rtn.value = rtn.value.concat(" ");
   operands.push(rtn);
};

