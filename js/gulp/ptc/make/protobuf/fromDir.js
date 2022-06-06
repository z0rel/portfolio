const path = require('path');
const fs = require('fs');

module.exports = function fromDir(startPath, filter) {
  let result = [];
  if (!fs.existsSync(startPath)) {
    console.log('no dir ', startPath);
    return result;
  }

  const files = fs.readdirSync(startPath);
  for (let i = 0; i < files.length; i++) {
    const filename = path.join(startPath, files[i]);
    const stat = fs.lstatSync(filename);
    if (stat.isDirectory()) {
      fromDir(filename, filter); // recurse
    } else if (filename.indexOf(filter) >= 0) {
      result.push(filename);
    }
  }
  return result;
}
