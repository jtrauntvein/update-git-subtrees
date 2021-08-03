/* CsiRandom.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Wednesday 09 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiRandom()
{ }
CsiRandom.prototype = new CsiExprFunction();
CsiExprToken.add_creator("RND", function() { return new CsiRandom(); });

CsiRandom.prototype.evaluate = function(operands, tokens)
{
   operands.push(new CsiOperand(Math.random(), new CsiLgrDate()));
};

