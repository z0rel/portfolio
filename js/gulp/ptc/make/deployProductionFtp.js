const gulp = require('gulp');
const fancylog = require('fancy-log');
const ftp = require('vinyl-ftp');

const genGetKey = require('./genGetKey');

module.exports = function deployProductionFtp(cb) {
  let getKey = genGetKey();

  let conn = ftp.create({
    host: getKey('DEPLOY_URL'),
    user: getKey('DEPLOY_LOGIN'),
    password: getKey('DEPLOY_PASWORD'),
    parallel: 10,
    log: (data) => fancylog(data),
  });

  return gulp.series(
    () =>
      gulp
        .src(path.join('.', 'build', '**'), {
          buffer: false,
          base: path.join('.', 'build'),
        })
        .pipe(conn.dest(getKey('DEPLOY_FOLDER'))),
    () => {
      return gulp
        .src(path.join('.', 'build', 'static', '**'), {
          buffer: false,
          base: path.join('.', 'build'),
        })
        .pipe(conn.dest(getKey('DEPLOY_BACKEND_FOLDER')));
    },
    (done) => {
      done();
      cb();
    },
  )();
};
