const process = require('process');

const gulp = require('gulp');

const buildProto = require('./make/protobuf/buildProto');
const describe = require('./make/describe');
const setDisplayName = require('./make/setDisplayName');
const generateAllDone = require('./make/generateAllDone');
const buildProduction = require('./make/buildProduction');
const deployProductionFtp = require('./make/deployProductionFtp');
const deployProductionRsync = require('./make/deployProductionRsync');
const genGetKey = require('./make/genGetKey');
const copyProto = require('./make/protobuf/copyProto');

gulp.task('proto_copy', describe(copyProto, 'Скопировать protobuf файлы из исходников бекэнда'));

gulp.task(
  'proto_compile',
  describe((cb) => {
    return buildProto(cb);
  }, 'Собрать JavaScript интерфейсы для protobuf файлов'),
);

gulp.task(
  'proto',
  describe((cb) => {
    return gulp.series('proto_copy', 'proto_compile', setDisplayName(generateAllDone(cb), 'proto task done'))();
  }, 'Скопировать protobuf файлы из исходников бекэнда и собрать из них JavaScript интерфейсы'),
);

gulp.task(
  'build',
  describe((cb) => {
    return buildProduction(cb);
  }, 'Собрать бандл приложения'),
);

gulp.task(
  'deploy',
  describe((cb) => {
    return deployProductionFtp(cb);
  }, 'Развернуть бандл приложения на сервере по FTP'),
);

gulp.task(
  'sdeploy',
  describe((cb) => {
    return deployProductionRsync(cb, '', 'TEST_USER', 'TEST_WEB_USER', 'TEST_HOST', 'SSH_KEY');
  }, 'Развернуть бандл приложения на сервере по SSH через Rsync'),
);

gulp.task(
  'bdeploy',
  describe((cb) => {
    return gulp.series('build', 'deploy', setDisplayName(generateAllDone(cb), 'bdeploy done'))();
  }, 'Собрать бандл приложения и развернуть его на сервере по FTP'),
);

gulp.task(
  'bsdeploy',
  describe((cb) => {
    return gulp.series('build', 'sdeploy', setDisplayName(generateAllDone(cb), 'bsdeploy done'))();
  }, 'Собрать бандл приложения и развернуть его на сервере по SSH через Rsync'),
);

gulp.task(
  'build-rts',
  describe((cb) => {
    let rtsdecauxConfig = '.env.production.local.rts';
    return gulp.series(
      setDisplayName((done) => {
        let getKey = genGetKey(rtsdecauxConfig);
        console.log(getKey('REACT_APP_BACKEND_URL'));
        buildProduction(done, { ...process.env, REACT_APP_BACKEND_URL: getKey('REACT_APP_BACKEND_URL') });
      }, 'build rts'),
      setDisplayName(generateAllDone(cb), 'build-rtsdone'),
    )();
  }, 'Собрать бандл приложения для сервера rts'),
);

gulp.task(
  'bsdeploy-rts',
  describe((cb) => {
    return gulp.series(
      'build-rts',
      'sdeploy-rts',
      setDisplayName(generateAllDone(cb), 'bsdeploy-rtsdone'),
    )();
  }, 'Собрать бандл приложения и развернуть его на сервере по SSH через Rsync на rts'),
);


gulp.task(
  'sdeploy-rts',
  describe((cb) => {
    let rtsdecauxConfig = '.env.production.local.rts';
    return gulp.series(
      setDisplayName((done) => {
        deployProductionRsync(done, rtsdecauxConfig, 'RTSDECAUX_USER', 'RTSDECAUX_WEB_USER', 'RTSDECAUX_HOST', 'SSH_KEY');
      }, 'sdeploy rts'),
      setDisplayName(generateAllDone(cb), 'sdeploy-rtsdone'),
    )();
  }, 'Собрать бандл приложения и развернуть его на сервере по SSH через Rsync на rts'),
);

gulp.task(
  'bsdeploy-all',
  describe((cb) => {
    return gulp.series('bsdeploy', 'bsdeploy-rts', setDisplayName(generateAllDone(cb), 'bsdeploy-all done'))();
  }, 'Собрать бандл приложения и развернуть его на всех серверах сервере по SSH через Rsync'),
);

gulp.task(
  'clean',
  describe((cb) => {
    rmbuild();
    cb();
  }, 'Удалить каталог build'),
);
