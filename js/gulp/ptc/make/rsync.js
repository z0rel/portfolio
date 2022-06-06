const path = require('path');
const fancylog = require("fancy-log");
const assert = require('better-assert');
const every = require('lodash.every');
const isString = require('lodash.isstring');
const spawn = require('child_process').spawn;
const PluginError = require('plugin-error');
const through = require('through2');
const util = require('util');


class RsyncCmd {
  constructor(config) {
    assert(typeof config === 'object');
    assert(!config.options || typeof config.options === 'object');
    this._options = config.options || {};
    let sources = config.source;
    if (!Array.isArray(sources)) {
      sources = [sources];
    }
    assert(sources.length > 0 && every(sources, isString));
    assert(sources.length === 1 || config.destination);
    this._sources = sources;
    assert(!config.destination || typeof config.destination === 'string');
    this._destination = config.destination;
    assert(!config.cwd || typeof config.cwd === 'string');
    this._cwd = config.cwd;
    assert(!config.stdoutHandler || typeof config.stdoutHandler === 'function');
    this._stdout = config.stdoutHandler;
    assert(!config.stderrHandler || typeof config.stderrHandler === 'function');
    this._stderr = config.stderrHandler;
    this._rsync_cmd = config.rsync_cmd || 'rsync'
  }

  command() {
    let args = [];

    let shortOptions = [];
    let longOptions = [];

    for (let key in this._options) {
      let value = this._options[key];
      if (typeof value !== 'undefined' && value !== false) {
        if (key.length === 1 && value === true) {
          shortOptions.push(key);
        } else {
          let values = Array.isArray(value) ? value : [value];
          for (let i = 0, l = values.length; i < l; i++) {
            longOptions.push({key: key, value: values[i]});
          }
        }
      }
    }

    if (shortOptions.length > 0) {
      args.push('-' + shortOptions.join(''));
    }

    // "include" -argument must be applied before "exclude"
    longOptions.sort(function (a, b) {
      if (a.key === 'include') {
        return -1;
      }
      if (b.key === 'include') {
        return 1;
      }
      return 0;
    })

    if (longOptions.length > 0) {
      args = args.concat(longOptions.map(function(option) {
        let single = option.key.length === 1;
        let output = (single ? '-' : '--') + option.key;
        if (typeof option.value !== 'boolean') {
          if (option.key !== 'e')
            output += (single ? ' ' : '=') + escapeShellArg(option.value);
          else
            output += (single ? ' ' : '=') + option.value;
        }
        return output;
      }));
    }

    args = args.concat(this._sources.map(escapeShellArg));

    if (this._destination) {
      args.push(escapeShellArg(this._destination));
    }

    return this._rsync_cmd + ' ' + args.join(' ');
  }

  execute(callback) {
    let command = this.command();
    // console.log(command)

    let childProcess;
    if (process.platform === 'win32') {
      let q = ``;
      childProcess = spawn('cmd.exe', ['/s', '/c', q + command + q], {
        cwd: this._cwd,
        stdio: [process.stdin, 'pipe', 'pipe'],
        env: process.env,
        windowsVerbatimArguments: true
      });
    } else {
      childProcess = spawn('/bin/sh', ['-c', command], {
        cwd: this._cwd,
        stdio: 'pipe',
        env: process.env
      });
    }

    if (this._stdout) {
      childProcess.stdout.on('data', this._stdout);
    }

    if (this._stderr) {
      childProcess.stderr.on('data', this._stderr);
    }

    childProcess.on('close', function(code) {
      let error = null;
      if (code !== 0) {
        error = new Error('rsync exited with code ' + code);
      }

      if (typeof callback === 'function') {
        callback(error, command);
      }
    });

    return childProcess;
  }
}

function escapeShellArg(arg) {
  if (!/(["'`\\\(\) ])/.test(arg)) {
    return arg;
  }
  arg = arg.replace(/([\\])/g, '/');
  return '"' + arg.replace(/(["'`\\])/g, '\\$1') + '"';
}



function log() {
  function _log() {
      process.stdout.write(util.format.apply(this, arguments));
  }
  // HACK: In order to show rsync's transfer progress, override `console` temporarily...
  if (process.platform === 'win32') {
    console.log(util.format.apply(this, ['[', new Date().toLocaleString(), ']', ...arguments]).trimEnd());
  }
  else {
    let orig = console.log;
    console.log = _log;
    let retval = fancylog.apply(this, arguments);
    console.log = orig;
    return retval;
  }
}

module.exports = function(options) {

  let sources = [];

  let cwd = options.root ? path.resolve(options.root) : process.cwd();

  return through.obj(
    function(file, enc, cb) {
      if (file.isStream()) {
        this.emit(
          'error',
          new PluginError('gulp-rsync', 'Streams are not supported!')
        );
      }

      if (path.relative(cwd, file.path).indexOf('..') === 0) {
        this.emit(
          'error',
          new PluginError('gulp-rsync', 'Source contains paths outside of root')
        );
      }

      sources.push(file);
      cb(null, file);
    },
    function(cb) {
      sources = sources.filter(function(source) {
        return !source.isNull() ||
          options.emptyDirectories ||
          (source.path === cwd && options.recursive);
      });

      if (sources.length === 0) {
        cb();
        return;
      }

      let shell = options.shell;
      if (options.port) {
        shell = 'ssh -p ' + options.port;
      }

      let destination = options.destination;
      if (options.hostname) {
        destination = options.hostname + ':' + destination;
        if (options.username) {
          destination = options.username + '@' + destination;
        }
      } else {
        destination = path.relative(cwd, path.resolve(process.cwd(), destination));
      }

      let config = {
        options: {
          'a': options.archive,
          'n': options.dryrun,
          'R': options.relative !== false,
          'c': options.incremental,
          'd': options.emptyDirectories,
          'e': shell,
          'r': options.recursive && !options.archive,
          't': options.times && !options.archive,
          'u': options.update,
          'v': !options.silent,
          'z': options.compress,
          'omit-dir-times': options.omit_dir_times,
          'no-perms': options.no_perms,
          'chmod': options.chmod,
          'chown': options.chown,
          'exclude': options.exclude,
          'include': options.include,
          'progress': options.progress,
          'links': options.links,
        },
        rsync_cmd: options.rsync_cmd,
        source: sources.map(function(source) {
          return path.relative(cwd, source.path) || '.';
        }),
        destination: destination,
        cwd: cwd
      };

      if (options.options) {
        for (let key in options.options) { config.options[key] = options.options[key]; }
      }

      if (options.clean) {
        if (!options.recursive && !options.archive) {
          this.emit(
            'error',
            new PluginError('gulp-rsync', 'clean requires recursive or archive option')
          );
        }
        config.options['delete'] = true;
      }

      if (!options.silent) {
        let handler = function(data) {
          data.toString().split('\r').forEach(function(chunk) {
            chunk.split('\n').forEach(function(line, j, lines) {
              log('gulp-rsync:', line, (j < lines.length - 1 ? '\n' : ''));
            });
          });
        };
        config.stdoutHandler = handler;
        config.stderrHandler = handler;

        log('gulp-rsync:', 'Starting rsync to ' + destination + '...');
      }

      (new RsyncCmd(config)).execute(function(error, command) {
        if (error) {
          this.emit('error', new PluginError('gulp-rsync', error.stack));
        }
        if (options.command) {
          log(command);
        }
        if (!options.silent) {
          log('gulp-rsync:', 'Completed rsync.');
        }
        cb();
      }.bind(this));
    }
  );
}
