const path = require('path')
const gulp = require('gulp')

const genGetKey = require('./genGetKey');
const setDisplayName = require('./setDisplayName')
const rsync = require('./rsync')

const changePermissions = require('./changePermissions');

module.exports = function deployProductionRsync(cb, genkeySpec, cfgUser, cfgWebUser, cfgHost, cfgSshKey) {
  let getKey = genGetKey(genkeySpec);
  let srcPath = path.join('build', '**');
  let hostName = `${getKey('DEPLOY_SFTP_USER')}@${getKey('DEPLOY_URL')}`;
  let dest = getKey('DEPLOY_SFTP_DIRECTORY');
  let dest_backend = getKey('DEPLOY_SFTP_BACKEND_DIRECTORY') + '/static';
  console.log('src', srcPath, 'host', hostName, 'dst', dest);

  let rsync_args = {
    // exclude: config.deploy.exclude_html, // Excludes files from deploy
    hostname: hostName,
    recursive: true,
    archive: true,
    silent: false,
    compress: true,
    clean: true,
    omit_dir_times: true,
    no_perms: true,
  };

  if (process.platform === 'win32') {
    let RSYNC_WIN_CMD = getKey('RSYNC_WIN_CMD');
    let RSYNC_WIN_CMD_SSH = '"' + getKey('RSYNC_WIN_CMD_SSH') + '"';
    let RSYNC_WIN_CWRSYNCHOME = getKey('RSYNC_WIN_CWRSYNCHOME');

    process.env.CWRSYNCHOME = RSYNC_WIN_CWRSYNCHOME;
    process.env.CWOLDPATH = process.env.PATH;
    process.env.PATH = process.env.CWRSYNCHOME + '\\bin;' + process.env.PATH;

    rsync_args.shell = RSYNC_WIN_CMD_SSH;
    rsync_args.rsync_cmd = RSYNC_WIN_CMD;
  }

  return gulp.series(
    setDisplayName(
      () => gulp.src(srcPath).pipe(rsync({ root: 'build', destination: dest, ...rsync_args })),
      `rsync ${srcPath}, local root: build`,
    ),
    setDisplayName(
      () =>
        gulp
          .src(path.join('build', 'static', 'media', '**'))
          .pipe(rsync({ root: path.join('build', 'static'), destination: dest_backend, ...rsync_args })),
      'rsync build/static/media/**, local root: build/static',
    ),
    setDisplayName(
      () =>
        gulp
          .src(path.join('build', 'static', 'css', '**'))
          .pipe(rsync({ root: path.join('build', 'static'), destination: dest_backend, ...rsync_args })),
      'rsync build/static/css/**, local root: build/static',
    ),
    setDisplayName(
      () =>
        gulp
          .src(path.join('build', 'static', 'js', '**'))
          .pipe(rsync({ root: path.join('build', 'static'), destination: dest_backend, ...rsync_args })),
      'rsync build/static/js/**, local root: build/static',
    ),
    setDisplayName((cb2) => {
      changePermissions(cb2, cfgUser, cfgWebUser, cfgHost, cfgSshKey);
    }, 'ssh change permissions'),
    setDisplayName((done) => {
      done();
      cb();
    }, 'rsync done'),
  )();
};
