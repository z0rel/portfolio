const path = require('path');
const gulp = require('gulp');
const rename = require('gulp-rename');

module.exports = function copyProto() {
  const srcPath = path.join('..', 'django', 'ptc_deco', 'api', 'api_gql', 'queries', 'proto', '**.proto');
  return gulp
    .src(srcPath)
    .pipe(rename({ dirname: '' }))
    .pipe(gulp.dest(path.join('src', 'assets', 'proto')));
};
