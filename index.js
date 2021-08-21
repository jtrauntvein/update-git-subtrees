#!/usr/bin/env node
const update_subtrees = require("./update-git-subtrees");

async function main()
{
   await update_subtrees(process.argv[2] || process.cwd());
}

main().then(() => {
   process.exit(0);
}).catch((error) => {
   console.error(`subtree error: ${error}`);
   process.exit(1);
});