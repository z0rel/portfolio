import _config                       from '../_data/_config.yml'
import _config_js_asset              from '../_data/_config_js_asset.yml'
import _config_production            from '../_data/_config_production.yml'
import cfg                           from '../_data/cfg.yml'
import config                        from '../_data/config.yml'
import nav                           from '../_data/nav.yml'
import organization                  from '../_data/organization.yml'
import portfolio                     from '../_data/portfolio.yml'
import template_config               from '../_data/template_config.yml'
import _00_index                     from '../_data/portfolio/00_index.yml'
import _01_work_1512_2019            from '../_data/portfolio/01_work_1512_2019.yml'
import _02_work_65a60f1_2019         from '../_data/portfolio/02_work_65a60f1_2019.yml'
import _03_work_2637_2017            from '../_data/portfolio/03_work_2637_2017.yml'
import _04_work_65a60f4_2017         from '../_data/portfolio/04_work_65a60f4_2017.yml'
import _05_work_1512_2017            from '../_data/portfolio/05_work_1512_2017.yml'
import _06_work_5m841_2018           from '../_data/portfolio/06_work_5m841_2018.yml'
import _07_work_6r13f3_2017          from '../_data/portfolio/07_work_6r13f3_2017.yml'
import _08_work_3283_2017            from '../_data/portfolio/08_work_3283_2017.yml'
import _09_work_1m692f3_2017         from '../_data/portfolio/09_work_1m692f3_2017.yml'
import _10_work_coordinatograph_2017 from '../_data/portfolio/10_work_coordinatograph_2017.yml'
const env = process.env.NODE_ENV


const portfolio_works = {
    _01_work_1512_2019: _01_work_1512_2019,
    _02_work_65a60f1_2019: _02_work_65a60f1_2019,
    _03_work_2637_2017: _03_work_2637_2017,
    _04_work_65a60f4_2017: _04_work_65a60f4_2017,
    _05_work_1512_2017: _05_work_1512_2017,
    _06_work_5m841_2018: _06_work_5m841_2018,
    _07_work_6r13f3_2017: _07_work_6r13f3_2017,
    _08_work_3283_2017: _08_work_3283_2017,
    _09_work_1m692f3_2017: _09_work_1m692f3_2017,
    _10_work_coordinatograph_2017: _10_work_coordinatograph_2017
}

const site = {
    _config: _config,
    _config_js_asset: _config_js_asset,
    _config_production: _config_production,
    cfg: cfg,
    data: {
        config: config,
        nav: nav,
        organization: organization,
        portfolio: {
            ...portfolio,
            works: {
                ...portfolio_works
            }
        },
    },
    template_config: template_config,

    domains: env == "development" ? cfg : _config_production,

    works: {
        _00_index: _00_index,
        ...portfolio_works
    }
}

export default site