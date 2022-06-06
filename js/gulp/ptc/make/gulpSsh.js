const fs = require('fs');
const GulpSSH = require('./gulpSshExec');
const genGetKey = require('./genGetKey');

module.exports = function getGulpSsh(cfgHost, cfgUser, cfgSshKey) {
  let getKey = genGetKey('');
  // console.log(1111, getKey(cfgHost), getKey(cfgUser), getKey(cfgSshKey))

  let config = {
    host: getKey(cfgHost),
    port: 22,
    username: getKey(cfgUser),
    privateKey: fs.readFileSync(getKey(cfgSshKey)),
  };

  return new GulpSSH({
    ignoreErrors: false,
    sshConfig: config,
  });
};
