/* generate-scripts.js

   Copyright (C) 2018, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 05 April 2018
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

var fs = require("fs");
var path = require("path");
var dependencies = null;


/**
 * Loads the dependencies for modules in this library.
 *
 * @param on_complete Specifies the function to invoke when the dependencies have been loaded.
 * This function will be passed an error (null on sucess) and the dependencies that were loaded.
 */
function get_dependencies(on_complete)
{
   var dependencies_path = path.join(__dirname, "dependencies.json");
   if(dependencies !== null)
      on_complete(null, dependencies);
   else
   {
      fs.readFile(dependencies_path, function(error, content) {
         if(error)
            on_complete(error, null);
         else
         {
            dependencies = JSON.parse(content);
            on_complete(null, dependencies);
         }
      });
   }
}
 

/**
 * Generates the specified file with all of its required dependencies.
 *
 * @param {string} output Specifies the name of the destination file.
 *
 * @param {array} inputs Specifies the collection of input file names.
 *
 * @param {function} on_complete Specifies the completion function that will
 * be called when the file has been completed.  This function will be called
 * with one parameter, and error, that will be null if the operation succeeded.
 */
function generate_file(output, inputs, on_complete)
{
   var items = [];
   var picked = {};
   var pick_error = null;
   var do_pick_items = function(input) {
      var input_depends;
      
      if(!picked.hasOwnProperty(input))
      {
         if(dependencies.hasOwnProperty(input))
         {
            input_depends = dependencies[input].depends;
            if(input_depends !== undefined)
            {
               picked[input] = true;
               input_depends.every(function(input_dependency) {
                  return do_pick_items(input_dependency);
               });
               items.push(input);
            }
         }
         else
            pick_error = "undefined dependency: " + input;
      }
      return pick_error === null;
   };
   var output_stream;
   var do_next_item = function() {
      var item, item_path;
      if(items.length > 0)
      {
         if(!output_stream)
         {
            output_stream = fs.createWriteStream(output);
            output_stream.on("error", function(write_error) {
               on_complete(write_error);
            });
         }
         item = items.shift();
         item_path = path.join(__dirname, item);
         fs.readFile(item_path, function(item_error, item_content) {
            if(item_error)
               on_complete(item_error);
            else
            {
               output_stream.write(item_content);
               do_next_item();
            }
         });
      }
      else
         on_complete(null);
   };

   get_dependencies(function(depends_error, dependencies) {
      if(!depends_error)
      {
         if(inputs.every(do_pick_items))
            do_next_item();
         else
            on_complete(pick_error);
      }
      else
         on_complete(depends_error);
   });
}


/**
 * Generates a file using the specified array of category names to select the sources that will be included.
 *
 * @param {string} output Specifies the path of the output file.
 *
 * @param {array} categories Specifies the categories that will be included in the output file.
 *
 * @param {function} on_complete Specifies the callback when the generation is complete.
 */
function generate_categories(output, categories, on_complete)
{
   get_dependencies(function(depends_error, dependencies) {
      var inputs = [];   
      var keys;
      if(!depends_error)
      {
         // we need to select the inputs that will be generated.
         keys = Object.keys(dependencies);
         keys.forEach(function(key) {
            var source = dependencies[key];
            var source_has_category = false;
            if(source.hasOwnProperty("categories"))
            {
               categories.some(function(category) {
                  var category_index = source.categories.indexOf(category);
                  if(category_index >= 0)
                     source_has_category = true;                 
                  return source_has_category;
               });
            }
            if(source_has_category)
               inputs.push(key);
         });
         generate_file(output, inputs, on_complete);
      }
      else
         on_complete(depends_error);
   });
}


module.exports.generate_file = generate_file;
module.exports.get_dependencies = get_dependencies;
module.exports.generate_categories = generate_categories;
