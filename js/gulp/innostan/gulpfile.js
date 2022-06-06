// TODO: https://www.npmjs.com/package/gulp-combine-media-queries - реализовать поддержку для продакшена

const gulp            = require('gulp');

const config          = require('./make/config');
const site_images     = require('./make/images');
const web_tasks       = require('./make/web');
const styles          = require('./make/styles');
const utils           = require('./make/utils');
const defineGulpAnon  = utils.defineGulpAnon;

const jekyll_tasks    = require('./make/jekyll');
const js_tasks        = require('./make/js_tasks');
const webpack_config  = require('./make/webpack_config');
const build_dest      = require('./make/build_dest');
const watch_tasks     = require('./make/watch');
const configure_tasks = require('./make/configure_tasks');
const yargs           = require('yargs');

const browserSync     = require('browser-sync');
const rename          = require('gulp-rename');
const isthere         = require('is-there');
const yaml            = require('write-yaml');
const process         = require('process');


let argv = yargs.argv;

require('events').EventEmitter.defaultMaxListeners = 117;
/* mode = production, develop, test, see defines in configure_tasks.js */  
/* npx gulp config --mode=develop */
/* npx gulp config --mode=production */
gulp.task('config', function(cb) { return configure_tasks.configure(argv.mode, cb); }); 

gulp.task('svg-sprite-build',         function(cb) { return site_images.svg_sprite_build(cb);     });
gulp.task('svg2raster',               function() { return site_images.svg2raster();           });

// Уменьшить ширину изображений из заданного каталога. 
// Изображения размера меньше заданного будут просто копироваться
gulp.task('resize-big-portfolio',     function() { return site_images.resize_big_portfolio(); });  // --portfolio_item=dir --width=int
gulp.task('responsive-images',        function() { return site_images.responsiveFun( "**", config.src.img_build_dir);    });

gulp.task("responsive-images-arg",    function() { /* --from=... */  return site_images.responsiveFun(argv.from); });


// run imageoptim
gulp.task('responsive-compress-p1',   function(cb) { return site_images.responsive_compress_p1("**", cb, argv);  }); // --noimgbuild (не конвертить каталог ресайза)
gulp.task('responsive-compress-p1-arg',   function(cb) { return site_images.responsive_compress_p1(argv.from, cb, argv);  }); 
// download compressed 
gulp.task('responsive-compress-p2',   function(cb) { return site_images.responsive_compress_p2("**", cb, argv);  });
gulp.task('responsive-compress-p2-arg', function(cb) { return site_images.responsive_compress_p2(argv.from, cb, argv);  });
// guetzli extreme compression
gulp.task('responsive-compress-p3',   function(cb) { return site_images.responsive_compress_p3("**", cb);  });
gulp.task('responsive-compress-p3-arg',   function(cb) { return site_images.responsive_compress_p3(argv.from, cb);  });
// move compressed guetzli to base compressed directory
gulp.task('responsive-compress-p4',   function(cb) { return site_images.responsive_compress_p4();  });
gulp.task('responsive-compress-p4-arg',   function() { return site_images.responsive_compress_p4(argv.from);  });
// compress webp;
gulp.task('responsive-compress-p5',   function(cb) { return site_images.responsive_compress_p5("**", cb);  });
gulp.task('responsive-compress-p5-arg',   function(cb) { return site_images.responsive_compress_p5(argv.from, cb);  });

gulp.task('responsive-compress-p10',   function(cb) { return site_images.responsive_compress_p10(cb);  });
gulp.task('compressed-clean',         function(cb) { return site_images.compressed_clean(cb); });


gulp.task('rsync',                    function() { return web_tasks.rsync();             });
gulp.task('browser-sync',             function() { return web_tasks.browser_sync();      });

gulp.task('styles',                   function() { return styles.styles();               });
gulp.task('styles-minify',            function() { return styles.styles_minify();        });
gulp.task('styles-rename',            function(cb) { return styles.styles_rename(cb);        });
gulp.task('styles-critical',          function() { return styles.styles_critical();      });
gulp.task('styles-critical-prepare1', function() { return styles.styles_critical_prepare1();  } );
gulp.task('styles-critical-prepare2', function() { return styles.styles_critical_prepare2();  } );
gulp.task('styles-critical-run',      function() { return styles.styles_critical_run(); } );
gulp.task('styles-critical-hand',     function() { return styles.styles_critical_hand(); });
gulp.task('styles-uncss',             function(cb) {return styles.styles_uncss(config.html, cb); });

gulp.task('jekyll',                   function(cb) { return jekyll_tasks.jekyll(cb); });
gulp.task('jekyll-sync',              function(cb) { return jekyll_tasks.jekyll_sync(cb); });
gulp.task('jekyll-sync-full',         function(cb) { return jekyll_tasks.jekyll_sync_full(cb); });
gulp.task('jekyll-watch',             function(cb) { return jekyll_tasks.jekyll_watch(cb); });
gulp.task('webpack',                  function(cb) { return js_tasks.webpack(cb); });
gulp.task('webpack-sync',             function(cb) { return js_tasks.webpack_sync(cb); });
gulp.task('webpack-watch',            function(cb) { return js_tasks.webpack_watch(cb); });
gulp.task('js-asset-noincr',          function(cb) { return js_tasks.js_asset_noincr(cb); });


gulp.task('js-asset-build',           function() { return js_tasks.js_asset_build(); });
gulp.task('js-asset-minify',          function() { return js_tasks.js_asset_minify(); });

gulp.task('js-asset',                 gulp.series('js-asset-build', 'js-asset-minify'));

gulp.task('js-minify',                function() { return js_tasks.js_minify(); });

gulp.task('dest-copy-html',           function() { return build_dest.copy_html(); });
gulp.task('dest-copy-css',            function() { return build_dest.copy_css(); });
gulp.task('dest-copy-js',             function() { return build_dest.copy_js(); });
gulp.task('dest-build',               function() { return build_dest.build_dest(); });
gulp.task('dest-img',                 function(cb) { return build_dest.build_img(cb); });
gulp.task('dest-build-full',          function() { return build_dest.build_full(); });
gulp.task('dest-clean',               function() { return build_dest.clean(); });
gulp.task('copy-static-html',         function(cb) { build_dest.copy_static(cb); } );



function if_set_include_critical(old, val) {
	if (old) {
        config.project.include_critical = val ? 1 : 0;
        yaml.sync('./app/_data/config.yml', config.project, function(err) { console.log(err); });
    }
	return old;
}


gulp.task('copy-hand-critical', function() {
    let files = config.critical_css_hand();
    return gulp.src(files)
               .pipe(gulp.dest('app/_includes/build_critical'));
});

var old_include_critical = config.project.include_critical;
var old_rename = config.project.need_rename;
var incremental_status = undefined;

const run_jekyll_t_f = (cb) => {
    // jekyll компилирует js_asset и html для анализа критического пути
    jekyll_tasks.run_jekyll({needSync: true, needWatch: false, incremental: incremental_status, cb: cb});
}

gulp.task('rebuild2', (done_rebuild) => {
    let incremental_j = false;
    incremental_status = incremental_j;
	gulp.series('dest-img', 'styles', 
        defineGulpAnon((cb) => { styles.styles_uncss({"index.html": null}, cb); }, 'styles_uncss index.html'),
        'int-jekyll-stage2', 
        defineGulpAnon((cb) => { 
            if (config.project.really_need_rename) { styles.styles_rename(cb); } else { cb(); }
        }, 'styles_rename_if_really_need_rename'), 
        'int-jekyll-webpack-stage3', 'dest-copy-css', 'dest-copy-js', 'dest-copy-html', 'dest-img', 'int-copy-static', 
        (cb) => { cb(); done_rebuild(); }
    )();
})


function rebuildProject(incremental_j, cb_done) {
    incremental_status = incremental_j;
    const gen_critical_stage1 = (task2, task3) => {
        let criticalTasksList = ['styles-critical-hand']
        if (task2)
            criticalTasksList.push('int-jekyll-critical-stage0')

        return defineGulpAnon((cb) => {
            if (config.project.include_critical) { 
                gulp.series(...criticalTasksList, (done) => { done(); cb(); })(); 
            } 
            else { 
                if (task3) { 
                    run_jekyll_t_f(cb); 
                }
                else {
                    cb()
                }
            }
        }, 'critical_stage1')
    };
    const asset_minify = defineGulpAnon(
        (cb) => {
            gulp.series(config.tasks.minify_assets_js ? 'js-asset-minify' : js_tasks.js_asset_minify_stub, defineGulpAnon((done) => { done(); cb(); }, 'finish_asset_minify')  )();
        }, 
        'asset_minify'
    );
    utils.set_need_inlining_css(0);
	gulp.series(
        'dest-img', 
        'styles', 
        gen_critical_stage1('int-jekyll-critical-stage0', (cb) => run_jekyll_t_f(cb)),
		'styles-minify', 
        gen_critical_stage1('copy-hand-critical', false),
        'js-asset-build', 
        asset_minify,
        defineGulpAnon((cb) => {
            if (config.project.include_critical) {
                gulp.series('styles-critical-prepare1', 'styles-critical-prepare2', 'styles-critical-run', defineGulpAnon((done) => { done(); cb(); }, 'finish-critical-series-1-2-run'))();
            }
            else { 
                cb(); 
            }
        }, 'critical-series-1-2-run'),
        'int-jekyll-stage1',
        // минифицируются переименованные стили и создается переименованный js asset
        defineGulpAnon((cb) => { 
            if (config.project.really_need_rename) { styles.styles_rename(cb); } else { cb(); } 
        }, 'styles_rename_if_really_need_rename'), 
        'styles-minify', 
        defineGulpAnon((cb) => { 
            if (config.project.include_critical) { 
                gulp.series('styles-critical-hand', defineGulpAnon((done) => {done(); cb();}, 'finish-styles-critical-hand'))(); 
            } 
            else { cb(); }
        }, 'styles-critical-hand'), 
        'js-asset-build',
        asset_minify,
        defineGulpAnon((cb) => { 
            if (config.project.include_critical) { 
                gulp.series('styles-critical-prepare1',
                    'styles-critical-prepare2',
                    'styles-critical-run',
                    defineGulpAnon((done) => { done(); cb(); }, 'finish-critical-series-1-2-run'))();
            } 
            else { cb(); }
        }, 'critical-series-1-2-run'), 
        defineGulpAnon((cb) => { utils.set_need_inlining_css(1); cb(); }, 'set_need_inlining_css'),
        'styles-uncss',
        'int-jekyll-stage2', 
        defineGulpAnon((cb) => { 
            if (config.project.really_need_rename) { styles.styles_rename(cb); } else { cb(); }
        }, 'styles_rename'), 
        'int-jekyll-webpack-stage3', 'dest-copy-css', 'dest-copy-js', 'dest-copy-html', 'dest-img', 'int-copy-static',
        defineGulpAnon((done) => { done(); cb_done(); }, 'finish-rebuild')
    )();
}

gulp.task('int-jekyll-critical-stage0', (cb) => {
	    // jekyll компилирует всё без переименования - html и js assets
	    utils.set_need_rename(false);
	    // Чтобы jekyll первый раз не ругался - отключить критический путь если он включен
        if (config.project.include_critical) {
            if_set_include_critical(old_include_critical, false);

            // jekyll компилирует js_asset и html для анализа критического пути
            run_jekyll_t_f(() => {
                if_set_include_critical(old_include_critical, true);
                cb();
            });
        }
        else {
            run_jekyll_t_f(cb);
        }
        // минифицируются непереименованные стили и бабелефицируется js asset непереименованный
    }
);
gulp.task('int-jekyll-stage1', function(cb) { 
        if (config.project.include_critical) {
        	// включить обновленный критический путь в html
            jekyll_tasks.run_jekyll({needSync: true, needWatch: false, incremental: incremental_status, 
                cb: () => {
                    config.project.really_need_rename && utils.set_need_rename(true); // запускается переименование
                    cb();
                }
            });
        }
        else {
           config.project.really_need_rename && utils.set_need_rename(true); // запускается переименование
           cb();
        }
        
    }
);
gulp.task('int-jekyll-stage2', function(done) {
        // jekyll компилирует html со переименованным assets
        jekyll_tasks.run_jekyll({needSync: true, needWatch: false, incremental: incremental_status, cb: done});
    } // и всё переименуется еще раз, чтобы переименованные assets попали в результирующий html
);
gulp.task('int-styles-rename', (cb) => styles.styles_rename(cb));
gulp.task('int-jekyll-webpack-stage3', function(cb) {
	    // webpack компилирует переименованные js
	    utils.set_need_rename(old_rename);
        // Если переименования не было - обновить исходник для копирования html, чтобы в нем не было переименованных assets
        if (old_rename == false) {
            jekyll_tasks.run_jekyll({needSync: true, needWatch: false, incremental: incremental_status});
        }
        js_tasks.js_webpack({needSync: true, needWatch: false, needlibs: true}); 
        cb();
    }
);

gulp.task('int-copy-static', function(cb) { build_dest.copy_static(cb); });


gulp.task('rebuild', function(cb) { rebuildProject(/* incremental_j = */ false, cb); });
gulp.task('rebuild-i', function(cb) { rebuildProject(/* incremental_j = */ true, cb); });

let need_jekyll_on_fast = false;
let incremental_jekyll_on_fast = true;




gulp.task('default', function(cb) { watch_tasks.watch_task(cb); });



var inlineImages = require('gulp-inline-images');
 
gulp.task('inline-images', function(){
    return gulp.src(['images_ai/emails/build/*.html'])
    .pipe(inlineImages({/* options */}))
    .pipe(gulp.dest('build/emails'));
});




