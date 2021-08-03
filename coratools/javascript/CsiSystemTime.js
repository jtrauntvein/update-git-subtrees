/* CsiSystemTime.js

   Copyright (C) 2011, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 02 February 2011
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiSystemTime()
{ }
CsiSystemTime.prototype = new CsiExprFunction();
CsiExprToken.add_creator("SystemTime", function(){return new CsiSystemTime();});

CsiSystemTime.prototype.evaluate = function(operands, tokens)
{
   var rtn = CsiLgrDate.local();
   operands.push(new CsiOperand(rtn, rtn));
};



