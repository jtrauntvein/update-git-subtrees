"use strict";
module.exports = {
   reporter: function(res)
   {
      var str;
      var file;
      var error;
      res.forEach(function(r) {
         file = r.file;
         error = r.error;
         process.stderr.write(file + ":" + error.line + ": (" + error.character + ") " + error.reason + "\n");
      });
   }
};
