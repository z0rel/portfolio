import xlwings as xw
from scipy import optimize
from .prices import Config
from . import misc, prices
from .commercial_data import CommercialData, prepare_smet_prib_values
from .misc import UninitializedException
from target_fun import TargetFun

PREV_X0_DICT = {}
ENABLE_PREVIOUS_SAVED_LAST_DF = False


class LastResult:
    def __init__(self):
        self.df = [['Категория', 'Корректура']]
        self.x0 = []


def extend_cathegories(src_list, cat_list):
    try:
        for cat in src_list:
            if cat is None:
                continue
            fcat = float(cat)
            cat_list.append(fcat)
    except KeyError:
        pass


@xw.func
@xw.ret(expand='table', index=False, async_mode='threading')
def calculate_correcture_r1(range_eval, range_eval_cfg, range_cathegories, range_values, need_calc, stage=1):
    range_cathegories1 = xw.books.active.sheets['Brief'].range(range_cathegories).value
    calculate_correcture(range_eval, range_eval_cfg, range_cathegories1, range_values, need_calc, stage)


@xw.func
@xw.ret(expand='table', index=False)
def calculate_correcture_cat(
    cat_column, range_eval, range_eval_cfg, range_cathegories, range_values, need_calc, additional_data
):
    stage = need_calc
    is_error = False
    if cat_column is None:
        return [['ERR', 'ERROR calculate_correcture_cat: cat_column is None']]
    if range_eval is None:
        return [['ERR', 'ERROR calculate_correcture_cat: range_eval is None']]
    if is_error:
        print(range_eval)
        print(range_eval_cfg)
        print(range_cathegories)
        print(range_values)
        return [['ERR', 'ERR']]
    try:
        return calculate_correcture_internal(
            range_eval,
            range_eval_cfg,
            range_cathegories,
            range_values,
            need_calc,
            stage,
            int(cat_column),
            additional_data,
        )
    except UninitializedException:
        return [['ERR', 'ERR - UninitializedException']]


@xw.func
@xw.ret(expand='table', index=False, async_mode='threading')
def calculate_correcture(range_eval, range_eval_cfg, range_cathegories, range_values, need_calc, stage=1):
    return calculate_correcture_internal(
        range_eval, range_eval_cfg, range_cathegories, range_values, need_calc, stage, None
    )


def get_last_result(ofname):
    if ofname not in PREV_X0_DICT:
        PREV_X0_DICT[ofname] = LastResult()
    return PREV_X0_DICT[ofname]


def calculate_correcture_internal(
    range_eval, range_eval_cfg, range_cathegories, range_values, need_calc, stage=1, cat_column=-1, additional_data=None
):
    cfg = Config(range_eval_cfg)

    global PREV_X0_DICT

    last_result = get_last_result(cfg.key_calculation())

    if not need_calc or cfg.is_skip_calculate():
        return last_result.df

    dst_cathegories = []
    dst_values = []
    all_cathegories = []

    for cat, val in zip(range_cathegories, range_values):
        if cat is not None and cat != '':
            if isinstance(cat, float):
                pass
            else:
                cat = cat.replace(',', '.')
            try:
                fcat = float(cat)
                all_cathegories.append(fcat)
            except Exception:
                pass
        else:
            continue
        if val is None or val == '':
            continue
        try:
            fval = float(val)
            dst_cathegories.append(float(cat))
            dst_values.append(fval)
        except Exception:
            pass

    dst_cathegories_set = {k: v for k, v in zip(dst_cathegories, dst_values)}

    dst_aggr_works = []
    target_values = []

    cfg.is_correcture_minimizer = True
    agctx = prepare_smet_prib_values(range_eval, cfg, additional_data, cat_column, None, None)
    if agctx is None:
        return None

    first_dogovor_object = CommercialData(agctx, None, None)
    first_dogovor_object.set_risk_itog_only()
    first_dogovor_object.generate()

    print('price_dogovor', first_dogovor_object.price_dogovor)

    for wrk in first_dogovor_object.aggregated_works:
        cat = wrk.get_double_cathegory()
        if cat in dst_cathegories_set:
            dst_aggr_works.append(wrk)
            target_values.append(dst_cathegories_set[cat])

    # for x, y in zip([wrk for wrk in dst_aggr_works], target_values):
    #     print(x.cb_itog_data[x.cb_itog_data_idx, 2], x.get_double_cathegory(), y)

    x0_v = [wrk.get_local_correcture_raw() for wrk in dst_aggr_works]

    if ENABLE_PREVIOUS_SAVED_LAST_DF:
        if len(last_result.x0) == len(x0_v) and all([x == y for (x, y) in zip(last_result.x0, x0_v)]):
            return last_result.df

    last_result.x0 = x0_v[:]

    def custom_minimize(dogovor_object, method=None, options=None):
        target_fun = TargetFun(stage, dst_aggr_works, target_values, dogovor_object)

        try:
            if options:
                res = optimize.minimize(target_fun.target_fun, x0_v, method=method, options=options)
            else:
                if method:
                    res = optimize.minimize(target_fun.target_fun, x0_v, method=method)
                else:
                    res = optimize.minimize(target_fun.target_fun, x0_v)

            founded_x0 = res.x
        except misc.StopMinimization:
            founded_x0 = target_fun.founded_x0
            pass

        result = []
        for wrk, x in zip(dst_aggr_works, founded_x0):
            if wrk.is_work():
                normalized = x
                item = (wrk.get_double_cathegory(), float('{0:.3f}'.format(normalized)))
                result.append(item)
                # print(item)

        for wrk, x in zip(dst_aggr_works, founded_x0):
            if not wrk.is_work():
                normalized = x / 1000
                item = (wrk.get_double_cathegory(), float('{0:.3f}'.format(normalized)))
                result.append(item)
                # print(item)
        print(target_fun.last_result)
        return result

    # Powell +-
    # COBYLA -+
    # result = custom_minimize('Nelder-Mead', options={'xatol': 0.1, 'fatol': 0.1})
    # METHOD = 'Nelder-Mead'
    METHOD = 'Powell'
    # METHOD = 'CG'
    # METHOD = 'BFGS'
    # METHOD = 'Newton-CG'
    # METHOD = 'L-BFGS-B'
    # METHOD = 'TNC'
    # METHOD = 'COBYLA'
    # METHOD = 'SLSQP'
    # METHOD = 'trust-constr'
    # METHOD = 'dogleg'
    # METHOD = 'trust-ncg'
    # METHOD = 'trust-krylov'
    # METHOD = 'trust-exact'

    maxiter = 1000000
    options = {'xtol': 0.0000000000001, 'ftol': 0.0000000000001, 'maxiter': maxiter, 'maxfev': maxiter}
    if stage == 3:
        result = custom_minimize(first_dogovor_object, METHOD, options=options)
    else:
        result = custom_minimize(first_dogovor_object, METHOD, options=options)

    cat_by_res = {(float(x) if x else ''): y for (x, y) in result}

    dst = [('Категория', 'Корректура')]
    for x in range_cathegories:
        try:
            if isinstance(x, float):
                cat = x
            elif x is None or x == '':
                dst.append((None, None))
                continue
            else:
                cat = float(x.replace(',', '.'))
            if cat in cat_by_res:
                dst.append((cat, cat_by_res[cat]))
        except Exception:
            dst.append((None, None))

    last_result.df = dst
    return dst
