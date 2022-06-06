const gulp             = require('gulp'),
      svgmin           = require('gulp-svgmin'),
      svgsprite        = require('gulp-svg-sprites'),
      raster           = require('gulp-raster'),
      rename           = require('gulp-rename'),
      responsive       = require('gulp-responsive'),
      Stream           = require('stream'),
      xml2js           = require('xml2js'),
      isFunction       = require('lodash.isfunction'),
      isObject         = require('lodash.isobject'),
      assign           = require('object-assign'),
      PluginError      = require('plugin-error'),
      path             = require('path'),
      size             = require('gulp-filesize'),
      cache            = require('gulp-cache'),
      imagemin         = require('gulp-imagemin'),
      imageminPngquant = require('imagemin-pngquant'),
      imageminZopfli   = require('imagemin-zopfli'),
      imageminMozjpeg  = require('imagemin-mozjpeg'), // need to run 'brew install libpng'
      argv             = require('yargs').argv,
      imageminGiflossy = require('imagemin-giflossy'),
      GulpSSH          = require('gulp-ssh'),
      rsync            = require('gulp-rsync'),
      process          = require('process'),
      fs               = require('fs'),
      rimraf           = require("rimraf"),
      shell            = require("gulp-shell"),
      webp             = require("gulp-webp"),
      fancylog         = require("fancy-log"),
      through2         = require('through2'),
      sizeOf           = require('image-size'),
      filter           = require('gulp-filter');

const config           = require('./config');

g = module.exports;

const NC='\033[0m';
const BrownOrange  = '\033[0;33m';     

const config_ssh = {
  host: '192.168.0.116',
  username: 'vbox',
  password: 'at89c51'
};
var gulpSSH = new GulpSSH({
  ignoreErrors: true,
  sshConfig: config_ssh
});


g.svg_sprite_build =  function (cb) {
    let mapSprites = [ [config.src_dir + "/img/sprites_src/main/*.svg", "main"],
                       [config.src_dir + "/img/sprites_src/internal/*.svg", "internal"],
                       [config.src_dir + "/img/sprites_src/internal-min/*.svg", "internal-min"]
                     ];
    svgmin_opts = [ 
        { cleanupNumericValues: { floatPrecision: 2 } },
        { removeHiddenElems: false },
        { convertTransform: { floatPrecision: 2 } } 
    ]
    let arr = [];
    for (let [src, dst] of mapSprites) {
        arr.push(function() {
            return gulp.src(src)
                       .pipe(svgmin(svgmin_opts))
                       .pipe(svgsprite({
                               mode: "symbols",
                                 preview: false,
                                 selector: "s-ic-%f",
                                 svg: {
                                     symbols: dst + '.svg'
                                 }
                            }))
                       .pipe(gulp.dest(config.src_dir + "/_includes/build"))
                       .pipe(gulp.dest(config.src_dir + "/img"));
        });
    }
    gulp.series(...arr, function(done) { cb(); done(); })();
};




const xmlEdit = function(transform, options) {
    const stream = new Stream.Transform({ objectMode: true });

    stream._transform = function(file, unused, done) {
        const that = this;

        if (file.isNull()) {
            return done(null, file);
        }

        if (file.isStream()) {
            return done(new PluginError('svg2raster', 'Streaming not supported'));
        }

        const content = file.contents.toString('utf-8');
        const parser  = new xml2js.Parser({});
        const builder = new xml2js.Builder({ headless: true, renderOpts: { pretty: false } });

        parser.parseString(content, function(err, data) {
            let content = transform.call(null, data, file);

            if (!isObject(content)) {
                done(new PluginError('gulp-xml-edit', 'transformation does not returns an object'));
                return;
            }

            content = builder.buildObject(content);
            file.contents = new Buffer(content);

            return done(null, file);
        });
    };

    return stream;
};



g.svg2raster = function() {
    let white_files = [ 'edit.svg', 'mail.svg', 'person.svg', 'phone.svg', 'vk-circle.svg' ];
    let white_files_k = {};
    for (let o of white_files) {
        white_files_k[o] = 1;
    }

    function svg_modificator(content, file) {
        let filename = path.basename(file.path);
        let base_color_fill = '#4C82A4';
        let white_color = '#FCFCFC';
        if (filename in white_files_k) {
            console.log(filename)
            content.svg.$.fill = white_color;
        }
        else {
            content.svg.$.fill = base_color_fill;
        }

        return content;
    };


    return gulp.src(config.src_dir + '/img/sprites_src/icons/**/*.svg')
        .pipe(xmlEdit(svg_modificator))
        .pipe(raster())
        .pipe(rename({extname: '.png'}))
        .pipe(gulp.dest(config.src_dir + '/img/sprites_png'))
        .pipe(gulp.dest(config.dst_dir + '/img/sprites_png'));
        // .pipe(size());
};

var rxp = new RegExp(String.fromCodePoint(1080, 774), 'g')
function transformerYjpg(path) {
    path.extname = '.jpg'
    path.basename = path.basename.replace(rxp, "й").toLowerCase();
    // return fileObj.hash
    // b'\xd0\xb8'
}


g.resize_big_portfolio = function() {
    // --portfolio_item=dir --width=int


    let portfolio_item = argv.portfolio_item;
    let width = parseInt(argv.width);
    let srcDir = config.src_dir + "/img/portfolio_big/" + portfolio_item + "/*.{jpg,png}";
    const widthFilter = filter(f => {
        let dimensions = sizeOf(f.path)
        console.log(f.path, dimensions.width, dimensions.height)
        return dimensions.width > width;
    }, {restore: true})
    
    console.log(srcDir)

    return gulp.src(srcDir)
        .pipe(widthFilter)
        .pipe(responsive(
              { 
		 		 '**/*.jpg': [ { width:  width, rename: { suffix: '', extname: '.jpg' } } ] 
		 	 }, 
              {
                  quality: 100,
                  progressive: true,
                  withMetadata: false,
                  withoutEnlargement: true
              }))
        .pipe(rename(transformerYjpg))
        .pipe(widthFilter.restore)
        .pipe(gulp.dest(config.src_dir + "/img/portfolio/" + portfolio_item));
};




function responsiveFun(from="**", to=config.src.img_build_dir) {
    var fromvar = from == '**' ? '**' : from + path.sep + "**";
    function generate_specification(extnamespec) {
        var instance = [ {},
	    	       {
                       quality: 100,
                       progressive: false,
                       withMetadata: false,
                       withoutEnlargement: false
                   }
               ];

        instance[0][path.join("**" , '*' + extnamespec)] = 
                           Array.from(Array(30).keys())
                                .map((x) => (
                                    { 
                                        width: (x+1)*50, 
                                        rename: { 
                                            suffix: "-" + ((x+1)*50).toString() + "px", 
                                            extname: extnamespec  
                                        } 
                                    })); 
        console.log(instance[0]);
        return instance;

    };

    return gulp.src(path.join("app",  "img", "portfolio", fromvar, "*.{jpg,png}"))
        .pipe(responsive(...generate_specification(".jpg")))
        .pipe(rename(transformerYjpg))
        .pipe(gulp.dest(path.join(config.src_dir, config.build_dir, from == "**" ? to : config.src.img_build_dir + path.sep + from)));
}

g.responsiveFun = responsiveFun;


g.compressed_clean = function(cb) {
    if (fs.existsSync('app/build/compressed_tmp')) {
        rimraf.sync("app/build/compressed_tmp");
    }
    if (fs.existsSync('app/build/compressed')) {
        rimraf.sync("app/build/compressed");
    }
    return gulp.src('app/build');
};


g.responsive_compress_p1 = function(srcdir, cb, argv) {
    let dst_pat = '*';
    let src_pat = '*';
    let clnsrcdir = '';
    if (srcdir != "**") {
        clnsrcdir = srcdir;
        dst_pat = srcdir + '/*';
        srcdir = '/' + srcdir;
    }
    else {
        srcdir = '';
    }
    gulp.series(
        function () {
            if (fs.existsSync(config.src.compressed_tmp_dir)) {
                rimraf.sync(config.src.compressed_tmp_dir);
            }
            fs.mkdirSync(config.src.compressed_tmp_dir)
            return gulpSSH.shell(['rm -rf /Volumes/ops/img_portfolio/' + dst_pat]);
        },
        function () {
            return gulpSSH.shell(['rm -rf /Volumes/ops/img_wrk/' + dst_pat])
        },
        function () {
           let src = path.join(config.src_dir,  "build/img_build" + srcdir);
           if (srcdir && ! fs.existsSync(src)) { return gulp.src(config.src_dir); }
           fancylog(src);
           return gulp.src(src)
                  .pipe(rsync({
                      root:  "app/build/img_build/" + clnsrcdir,
                      hostname: 'vbox@192.168.0.116',
                      destination: '/Volumes/ops/img_wrk' + srcdir,
                      recursive: true,
                      archive: true,
                      silent: false,
                      compress: true,
                      clean: srcdir == '' ? true : false
                  }));
        },
        function () {
            let src = path.join(config.src_dir,  "img/portfolio" + srcdir);
            fancylog(src);
            if (srcdir && ! fs.existsSync(src)) { return gulp.src(config.src_dir); }

            return gulp.src(src)
                   .pipe(rsync({
                       root:  "app/img/portfolio/" + clnsrcdir,
                       hostname: 'vbox@192.168.0.116',
                       destination: '/Volumes/ops/img_portfolio' + srcdir,
                       recursive: true,
                       archive: true,
                       silent: false,
                       compress: true,
                       clean: srcdir == '' ? true : false
                   }))
        },
        function () {
            if (argv.noimgbuild) {
                return gulpSSH.shell(['open -a ImageOptim /Volumes/ops/img_portfolio' + srcdir]);
            }
            else {
                return gulpSSH.shell(['open -a ImageOptim /Volumes/ops/img_wrk' + srcdir + ' /Volumes/ops/img_portfolio' + srcdir]);
            }
        },
        function(done) { done(); cb(); }
    )();
};

g.responsive_compress_p10 = function(cb) {
    return gulpSSH.shell(['open -a ImageOptim /Volumes/ops/img_wrk /Volumes/ops/img_portfolio']) 
        // function (next) {
        //     return gulpSSH.shell(['/Users/vbox/Desktop/ImageOptim.app/Contents/MacOS/ImageOptim /Volumes/ops/img_wrk/'])
        //    //        .on('finish', next)
        //     ;
        // },
        // function (next) {
        //     return gulpSSH.shell(['/Users/vbox/Desktop/ImageOptim.app/Contents/MacOS/ImageOptim /Volumes/ops/img_portfolio/'])
        //    //        .on('finish', next)
        //     ;
        // },
};

g.responsive_compress_p2 = function(srcdir, cb, argv) {
    let isall = false;
    let presrcdir = '';
    if (srcdir != "**") {
        presrcdir = '/' + srcdir;
    }
    else {
        isall = true;
        srcdir = ''
    }
    var cwd = process.cwd();
    gulp.series(
        function () {
            if (fs.existsSync(config.src.compressed_tmp_dir)) {
                rimraf.sync(config.src.compressed_tmp_dir);
            }
            fs.mkdirSync(config.src.compressed_tmp_dir);
            if (!fs.existsSync(config.src.compressed_tmp_dir + '/img_build')) {
                fs.mkdirSync(config.src.compressed_tmp_dir + '/img_build');
            }
            let cmd = 'rsync --compress ' + (isall ? '--delete' : '') + ' -av /Volumes/ops/img_wrk/' + srcdir + ' ' + config.src.build_hostname + ':' + path.join(cwd, config.src.compressed_tmp_dir, 'img_build') 
            fancylog(cmd);
            return gulpSSH.shell([cmd]);
        },
        function () {
            if (!fs.existsSync(config.src.compressed_tmp_dir + '/portfolio')) {
                fs.mkdirSync(config.src.compressed_tmp_dir + '/portfolio');
            }
            let cmd = 'rsync --compress ' + (isall ? '--delete' : '') + ' -av /Volumes/ops/img_portfolio/' + srcdir + ' ' + config.src.build_hostname + ':' + path.join(cwd, config.src.compressed_tmp_dir, 'portfolio') 
            fancylog(cmd);
            return gulpSSH.shell([cmd]);
        },
        function () {
            return gulp.src(path.join(config.src.compressed_tmp_dir,  '**', '*.{jpg,png,jpeg}'))
                   .pipe(rename(transformerYjpg))
                   .pipe(gulp.dest(config.src.compressed_dir));
            ;
        },
        function () {
            if (fs.existsSync(config.src.compressed_tmp_dir)) {
                rimraf.sync(config.src.compressed_tmp_dir);
            }
            return gulp.src('app/build');
        },
        function(done) { done(); cb(); }
    )();
};

g.responsive_compress_p3 = function(srcpath, cb) {
    var cwd = process.cwd();
    let quality = 84; // Guetzli should be called with quality >= 84, otherwise the output will have noticeable artifacts. If you want to proceed anyway, please edit the source code.
    gulp.series(
        function () {
            if (srcpath == '**') {
                if (fs.existsSync(config.src.compressed_tmp2_dir)) {
                    rimraf.sync(config.src.compressed_tmp2_dir);
                }
                fs.mkdirSync(config.src.compressed_tmp2_dir);
            }
            else if (!fs.existsSync(config.src.compressed_tmp2_dir)) {
                fs.mkdirSync(config.src.compressed_tmp2_dir);
            }
            if (srcpath != '**') {
                srcpath = path.join("{img_build,portfolio}", srcpath, '**');
            }
            const newfiledir = '"' + path.join(cwd, config.src.compressed_tmp2_dir, '"$(realpath --relative-to "' + path.join(cwd, config.src.compressed_dir) + '" "<%= file.dirname %>")');
            const newfilepath = path.join(newfiledir, '"<%= file.basename %>"');

            let src = path.join(config.src.compressed_dir, srcpath, "*.jpg");
            fancylog(src);
            // TODO: 1250 p is error - need experiment with w1250 and implementing error handling - copy from previous packed 1200 to 1250
            return gulp.src(src, { read: false })
                       .pipe(shell(['mkdir -p ' + newfiledir]))
                       .pipe(shell(['echo -e "' + BrownOrange + 'converting <%= file.path %> ' + newfilepath + '"' + NC]))
                       // .pipe(shell(['echo -e "' + BrownOrange + 'converting <%= file.basename %>:' + NC + '" && ./guetzli --cuda --quality 84 "<%= file.path%>" ' + newfilepath], {cwd: config.src.guetzli_path}))
                       .pipe(shell(['echo -e "' + BrownOrange + 'converting <%= file.basename %>:' + NC + '" && python3 make/guetzli.py ' + quality.toString() + ' "<%= file.path%>" ' + newfilepath + " " + config.src.guetzli_path]))
                       .pipe(shell(['echo $(du -h ""<%= file.path%>"" | cut -f1) "->" $(du -h ' + newfilepath + ' | cut -f1)']))
                       // Если новый файл меньше старого - заменть новый на старый
                       .pipe(shell([`[[ $(du -h ""<%= file.path%>"" | cut -f1) < $(du -h ${newfilepath} | cut -f1) ]] && echo 'changing new to old' || true`]))
                       .pipe(shell([`[[ $(du -h ""<%= file.path%>"" | cut -f1) < $(du -h ${newfilepath} | cut -f1) ]] && cp -f "<%= file.path%>" ${newfilepath} || true`]));

        },
        function(done) { done(); cb(); }
    )();
};

g.responsive_compress_p4 = function() {
    return gulp.src(path.join(config.src.compressed_tmp2_dir, "**", "*.jpg"))
               .pipe(gulp.dest(config.src.compressed_dir))
};

g.responsive_compress_p4_arg = function(srcpath) {
    return gulp.src(path.join(config.src.compressed_tmp2_dir, "srcpath", "*.jpg"))
               .pipe(gulp.dest(config.src.compressed_dir))
};

g.responsive_compress_p5 = function(srcpath, cb) {
    var cwd = process.cwd();
    let files_array = [];
    let oldfile         = "";
    let oldrelativefile = ""; 
    let newfile         = ""; 
    let newrelativefile = ""; 
    let srcpath_src     = "";
    var dstbase = config.src.compressed_webp_base;

    var dst = path.join('app', 'build', dstbase); 
    if (srcpath == '**') {
        if (fs.existsSync(dst)) {
            rimraf.sync(dst);
        }
    }
    else if (!fs.existsSync(dst)) {
        fs.mkdirSync(dst);
    }
    var srcbase = config.src.compressed_tmp2_base;
    var src = path.join('app', 'build', srcbase); 
    var cwd = process.cwd();
    // const newfile= '"' + path.join(cwd, src) + '/"$(realpath --relative-to ' + path.join(cwd, src) + '/<%= file.dirname %>/<%= file.stem %>.jpg)';
    // const newfile = path.join(cwd, src) + '/<%= file.dirname %>/<%= file.stem %>.jpg';
    oldfile         = path.join(cwd, src, '$(realpath --relative-to ' + path.join(cwd, dst) + ' <%                      = file.dirname %>)/<% = file.stem %>.jpg');
    oldrelativefile = path.join(config.appbuild_dir,  srcbase, '$(realpath --relative-to ' + path.join(cwd, dst) + ' <% = file.dirname %>)/<% = file.stem %>.jpg');
    newfile         = path.join(cwd, dst, '$(realpath --relative-to ' + path.join(cwd, dst) + ' <%                      = file.dirname %>)/<% = file.stem %>.webp');
    newrelativefile = path.join(config.appbuild_dir, dstbase, '$(realpath --relative-to ' + path.join(cwd, dst) + ' <%  = file.dirname %>)/<% = file.stem %>.webp');
    srcpath_src       = path.join('app', 'build', srcbase, "**", "*.jpg")
    if (srcpath != '**') {
        srcpath_src = path.join('app', 'build', srcbase, "{img_build,portfolio}", srcpath.toString(), "**", "*.jpg")
    }
    let srcbase_path = path.join('app', 'build', srcbase);

    const glob = require('glob');
    
    files_array = glob.sync(srcpath_src);
    let threads  = 150;


    let partasks = [];
    for (let i = 0; i < files_array.length; i += threads) {
        let sl = files_array.slice(i, i + threads);
        let tasks = [];
        for (let fname_item of sl) {
            tasks.push(function(cab) {
                console.log(fname_item, dst);
                return gulp.src(fname_item, {base: srcbase_path})
                .pipe(webp({
                    quality: 88,
                    sns: 80,
                    filter: 20,
                    method: 6,
                    autoFilter: true,
                    preset: 'photo',
                }))
                .pipe(gulp.dest(dst))
                // .pipe(shell(['echo -e "           ' + BrownOrange + '<%= file.basename %>  $(du -h ' + oldfile + ' | cut -f1) -> $(du -h ' + newfile + ' | cut -f1) : ' + oldrelativefile + ' -> ' + newrelativefile + NC + '"']))
                .on('end', function() { cab(); });
            });
        };
        partasks.push(gulp.parallel(...tasks));
    }
    gulp.series(...partasks, function(done) { cb(); done(); })();
};



g.responsive_images = function(cb) {
    return responsiveFun(cb);
}; 




function imageminFun(gulp_path, dst) {
    return gulp.src([gulp_path])
               .pipe(cache(imagemin([
                   //png
                   imageminPngquant({
                       speed: 1,
                       quality: 98  // lossy settings
                   }),
                   imageminZopfli({
                       more: true
                       ,iterations: 50 // very slow but more effective
                   }),
                   // gif
                   //  imagemin.gifsicle({
                   //      interlaced: true,
                   //      optimizationLevel: 3
                   //  }),
                   // gif very light lossy, use only one of gifsicle or Giflossy
                   imageminGiflossy({
                       optimizationLevel: 3,
                       optimize: 3, // keep-empty: Preserve empty transparent frames
                       lossy: 2
                   }),
                   // svg
                   imagemin.svgo({
                       plugins: [{
                           removeViewBox: false
                       }]
                   }),
                   // jpg lossless
                   imagemin.jpegtran({
                       progressive: true
                   }),
                   // jpg very light lossy, use vs jpegtran
                   imageminMozjpeg({
                       quality: 90
                   })
               ])))
               .pipe(gulp.dest(dst));

}

gulp.task('imagemin-portfolio', function() {
    return imageminFun('app/img/portfolio/**/*.{gif,png,jpg,jpeg}', 'app/build/compressed/portfolio');
});

gulp.task('imagemin-portfolio-1512', function() {
    return imageminFun('app/build/img_build/1512/*.{gif,png,jpg,jpeg}', 'app/build/compressed/img_build/1512');
});

gulp.task('imagemin-portfolio-1M629F3', function() {
    return imageminFun('app/build/img_build/1M629F3/*.{gif,png,jpg,jpeg}', 'app/build/compressed/img_build/1M629F3');
});

gulp.task('imagemin-portfolio-5M841', function() {
    return imageminFun('app/build/img_build/5M841/*.{gif,png,jpg,jpeg}', 'app/build/compressed/img_build/5M841');
});

gulp.task('imagemin-portfolio-65A60F4_11', function() {
    return imageminFun('app/build/img_build/65A60F4_11/*.{gif,png,jpg,jpeg}', 'app/build/compressed/img_build/65A60F4_11');
});

gulp.task('imagemin-portfolio-6R13F3', function() {
    return imageminFun('app/build/img_build/6R13F3/*.{gif,png,jpg,jpeg}', 'app/build/compressed/img_build/6R13F3');
});

gulp.task('imagemin-portfolio-coordinatograph', function() {
    return imageminFun('app/build/img_build/coordinatograph/*.{gif,png,jpg,jpeg}', 'app/build/compressed/img_build/coordinatograph');
});

gulp.task('imagemin-compressed', function() {
    return imageminFun('app/build/img_build/**/*.{gif,png,jpg,jpeg}', 'app/build/compressed/img_build');
});

gulp.task('imagemin', gulp.series('imagemin-portfolio', 'imagemin-compressed', (cb) => { cb() } ))



