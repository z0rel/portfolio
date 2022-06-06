const path = require('path');
const fromDir = require('./fromDir')

module.exports.PROTO_PATH = path.join('src', 'assets', 'proto');
module.exports.PROTO_FILES = fromDir(module.exports.PROTO_PATH, '.proto');
