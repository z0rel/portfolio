const os = require('os');
const spawn = require('child_process').spawn;

const gulp = require('gulp');
const fancylog = require('fancy-log');

const setDisplayName = require('./setDisplayName');
const rmbuild = require('./rmbuild');

module.exports = function buildProduction(alldone, env = undefined) {
  gulp.series(
    setDisplayName((cb) => {
      rmbuild();
      fancylog('npx react-scripts build');

      let cmd = os.platform() === 'win32' ? 'npx.cmd' : 'npx';

      const npx = env ? spawn(cmd, ['react-scripts', 'build'], { env: env }) : spawn(cmd, ['react-scripts', 'build']);

      npx.stdout.on('data', (data) => {
        fancylog(data.toString());
      });

      npx.stderr.on('data', (data) => {
        fancylog.error(data.toString());
      });

      npx.on('error', (error) => {
        fancylog.error(error);
      });

      npx.on('close', (code) => {
        fancylog(`return code: ${code}`);
        cb();
      });
    }, 'build production'),
    setDisplayName((done) => {
      done();
      alldone();
    }, 'build production done'),
  )();
}
