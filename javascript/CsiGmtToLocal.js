/* CsiGmtToLocal.js

   Copyright (C) 2013, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 19 December 2013
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/


function CsiGmtToLocal()
{ }
CsiGmtToLocal.prototype = new CsiExprFunction();
CsiExprToken.add_creator("GmtToLocal", function(){return new CsiGmtToLocal();});

CsiGmtToLocal.prototype.evaluate = function(operands, tokens)
{
   var arg = operands.pop();
   var rtn = new CsiOperand();
   var stamp = arg.get_val_date();
   stamp.milliSecs += stamp.gmt_offset();
   rtn.set_val(stamp, stamp);
   operands.push(rtn);
};


