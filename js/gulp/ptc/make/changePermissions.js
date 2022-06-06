const gulp = require('gulp');

const getGulpSsh = require('./gulpSsh');
const genGetKey = require('./genGetKey');
const generateAllDone = require('./generateAllDone');

module.exports = function changePermissions(cb2, cfgUser, cfgWebUser, cfgHost, cfgSshKey) {
  let getKey = genGetKey('');
  const webuser = getKey(cfgWebUser);
  const gulpSSHRtsdecaux = getGulpSsh(cfgHost, cfgUser, cfgSshKey);
  gulp.series(
    cb3 => gulpSSHRtsdecaux.exec(cb3, [`chown -R ${webuser}:${webuser} /opt/ptc/static/ /opt/ptc_frontend/app/`]),
    generateAllDone(cb2)
  )();
}
