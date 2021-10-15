const path = require("path");
const child_process = require("child_process");
const subtrees_file_name = ".subtrees.json";
const fs = require("fs");
const { cwd } = require("process");


/**
 * @typedef SubtreeType
 * @property {string} repo Specifies the URL for the subtree source repository.
 * @property {string} prefix Specifies the subdirectory of the project where the subtree should be pulled.
 */
/**
 * @returns {SubtreeType[]} The collection of subtree descriptions read from the .subtrees.json file.
 * @param {string?} dir Specifies the directory where the subtrees will be maintained.  
 * If not specified, will default to the current working directory.
 */
async function do_read_subtrees(dir = cwd())
{
   return require(path.join(dir, subtrees_file_name));
}

/**
 * @return {Promise} Returns a promise that, when complete, will have updated the 
 * subtrees in the current directory and, optionally, subtrees in subdirectories.
 * 
 * @param {string?} options.dir Specifies the repository directory to update.
 * @param {SubtreeType[]?} options.subtrees Optionally pecifies the descriptors for the subtrees.  
 * If null (the default), this structure will be read from the directories index file.
 * @param {string[]?} options.selected Optionally specifies the subtrees that should be operated.  
 * If null (the default), all subtrees in the index file will be selected.
 */
async function do_update_subtrees({ dir = process.cwd(), subtrees = null, selected = null })
{
   // Fill in any needed defaults
   if(subtrees === null)
      subtrees = do_read_subtrees(dir);
   if(selected === null)
   {
      selected = subtrees.map((subtree) => {
         return subtree.prefix;
      });
   }

   // we need to select all of the subtrees that match the prefixes given in selected.
   const active_trees = selected.map((prefix) => {
      let rtn = null;
      const subtree = subtrees.find((candidate) => {
         return candidate.prefix === prefix;
      });
      if(subtree)
         rtn = subtree;
      return rtn;
   });

   // now switch to the directory and work on the selected subtrees
   process.chdir(dir);
   console.info(`working in ${process.cwd()}`);
   for(let subtree of active_trees)
   {
      if(subtree !== null)
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
            });
         });
      }
   }
}

/**
 * Pushes changes in selected subtrees to their source repository.
 * @param {string?} options.dir Specifies the working directory.
 * @param {SubtreeType[]?} options.subtrees Specifies the collection of subtree descriptions.
 * @param {string[]?} options.selected Specifies the subtrees selected to be pushed.
 */
async function do_push_subtrees({ dir = process.cwd(), subtrees = null, selected = null })
{
   // we need to expand default parameters.
   if(subtrees === null)
      subtrees = do_read_subtrees(dir);
   if(selected === null)
   {
      selected = subtrees.map((subtree) => {
         return subtree.prefix;
      });
   }

   // now change to the directory and work on the selected subtrees.
   const active_trees = selected.map((prefix) => {
      let rtn = null;
      const subtree = subtrees.find((candidate) => {
         return candidate.prefix === prefix;
      });
      if(subtree)
         rtn = subtree;
      return rtn;
   });
   process.chdir(dir);
   console.info(`working in ${process.cwd()}`);
   for(let subtree of active_trees)
   {
      if(subtree !== null)
      {
         await new Promise((accept, reject) => {
            const git_proc = child_process.spawn("git", [
               "subtree",
               "push",
               `--prefix=${subtree.prefix}`,
               "--squash",
               subtree.repo,
               subtree.branch || "main"
            ]);
            git_proc.stdout.on("data", (data) => {
               console.log(data.toString());
            });
            git_proc.stderr.on("data", (data) => {
               console.error(data.toString());
            });
            git_proc.on("exit", (code) => {
               if(code === 0)
                  accept();
               else
                  reject(new Error("sit-subtree failed"));
            });
         });
      }
   }
}

module.exports = {
   read_subtrees: do_read_subtrees,
   update_subtrees: do_update_subtrees,
   push_subtrees: do_push_subtrees
}