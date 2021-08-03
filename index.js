#!/usr/bin/env node
const update_subtrees = require("./update-git-subtrees");
const args_parser = require("command-line-args");

const option_defs = [
   { name: "recursive", alias: "R", type: Boolean, defaultOption: true },
   { name: "dir", alias: "d", type: String, defaultOption: process.cwd() }
];

async function main()
{
   const options = args_parser(option_defs);
   await update_subtrees(options);
}

main().then(() => {
   process.exit(0);
}).catch((error) => {
   console.error(`subtree error: ${error}`);
   process.exit(1);
});