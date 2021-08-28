const path = require("path");
const child_process = require("child_process");
const subtrees_file_name = ".subtrees.json";
const fs = require("fs");


/**
 * @return {Promise} Returns a promise that, when complete, will have updated the 
 * subtrees in the current directory and, optionally, subtrees in subdirectories.
 * 
 * @param {string} dir Specifies the repository directory to update.
 */
async function do_update_subtrees(dir)
{
   // We need to put together a collection of all of the files that will be processed.
   const subtrees = require(path.join(dir, subtrees_file_name));
   process.chdir(dir);
   console.info(`working in ${process.cwd()}`);
   for(let subtree of subtrees)
   {
      await new Promise((accept, reject) => {
         let prefix_stat = undefined;
         try {
            prefix_stat = fs.statSync(subtree.prefix, { throwIfNoEntry: false });
         }
         catch(err) { 
            prefix_stat = undefined;
         }
         const op = (prefix_stat && prefix_stat.isDirectory() ? "pull" : "add");
         const git_proc = child_process.spawn("git", [ 
            "subtree", 
            op, 
            `--prefix=${subtree.prefix}`,
            "--squash",
            subtree.repo, 
            subtree.branch || "HEAD"
         ]);
         git_proc.stdout.on("data", (data) => {
            console.log(data.toString());
         });
         git_proc.stderr.on("data", (data) => {
            console.error(data.toString());
         });
         git_proc.on("exit", (code) => {
            if(code == 0)
               accept();
            else
               reject(new Error("git-subtree failed"));
         })
      });
   }
}

module.exports = do_update_subtrees;