#!/usr/bin/env node
const { update_subtrees, read_subtrees, push_subtrees } = require("./update-git-subtrees");
const prompts = require("prompts");
const fs = require("fs");
const path = require("path");


async function main()
{
   const dir = process.argv[2] || process.cwd();
   const subtrees = await read_subtrees(dir);
   const dialog = [
      {
         type: "select",
         name: "operation",
         message: "Pull or Push?",
         initial: 0,
         choices: [
            { title: "Pull", value: "pull" },
            { title: "Push", value: "push" }
         ]
      },
      {
         type: (prev) => {
            return prev === "pull" ? "multiselect" : null;
         },
         name: "pull_subtrees",
         message: "Select the subtrees to pull or update",
         choices: subtrees.map((subtree) => {
            return {
               title: subtree.prefix,
               value: subtree.prefix,
               selected: true
            }
         })
      },
      {
         type: (prev) => {
            return prev === "push" ? "multiselect" : null;
         },
         name: "push_subtrees",
         message: "Select the subtrees to push to their source repos",
         choices: subtrees.reduce((accum, subtree) => {
            // we can only push from directories that already exist
            const subtree_path = path.join(dir, subtree.prefix);
            const subtree_stat = fs.statSync(subtree_path);
            if(subtree_stat.isDirectory())
               accum.push({ 
                  title: subtree.prefix, 
                  value: subtree.prefix,
                  selected: true 
               });
            return accum;
         }, [])
      }
   ];
   const response = await prompts(dialog);
   if(response.operation === "pull")
   {
      await update_subtrees({dir, subtrees: subtrees, selected: response.pull_subtrees });
   }
   else if(response.operation === "push")
   {
      await push_subtrees({ dir, subtrees: subtrees, selected: response.push_subtrees });
   }
}

main().then(() => {
   process.exit(0);
}).catch((error) => {
   console.error(`subtree error: ${error}`);
   process.exit(1);
});