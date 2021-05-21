/* CsiRound.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiRound()
{ }
CsiRound.prototype = new CsiExprFunction();
CsiExprToken.add_creator("Round", function(){return new CsiRound();});

CsiRound.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 2)
      throw "Round requires two operands";
   var op2 = operands.pop();
   var op1 = operands.pop();
   var power = 0;
   var places = op2.get_val_int();
   var val = op1.get_val();
   var max_power_ten = 8;
   var power_ten = [
      1,
      10,
      100,
      1000,
      10000,
      100000,
      1000000,
      10000000,
      100000000
   ];
   var rtn = new CsiOperand(0, op1.timestamp);
   if(places > 0)
   {
      if(places < max_power_ten)
      {
         power = power_ten[places];
         rtn.value = Math.floor(Math.abs(val) * power + 0.5) / power;
      }
      else
      {
         rtn.value = val;
      }
   }
   else if(places === 0)
   {
      rtn.value = Math.floor(Math.abs(val) + 0.5);
   }
   else
   {
      if(places > -max_power_ten)
      {
         power = power_ten[-places];
         rtn.value = Math.floor(Math.abs(val) / power + 0.5) * power;
      }
      else
      {
         rtn.value = val;
      }
   }
   if(val < 0 && rtn.value > 0)
      rtn.value = -rtn.value;
   operands.push(rtn);
};

