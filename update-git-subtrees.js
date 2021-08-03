const path = require("path");
const read_dir = require("recursive-readdir");
const child_process = require("child-process");


/**
 * @return {object[]} Returns the list of subtree index files to process.
 * @param {boolean} options.recursive Set to true if subdirectories should be searched
 * @param {string} options.dir Specifies the repository directory to update.
 */
async function build_subtree_index({ recursive, dir })
{
   let subtree_files = [ path.join(dir, ".subtrees.json") ];
   if(recursive)
   {
      subtree_files = await read_dir(dir, (candidate) => {
         return candidate !== ".subtrees.json";
      });
   }
   return subtree_files.map((subtree_file) => {
      const rtn = require(subtree_file);
      rtn.dir = path.dirname(subtree_file);
      return rtn;
   });
}

/**
 * @return {Promise} Returns a promise that, when complete, will have updated the 
 * subtrees in the current directory and, optionally, subtrees in subdirectories.
 * 
 * @param {boolean} options.recursive Set to true if the child subtrees should also be updated.
 * @param {string} options.dir Specifies the repository directory to update.
 */
async function do_update_subtrees({ recursive, dir })
{
   // We need to put together a collection of all of the files that will be processed.
   const files = build_subtree_index({ recursive, dir });
   const promises = files.reverse().reduce((accum, file) => {
      accum.push(new Promise((accept, reject) => {
         const git_proc = child_process.spawn("git", [ 
            "subtree", 
            "pull", 
            `--prefix=${path.join(file.dir, file.prefix)}`, 
            file.repo, 
            "HEAD",
            "--squash"]);
         git_proc.stdout.on("data", (data) => {
            console.log(data);
         });
         git_proc.stderr.on("data", (data) => {
            console.error(data);
         });
         git_proc.on("exit", (code) => {
            if(code == 0)
               accept();
            else
               reject(new Error("git-subtree failed"));
         })
      }));
   }, []);
   await Promise.all(promises);
}

module.exports = do_update_subtrees;