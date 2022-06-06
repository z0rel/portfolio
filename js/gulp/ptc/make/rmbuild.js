const path = require('path');
const process = require('process');
const fs = require('fs');

const fancylog = require('fancy-log');
const rimraf = require('rimraf');

module.exports = function rmbuild() {
  let cwd = process.cwd();
  let pathBuild = path.join(cwd, 'build');
  if (fs.existsSync(pathBuild)) {
    fancylog(`deleting directory ${pathBuild}  ...`);
    rimraf.sync(pathBuild);
    fancylog('deleting done');
  }
}
