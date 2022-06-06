const pbjs = require('protobufjs/cli/pbjs');
const PROTO_FILES = require('./constants.js').PROTO_FILES;

module.exports = function buildProto(cb) {
  const outModule = path.join('src', 'assets', 'proto_compiled', 'proto.js');
  pbjs.main(['--target', 'static-module', '-w', 'es6', '-o', outModule, ...PROTO_FILES], (err, output) => {
    if (err) throw err;
    cb();
    // do something with output
  });
};
