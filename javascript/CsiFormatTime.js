/* CsiFormatTime.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiFormatTime()
{ }
CsiFormatTime.prototype = new CsiExprFunction();
CsiExprToken.add_creator("FormatTime", function(){return new CsiFormatTime();});

CsiFormatTime.prototype.evaluate = function(operands, tokens)
{
   var op2 = operands.pop();
   var op1 = operands.pop();
   var format = op2.get_val_str();
   var stamp = op1.get_val_date();
   operands.push(new CsiOperand(stamp.format(format), op1.timestamp));
};

