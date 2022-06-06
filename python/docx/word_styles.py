from docx.enum.style import WD_STYLE_TYPE
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.text import WD_TAB_ALIGNMENT
from docx.enum.text import WD_LINE_SPACING

from docx.shared import Pt
from docx.shared import Cm
from docx.shared import RGBColor

from parsing_context import AdditionalTkpData

from .. import commercial_config

from .docx_ext import (
    set_cell_border,
    set_repeat_table_header,
    lining_elem,
    shading_elem,
)


class TS:
    def __init__(self, cm, alignment):
        self.cm = cm
        self.alignment = alignment


def add_styles(
    document, data_obj, cfg: commercial_config.Config, item_cfg: AdditionalTkpData
):
    left_indent = 1
    left = WD_ALIGN_PARAGRAPH.LEFT
    center = WD_ALIGN_PARAGRAPH.CENTER
    justify = WD_ALIGN_PARAGRAPH.JUSTIFY
    right = WD_ALIGN_PARAGRAPH.RIGHT
    justify_low = WD_ALIGN_PARAGRAPH.JUSTIFY_LOW
    li_ts = [left_indent + 0.5]
    ts052 = [0.52]
    ts01_07 = [0.1, 0.7]
    ts01_052 = [0.1, 0.52]
    ts038_075 = [0.38, 0.75]
    ts06_1 = [0.6, 1.0]
    styles_dict = {}
    sps = WD_LINE_SPACING.SINGLE
    spm = WD_LINE_SPACING.MULTIPLE

    core_properties = document.core_properties
    core_properties.author = "Галина Петровна Шмарина"
    core_properties.keywords = data_obj.cfg.stan_name_brief if data_obj else ""
    core_properties.subject = "Коммерческое предложение"
    core_properties.language = "RU"
    core_properties.comments = ""

    d = {
        "f_main": "Roboto",
        "f_main_b": "Roboto Medium",
        "f_main_l": "Roboto Light",
        "f_mono": "Consolas",
        "f_title": "Century",  # 'Playfair Display'
        "o_title": 0.5,
        "s_main": 11,
        "s_main_s": 10,
        "s_price_s": 9,
        "s_tabhead_s": 9.4,
        "s_title_1": 16,
        "s_title_2": 14,
        "s_small": 8,
    }

    for x in ["f_main", "f_main_b", "f_main_l", "f_mono", "f_title"]:
        k = d[x] + "_need_bold"
        if k not in d:
            d[k] = False

    o_title = d["o_title"]
    f_main = d["f_main"]
    f_main_b = d["f_main_b"]
    f_main_l = d["f_main_l"]
    f_mono = d["f_mono"]
    f_title = d["f_title"]
    s_main = d["s_main"]
    s_main_s = d["s_main_s"]
    s_price_s = d["s_price_s"]
    s_tabhead_s = d["s_tabhead_s"]
    s_title_1 = d["s_title_1"]
    s_title_2 = d["s_title_2"]
    s_small = d["s_small"]
    ro = 0.15
    rl = -0.2
    cli = -0.2
    cri = -0.2

    s_tc_head = s_main + o_title
    s_calc = s_title_2 + o_title * 2
    s_tc_head_s = s_main_s + o_title
    s_tc_hpirce = s_tabhead_s + o_title * 0.8
    s_price_titl = s_title_1 + o_title * 2

    roboto = "Roboto"
    robotol = "Roboto Light"
    century = "Century"
    light_gray = RGBColor(165, 168, 181)
    dark_gray = RGBColor(166, 166, 166)
    dark_gray_accent = RGBColor(113, 118, 137)
    url_link_color = RGBColor(22, 132, 177)

    tsr = WD_TAB_ALIGNMENT.RIGHT
    commercial_tab = 0.4
    ct = 1.25 + commercial_tab

    robotom = "Roboto Medium"
    ts_footer = [TS(16.5, tsr)]

    lemma_before = item_cfg.spacer_before_lemma
    tkp_mrg = (
        item_cfg.spacer_before_title
        if item_cfg.spacer_before_title >= 0
        else data_obj.cfg.margin_before_tkp
    )

    styles = [
        # name                                , family   , pt           , tabstops                , first_line_indent , space_before , space_after , alignment , left_indent , right_indent , line_spacing , spacing_rule , color            , is_paragraph , is_italic
        ["Picture_title"                      , century  , s_main + 1   , None                    , 0                 , 0            , 0           , center    , None        , 0            , 1.15         , spm          , ]                ,
        ["Picture_title_tire"                 , robotol  , s_main + 1   , None                    , 0                 , 0            , 0           , center    , None        , 0            , 1.15         , spm          , None             , False        , ]         ,
        ["Tabcell_first_line"                 , f_title  , s_tc_head    , ts052                   , None              , 0            , 2           , left      , None        , 0            , 1.15         , spm          , ]                ,
        ["Tabcell_first_line_tire"            , f_main_l , s_tc_head    , ts052                   , None              , 0            , 2           , left      , None        , 0            , 1.15         , spm          , None             , False        , ]         ,
        ["Picture_self"                       , century  , s_main       , None                    , 0                 , 6            , 12          , center    , None        , 0            , 1.15         , spm]         ,
        ["Picture_title_left"                 , century  , s_main + 1   , None                    , 0                 , 0            , 0           , left      , None        , 0            , 1.15         , spm          , ]                ,
        ["Picture_title_tire_left"            , robotol  , s_main + 1   , None                    , 0                 , 0            , 0           , left      , None        , 0            , 1.15         , spm          , None             , False        , ]         ,
        ["Picture_self_left"                  , century  , s_main       , None                    , 0                 , 6            , 12          , left      , None        , 0            , 1.15         , spm          , ]                ,
        ["Price_title_left"                   , century  , s_main       , None                    , 0                 , 0            , 2           , left      , None        , 0            , 1.15         , spm]         ,
        ["Price_title_left_nonfirstonpage"    , century  , s_main       , None                    , 0                 , 12           , 2           , left      , None        , 0            , 1.15         , spm          , ]                ,
        ["Price_title_tire_left"              , robotol  , s_main       , None                    , 0                 , 0            , 2           , left      , None        , 0            , 1.15         , spm          , None             , False        , ]         ,
        ["Price_self_left"                    , century  , s_main       , None                    , 0                 , 6            , 12          , left      , None        , 0            , 1.15         , spm]         ,
        ["Price_deadline"                     , f_main   , s_main       , None                    , 0                 , 18           , 0           , left      , None        , 0            , 1.15         , spm]         ,
        ["Price_payment"                      , f_main   , s_main       , [3.4]                   , 0                 , 6            , 0           , left      , None        , 0            , 1.15         , spm]         ,
        ["Price_payment2"                     , f_main   , s_main       , [3.4]                   , 0                 , 3            , 0           , left      , None        , 0            , 1.15         , spm]         ,
        ["Price_additional1"                  , f_main   , s_main       , [3.4]                   , 0                 , 15           , 0           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        ["Price_additional2"                  , f_main   , s_main       , [3.4]                   , 0                 , 6            , 0           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        ["Brief_uvazh"                        , f_main   , s_main       , None                    , 0                 , 24           , 6           , justify   , None        , 0            , 1.15         , spm          , None             , True         , True      , ] ,
        ["Brief_dir"                          , robotom  , s_main + 1   , [13.75]                 , 0                 , 0            , 0           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        ["Brief_dir1"                         , robotom  , s_main + 1   , [13.75]                 , 0                 , 18           , 0           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        ["Commercial_text_sum"                , f_main   , s_main       , None                    , 1.25              , 12           , 4           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        ["Commercial_text_sum1"               , f_main   , s_main       , None                    , 1.25              , 4            , 4           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        ["Commercial_text_details"            , f_main   , s_main       , None                    , 1.25              , 0            , 0           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        ["Commercial_text_details_item_sign"  , robotol  , s_main       , [ct]                    , 1.25              , 0            , 0           , justify   , None        , 0            , 1.15         , spm          , None             , False        , ]         ,
        ["Commercial_text_details_item"       , f_main   , s_main       , [ct]                    , -0.35             , 0            , 0           , justify   , 1.6         , 0            , 1.15         , spm          , ]                ,
        ["Commercial_text_it_valuta"          , f_main   , s_main       , [ct, ct + 0.4]          , -0.35             , 0            , 0           , justify   , 1.6         , 0            , 1.15         , spm          , ],

        ["Commercial_payment_head"            , f_main   , 12           , None                    , 0                 , 24           , 6           , center    , None        , 0            , 1.15         , spm          , ]                ,
        ["Commercial_payment_item"            , f_main   , s_main       , [commercial_tab]        , -commercial_tab   , 0            , 6           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        ["Commercial_payment_item_sign"       , f_main   , s_main       , [commercial_tab]        , -commercial_tab   , 0            , 6           , justify   , None        , 0            , 1.15         , spm          , None             , False        , ]         ,
        ["Title_header_2s_0"                  , robotol  , 1            , None                    , 0                 , 0            , 0           , left      , None        , 0            , 1.0          , sps          , dark_gray        , ]            ,
        ["Title_header_2s_1"                  , robotol  , 9            , None                    , 0                 , 0            , 0           , left      , None        , 0            , 1.0          , sps          , dark_gray        , ]            ,
        ["Title_footer0"                      , robotol  , 9            , None                    , 0                 , 0            , 0           , left      , None        , 0            , 1.25         , spm          , light_gray       , ]            ,
        ["Title_footer1"                      , robotol  , 8            , ts_footer               , 0                 , 0            , 0           , left      , None        , 0            , 1.25         , spm          , light_gray       , ]            ,
        ["Title_footer1_l"                    , robotol  , 8            , None                    , 0                 , 0            , 0           , left      , None        , 0            , 1.0          , sps          , light_gray       , False        , ]         ,
        ["Title_footer1_page_number"          , robotol  , s_main       , None                    , 0                 , 0            , 0           , left      , None        , 0            , 1.0          , sps          , light_gray       , False        , ]         ,
        ["Title_tkp0"                         , century  , 16           , None                    , 0                 , tkp_mrg      , 4           , center    , None        , 0            , 1.15         , spm]         ,
        ["Title_tkp1"                         , century  , 16           , None                    , 0                 , 0            , 0           , center    , None        , 0            , 1.15         , spm]         ,
        ["Title_tkp_tire"                     , robotol  , 16           , None                    , 0                 , 0            , 12          , center    , None        , 0            , 1.15         , spm          , None             , False        , ]         ,
        ["Title_tkp0_im"                      , century  , 16           , None                    , 0                 , tkp_mrg      , 4           , center    , 3           , 0            , 1.15         , spm]         ,
        ["Title_tkp1_im"                      , century  , 16           , None                    , 0                 , 0            , 0           , center    , 3           , 0            , 1.15         , spm]         ,
        ["Title_tkp_tire_im"                  , robotol  , 16           , None                    , 0                 , 0            , 12          , center    , 3           , 0            , 1.15         , spm          , None             , False        , ]         ,
        ["Title_calc_tkp0"                    , century  , 15           , None                    , 0                 , 18           , 0           , center    , None        , 0            , 1.15         , spm]         ,
        ["Title_calc_tkp1"                    , century  , 15           , None                    , 0                 , 0            , 12          , center    , None        , 0            , 1.15         , spm]         ,
        ["Title_calc_tkp_tire"                , robotol  , 15           , None                    , 0                 , 0            , 12          , center    , None        , 0            , 1.15         , spm          , None             , False        , ]         ,
        ["Their_tkp"                          , f_main   , s_main       , None                    , 1.25              , lemma_before , 3           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        # name                                , family   , pt           , tabstops                , first_line_indent , space_before , space_after , alignment , left_indent , right_indent , line_spacing , spacing_rule , color            , is_paragraph , is_italic
        ["Their_tkp_punkts_appendix_tkp_list" , robotol  , s_main - 2   , None                    , 0                 , 0            , 3           , justify   , None        , 0            , 1.15         , spm          , None             , False        , ]         ,
        ["Their_tkp_punkts0"                  , f_main   , s_main       , [0.5]                   , 0                 , 0            , 3           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        ["Their_tkp_punkts0_1"                , f_main   , s_main       , [0.75]                  , 0                 , 0            , 3           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        ["Their_tkp_punkts0_tire"             , f_main   , s_main       , [0.5]                   , 0                 , 0            , 1           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        ["Their_tkp_punkts0_1_tire"           , f_main   , s_main       , [0.75]                  , 0                 , 0            , 1           , justify   , None        , 0            , 1.15         , spm          , ]                ,
        ["Title_header_l0"                    , roboto   , 8            , None                    , 0                 , 0            , 0           , left      , None        , 0            , 1.0          , sps          , light_gray       , ]            ,
        ["Title_header_l1"                    , roboto   , 8            , None                    , 0                 , 5            , 0           , left      , None        , 0            , 1.0          , sps          , light_gray       , ]            ,
        ["Title_header_l2"                    , roboto   , 8            , None                    , 0                 , 0            , 6           , left      , None        , 0            , 1.0          , sps          , light_gray       , ]            ,
        ["Title_header_l3"                    , roboto   , 8            , None                    , 0                 , 0            , 0           , left      , None        , 0            , 1.0          , sps          , light_gray       , ]            ,
        ["Title_header_l4"                    , f_main   , s_main       , None                    , 0                 , 15           , 0           , left      , None        , 0            , 1.0          , sps          , dark_gray_accent , ]            ,
        ["Title_header_l_additional"          , f_main   , s_main       , None                    , 0                 , 8            , 0           , left      , None        , 0            , 1.15         , spm          , dark_gray_accent , ]            ,
        ["Title_header_r0"                    , robotol  , 8            , None                    , 0                 , 0            , 0           , right     , None        , 0            , 1.2          , spm          , dark_gray        , ]            ,
        ["Title_header_r1"                    , robotol  , 8            , None                    , 0                 , 5            , 0           , right     , None        , 0            , 1.2          , spm          , dark_gray        , ]            ,
        ["Title_header_r"                     , robotol  , 8            , None                    , 0                 , 0            , 0           , right     , None        , 0            , 1.2          , spm          , dark_gray        , ]            ,
        ["Title_header_r_l"                   , robotol  , 8            , None                    , 0                 , 6            , 0           , right     , None        , 0            , 1.0          , sps          , url_link_color   , False        , ]         ,
        ["Footer_style"                       , f_main_l , s_small      , [16.5]                  , None              , 0            , 0           , None      , 0           , 0            , 1.0          , sps]         ,
        ["Tabprefix"                          , f_main   , s_main       , None                    , None              , 0            , 0]          ,
        ["Tabprefix_paragraph"                , f_main   , s_main       , li_ts                   , left_indent       , 6            , 3]          ,
        ["Tabprefix_paragraph_bf12"           , f_main   , s_main       , li_ts                   , left_indent       , 12           , 3]          ,
        ["Tabprefix_paragraph_bf12_bold"      , f_main_b , s_main       , li_ts                   , left_indent       , 12           , 3]          ,
        ["Paragraph_NOT_NDS"                  , f_main   , s_main_s     , li_ts                   , 0                 , 3            , 3           , justify   , 0]          ,
        ["Paragraph_NOT_NDS_main"             , f_main   , s_main_s     , li_ts                   , 0                 , 5            , 9           , justify   , 0           , 0            , 1.15         , spm          , ]                ,
        ["Paragraph_NOT_NDS_price"            , f_main   , s_price_s    , li_ts                   , 0                 , 5            , 9           , justify   , 0           , 0            , 1.15         , spm          , ]                ,
        ["Tabhead"                            , f_main_b , s_main       , None                    , 0                 , 2            , 1           , center]   ,
        ["TZ_lemma"                           , f_main   , s_main       , ts052                   , 1                 , 20           , 3           , None      , 0]          ,
        ["SmallPriceR"                        , f_main_b , s_small      , None                    , None              , 2            , 2           , center]   ,
        ["SmallPriceR_00"                     , f_main_b , s_small      , None                    , None              , 2            , 2           , center]   ,
        ["SmallPriceR_02"                     , f_main_b , s_small      , None                    , None              , 0            , 2           , center]   ,
        ["Tabcell_head_R"                     , f_main_b , s_main       , ts052                   , None              , 2            , 2           , center]   ,
        ["Tabcell_head_R_00"                  , f_main_b , s_main       , ts052                   , None              , 2            , 0           , center]   ,
        ["Tabcell_head_a"                     , f_main   , s_main       , ts052]                  ,
        ["Tabcell_number_R"                   , f_main_l , s_main       , ts052                   , None              , 1            , 1           , center    , cli         , cri]         ,
        ["Tabcell_number_R_r"                 , f_main_l , s_main       , ts052                   , None              , 1            , 1           , right     , rl          , ro]          ,
        ["Tabcell_descr_R"                    , f_main   , s_main       , ts052                   , None              , 1            , 1           , None]     ,
        ["Tabcell_descr_itog_R"               , f_main_b , s_main       , ts052                   , None              , 1            , 1           , None]     ,
        ["Tabcell_descr_R_00"                 , f_main   , s_main       , ts052                   , None              , 1            , 0           , None]     ,
        ["Tabcell_descr_R_01"                 , f_main   , s_main       , ts052                   , None              , 0            , 0           , None]     ,
        ["Tabcell_descr_R_02"                 , f_main   , s_main       , ts052                   , None              , 0            , 1           , None]     ,
        ["Tabcell_itog_R"                     , f_main   , s_main       , ts052                   , None              , 1            , 1           , center    , cli         , cri]         ,
        ["Tabcell_itog_R_r"                   , f_main   , s_main       , ts052                   , None              , 1            , 1           , right     , rl          , ro]          ,
        ["Tabcell_itog_itog_R"                , f_main_b , s_main       , ts052                   , None              , 1            , 1           , center    , cli         , cri]         ,
        ["Tabcell_itog_itog_R_r"              , f_main_b , s_main       , ts052                   , None              , 1            , 1           , right     , rl          , ro]          ,
        ["Tabcell_number_R_12"                , f_main_l , s_main       , ts052                   , None              , 2            , 1           , center    , cli         , cri]         ,
        ["Tabcell_number_R_12_r"              , f_main_l , s_main       , ts052                   , None              , 2            , 1           , right     , rl          , ro]          ,
        ["Tabcell_descr_R_12"                 , f_main   , s_main       , ts052                   , None              , 2            , 1           , None]     ,
        ["Tabcell_descr_R_00_12"              , f_main   , s_main       , ts052                   , None              , 2            , 0           , None]     ,
        ["Tabcell_descr_R_01_12"              , f_main   , s_main       , ts052                   , None              , 0            , 0           , None]     ,
        ["Tabcell_descr_R_02_12"              , f_main   , s_main       , ts052                   , None              , 0            , 1           , None]     ,
        ["Tabcell_itog_R_12_r"                , f_main   , s_main       , ts052                   , None              , 1            , 1           , right     , rl          , ro]          ,
        ["Tabcell_itog_R_12"                  , f_main   , s_main       , ts052                   , None              , 1            , 1           , center    , cli         , cri]         ,
        ["Tabcell"                            , f_main   , s_main       , ts052]                  ,
        ["Smeta_head"                         , f_main   , s_main       , None                    , None              , 0            , 0]          ,
        ["Smeta_head_last"                    , f_main   , s_main       , None                    , None              , 0            , 6]          ,
        ["Tabprefix_second"                   , f_main   , s_main       , li_ts                   , left_indent       , 18           , 3]          ,
        ["Tabprefix_second_pagebreaked"       , f_main   , s_main       , li_ts                   , left_indent       , 0            , 3]          ,
        ["Tabprefix_paragraph_bf12_c"         , f_main   , s_main       , li_ts                   , left_indent       , 12           , 3]          ,
        ["Tabprefix_paragraph_bf12_c_breaked" , f_main   , s_main       , li_ts                   , left_indent       , 0            , 3           , ]         ,
        ["Tabcell_work_head"                  , f_main   , s_main_s     , ts052                   , None              , 3            , 0           , center    , cli         , cri]         ,
        ["TabcellR"                           , f_main   , s_main       , ts052]                  ,
        ["Tabprefix_paragraph_itogo"          , f_main_b , s_main       , li_ts                   , left_indent       , 18           , 3]          ,
        ["Trud_itogo_header"                  , f_main   , s_main       , li_ts                   , left_indent       , 3            , 0]          ,
        ["Trud_itogo_detail"                  , f_main   , s_main       , li_ts                   , left_indent       , 0            , 0]          ,
        ["Trud_itogo_detail_last"             , f_main   , s_main       , li_ts                   , left_indent       , 18           , 3]          ,
        ["Tabcell_prochie"                    , f_main   , s_main       , ts052                   , None              , 1            , 2]          ,
        ["Tabcell_head"                       , f_title  , s_tc_head    , ts052                   , None              , 2            , 2           , center]   ,
        ["Calculation"                        , f_title  , s_calc       , None                    , None              , 18           , 12          , center]   ,
        ["Tabcell_head_price_first"           , f_title  , s_tc_head_s  , ts052                   , None              , 3            , 0           , center    , cli         , cri          , 1.0          , sps          , ]                ,
        ["Tabcell_head_price"                 , f_title  , s_tc_head_s  , ts052                   , None              , 0            , 0           , center    , cli         , cri          , 1.0          , sps          , ]                ,
        ["Tabcell_head_price1"                , f_title  , s_tc_hpirce  , ts052                   , None              , 6            , 4           , center    , cli         , cri          , 1.0          , sps          , ]                ,
        ["Tabcell_head_price2"                , f_title  , s_tc_hpirce  , ts052                   , None              , 3            , 2           , center    , cli         , cri          , 1.0          , sps          , ]                ,
        ["Tabcell_head_price1_0"              , f_title  , s_tc_hpirce  , ts052                   , None              , 3            , 0           , center    , cli         , cri          , 1.0          , sps          , ]                ,
        ["Tabcell_head_price1_1"              , f_title  , s_tc_hpirce  , ts052                   , None              , 0            , 0           , center    , cli         , cri          , 1.0          , sps          , ]                ,
        ["Tabcell_head_price1_2"              , f_title  , s_tc_hpirce  , ts052                   , None              , 0            , 4           , center    , cli         , cri          , 1.0          , sps          , ]                ,
        ["Price_title"                        , f_title  , s_price_titl , ts052                   , None              , 0            , 6           , center    , None]       ,
        ["Price_lemma_text"                   , f_main   , s_main       , ts052                   , left_indent       , 0            , 3           , justify]  ,
        ["Price_lemma_text_l"                 , f_main_l , s_main       , ts052                   , left_indent       , 0            , 3           , justify]  ,
        # name                                , family   , pt           , tabstops                , first_line_indent , space_before , space_after , alignment , left_indent , right_indent , line_spacing , spacing_rule , color            , is_paragraph , is_italic
        ["Tabcell_content_price"              , f_main   , s_main       , ts052                   , None              , 3            , 3]          ,
        ["Tabcell_content_price_appendix"     , f_main_l , s_main - 2   , ts052                   , None              , 3            , 3           ]          ,
        ["Tabcell_content_price_second"       , f_main_l , s_main - 1   , ts052                   , None              , 6            , 3]          ,
        ["Tabcell_content_price_0"            , f_main   , s_main       , ts052                   , None              , 3            , 0]          ,
        ["Tabcell_content_price_1"            , f_main   , s_main       , ts052                   , None              , 0            , 0]          ,
        ["Tabcell_content_price_2"            , f_main   , s_main       , ts052                   , None              , 0            , 3]          ,
        ["Tabcell_content_price_center"       , f_main   , s_main       , ts052                   , None              , 3            , 3           , center    , cli         , cri          , ]            ,
        ["Tabcell_content_price_center_0"     , f_main   , s_main       , ts052                   , None              , 3            , 0           , center    , cli         , cri          , ]            ,
        ["Tabcell_content_price_center_1"     , f_main   , s_main       , ts052                   , None              , 0            , 0           , center    , cli         , cri          , ]            ,
        ["Tabcell_content_price_center_2"     , f_main   , s_main       , ts052                   , None              , 0            , 3           , center    , cli         , cri          , ]            ,
        ["Tabcell_content_price_l"            , f_main_l , s_main       , ts052                   , None              , 3            , 3]          ,
        ["Tabcell_content_price_0_l"          , f_main_l , s_main       , ts052                   , None              , 3            , 0]          ,
        ["Tabcell_content_price_1_l"          , f_main_l , s_main       , ts052                   , None              , 0            , 0]          ,
        ["Tabcell_content_price_2_l"          , f_main_l , s_main       , ts052                   , None              , 0            , 3]          ,
        ["Tabcell_content_price_center_l"     , f_main_l , s_main       , ts052                   , None              , 3            , 3           , center    , cli         , cri          , ]            ,
        ["Tabcell_content_price_center_0_l"   , f_main_l , s_main       , ts052                   , None              , 3            , 0           , center    , cli         , cri          , ]            ,
        ["Tabcell_content_price_center_1_l"   , f_main_l , s_main       , ts052                   , None              , 0            , 0           , center    , cli         , cri          , ]            ,
        ["Tabcell_content_price_center_2_l"   , f_main_l , s_main       , ts052                   , None              , 0            , 3           , center    , cli         , cri          , ]            ,
        ["Tabcell_pt1"                        , f_main_l , s_main       , ts052                   , None              , 0            , 0           , left      , 0           , 0            , 1.1          , spm]         ,
        ["Tabcell_pt1_3lev"                   , f_main_l , s_main       , [0.8]                   , None              , 0            , 0           , left      , 0           , 0            , 1.1          , spm]         ,
        ["Tabcell_pt2"                        , f_main_l , s_main       , [0.7]                   , None              , 0            , 0           , left      , 0           , 0            , 1.1          , spm]         ,
        ["Tabcell_pt2_3lev"                   , f_main_l , s_main       , [1.0]                   , None              , 0            , 0           , left      , 0           , 0            , 1.1          , spm]         ,
        ["CodeConsolas"                       , f_mono   , s_main_s     , None                    , 0                 , 0            , 0]          ,
        ["CodeConsolas_1lev"                  , f_mono   , s_main_s     , [1]                     , 0                 , 0            , 0]          ,
        ["Work_paragraph_tire_pt11"           , f_main   , s_main       , ts038_075               , -0.75             , 0            , 0]          ,
        ["Work_paragraph_tire_pt11_last"      , f_main   , s_main       , ts038_075               , -0.75             , 0            , 2]          ,
        ["Work_paragraph_tire_pt12"           , f_main   , s_main       , ts06_1                  , -1.0              , 0            , 0]          ,
        ["Work_paragraph_tire_pt12_last"      , f_main   , s_main       , ts06_1                  , -1.0              , 0            , 2]          ,
        ["Work_paragraph_pt11"                , f_main   , s_main       , [0.75]                  , None              , 0            , 0]          ,
        ["Work_paragraph_pt12"                , f_main   , s_main       , [1.0]                   , None              , 0            , 0]          ,
        ["Work_paragraph_pt11_3lev"           , f_main   , s_main       , [1.1]                   , None              , 0            , 0]          ,
        ["Work_paragraph_pt12_3lev"           , f_main   , s_main       , [1.45]                  , None              , 0            , 0]          ,
        ["Tabcell_tire_pt1"                   , f_main_l , s_main       , ts01_052                , -0.52             , 0            , 0           , left      , 0           , 0            , 1.1          , spm          , ]                ,
        ["Tabcell_tire_pt1_last"              , f_main_l , s_main       , ts01_052                , -0.52             , 1            , 0           , left      , 0           , 0            , 1.1          , spm          , ]                ,
        ["Tabcell_tire_pt2"                   , f_main_l , s_main       , ts01_07                 , -0.7              , 0            , 0           , left      , 0           , 0            , 1.1          , spm          , ]                ,
        ["Tabcell_tire_pt2_last"              , f_main_l , s_main       , ts01_07                 , -0.7              , 1            , 0           , left      , 0           , 0            , 1.1          , spm          , ]                ,
    ]
    for x in styles:
        add_style(document, styles_dict, d, *x)

    for val in [
        "Tabcell_head_price",
        "Tabcell_head_price1",
        "Price_title",
        "Calculation",
        "Tabcell_head",
        "Tabcell_head_price_first",
        "Tabcell_head_price1_0",
        "Tabcell_head_price1_1",
        "Tabcell_head_price1_2",
        "Calculation",
        "Tabcell_first_line",
    ]:
        styles_dict[val]._element.get_or_add_rPr().append(lining_elem())

    styles_dict["Work_paragraph_tire_pt11"].paragraph_format.alignment = justify_low
    styles_dict["Work_paragraph_tire_pt11_last"].paragraph_format.alignment = justify_low
    styles_dict["Work_paragraph_tire_pt12"].paragraph_format.alignment = justify_low
    styles_dict["Work_paragraph_tire_pt12_last"].paragraph_format.alignment = justify_low
    styles_dict["Work_paragraph_pt11"].paragraph_format.alignment = justify_low
    styles_dict["Work_paragraph_pt12"].paragraph_format.alignment = justify_low
    styles_dict["Work_paragraph_pt11_3lev"].paragraph_format.alignment = justify_low
    styles_dict["Work_paragraph_pt12_3lev"].paragraph_format.alignment = justify_low


def add_style(
    document,
    styles_dict,
    meta_styles,
    name,
    family,
    pt,
    tabstops=None,
    first_line_indent=None,
    space_before=None,
    space_after=None,
    alignment=None,
    left_indent=None,
    right_indent=None,
    line_spacing=None,
    line_spacing_rule=None,
    color=None,
    is_paragraph=True,
    is_italic=False,
):
    pt = Pt(pt)
    styles = document.styles
    style = styles.add_style(
        name, WD_STYLE_TYPE.PARAGRAPH if is_paragraph else WD_STYLE_TYPE.CHARACTER
    )
    styles_dict[name] = style
    needbold = meta_styles[family + "_need_bold"]

    if is_paragraph:
        style.paragraph_format.line_spacing = 1.15

    style.font.name = family
    style.font.size = pt
    style.font.bold = needbold
    style.font.italic = is_italic
    if color:
        style.font.color.rgb = color
    if is_paragraph:
        paragraph_format = style.paragraph_format
        if line_spacing is not None:
            paragraph_format.line_spacing = line_spacing
        if line_spacing_rule is not None:
            paragraph_format.line_spacing_rule = line_spacing_rule
        if alignment is not None:
            paragraph_format.alignment = alignment
        if left_indent is not None:
            paragraph_format.left_indent = Cm(left_indent)
        if right_indent is not None:
            paragraph_format.right_indent = Cm(right_indent)
        if space_after is not None:
            paragraph_format.space_after = Pt(space_after)
        if space_before is not None:
            paragraph_format.space_before = Pt(space_before)
        if first_line_indent is not None:
            paragraph_format.first_line_indent = Cm(first_line_indent)
            if first_line_indent < 0 and left_indent is None:
                paragraph_format.left_indent = Cm(-first_line_indent)

        if tabstops:
            for i in tabstops:
                if isinstance(i, TS):
                    ts = paragraph_format.tab_stops.add_tab_stop(Cm(i.cm))
                    ts.alignment = i.alignment
                else:
                    paragraph_format.tab_stops.add_tab_stop(Cm(i))


def set_cell_style(
    cell, border_color, top, bottom, start, end, val_t, val_b, val_s, val_e
):
    set_cell_border(
        cell,
        top={"sz": 2, "val": val_t, "color": border_color, "space": "0"}
        if top
        else {"sz": 2, "val": "nil"},
        bottom={"sz": 2, "val": val_b, "color": border_color, "space": "0"}
        if bottom
        else {"sz": 2, "val": "nil"},
        start={"sz": 2, "val": val_s, "color": border_color, "space": "0"}
        if start
        else {"sz": 2, "val": "nil"},
        end={"sz": 2, "val": val_e, "color": border_color, "space": "0"}
        if end
        else {"sz": 2, "val": "nil"},
    )


def space_separated_fmt(value):
    return "{0:,}".format(int(value)).replace(",", "\u00A0")


def zero_converter_itog(x, k):
    if x is not None:
        result = int(round(x[k]))
        return space_separated_fmt(result) if result else None
    else:
        return None


def decorate_table_none(table):
    b_styles = {
        "first_col": (0, 0, 0, 0, "none", "none", "none", "none"),
        "last_col": (0, 0, 0, 0, "none", "none", "none", "none"),
        "first_row": (0, 0, 0, 0, "none", "none", "none", "none"),
        "first_row_cell_first": (0, 0, 0, 0, "none", "none", "none", "none"),
        "first_row_cell_last": (0, 0, 0, 0, "none", "none", "none", "none"),
        "none": (0, 0, 0, 0, "none", "none", "none", "none"),
        "last_row": (0, 0, 0, 0, "none", "none", "none", "none"),
        "last_row_cell_first": (0, 0, 0, 0, "none", "none", "none", "none"),
        "last_row_cell_last": (0, 0, 0, 0, "none", "none", "none", "none"),
    }
    decorate_table_int(table, 0xFFFFFF, 0xFFFFFF, b_styles, False, None)


def decorate_table_separated(
    table, border_color, cell_color, brief=False, cell_color_head=None, nolast=False
):
    if not cell_color_head:
        cell_color_head = cell_color
    border_int = "dashSmallGap"
    l = 1 if not nolast else 0
    b_styles = {
        "first_col": (1, 1, 1, 0, "single", "single", "single", "single"),
        "last_col": (1, 1, 0, 1, "single", "single", "single", "single"),
        "first_row": (1, 1, 0, 0, "single", border_int, "single", "single"),
        "first_row_cell_first": (1, 1, 1, 0, "single", border_int, "single", "single"),
        "first_row_cell_last": (1, 1, 0, 1, "single", border_int, "single", "single"),
        "none": (1, 1, 0, 0, border_int, border_int, "single", "single"),
        "last_row": (l, 1, 0, 0, border_int, "single", "single", "single"),
        "last_row_cell_first": (l, 1, 1, 0, border_int, "single", "single", "single"),
        "last_row_cell_last": (l, 1, 0, 1, border_int, "single", "single", "single"),
    }
    if not brief:
        b_styles["last_row"] = (l, 1, 0, 0, border_int, "single", "single", "single")
        b_styles["last_row_cell_first"] = (l, 1, 1, 0, border_int, "single", "single", "single", )
        b_styles["last_row_cell_last"] = (l, 1, 0, 1, border_int, "single", "single", "single", )
    decorate_table_int(
        table, border_color, cell_color, b_styles, brief, cell_color_head
    )


def decorate_table(
    table, border_color, cell_color, brief=False, cell_color_head=None, nolast=False
):
    if not cell_color_head:
        cell_color_head = cell_color
    border_int = "dashSmallGap"
    l = 1 if not nolast else 0
    b_styles = {
        "first_col": (0, 0, 1, 0, "single", "single", "single", "single"),
        "last_col": (0, 0, 0, 1, "single", "single", "single", "single"),
        "first_row": (1, 1, 0, 0, "single", border_int, "single", "single"),
        "first_row_cell_first": (1, 1, 1, 0, "single", border_int, "single", "single"),
        "first_row_cell_last": (1, 1, 0, 1, "single", border_int, "single", "single"),
        "none": (0, 0, 0, 0, "single", "single", "single", "single"),
        "last_row": (l, 1, 0, 0, border_int, "single", "single", "single"),
        "last_row_cell_first": (l, 1, 1, 0, border_int, "single", "single", "single"),
        "last_row_cell_last": (l, 1, 0, 1, border_int, "single", "single", "single"),
    }
    if not brief:
        b_styles["last_row"] = (l, 1, 0, 0, border_int, "single", "single", "single")
        b_styles["last_row_cell_first"] = ( l, 1, 1, 0, border_int, "single", "single", "single", )
        b_styles["last_row_cell_last"] = ( l, 1, 0, 1, border_int, "single", "single", "single", )
    decorate_table_int(
        table, border_color, cell_color, b_styles, brief, cell_color_head
    )


def decorate_table_price(
    table, border_color, cell_color, brief=False, cell_color_head=None, nolast=False
):
    if not cell_color_head:
        cell_color_head = cell_color
    border_int = "dashSmallGap"
    l = 1 if not nolast else 0
    b_styles = {
        "first_col": (1, 1, 1, 1, border_int, border_int, "single", border_int),
        "last_col": (1, 1, 1, 1, border_int, border_int, border_int, "single"),
        "first_row": (1, 1, 1, 1, "single", border_int, border_int, border_int),
        "first_row_cell_first": ( 1, 1, 1, 1, "single", border_int, "single", border_int, ),
        "first_row_cell_last": (1, 1, 1, 1, "single", border_int, border_int, "single"),
        "none": (1, 1, 1, 1, border_int, border_int, border_int, border_int),
        "last_row": (l, 1, 1, 1, border_int, "single", border_int, border_int),
        "last_row_cell_first": (l, 1, 1, 1, border_int, "single", "single", border_int),
        "last_row_cell_last": (l, 1, 1, 1, border_int, "single", border_int, "single"),
    }
    decorate_table_int(
        table, border_color, cell_color, b_styles, brief, cell_color_head
    )


def decorate_table_int(
    table, border_color, cell_color, b_styles, brief=False, cell_color_head=None
):
    for i, row in enumerate(table.rows):
        if not i % 2 and not brief:
            for c in row.cells:
                c._tc.get_or_add_tcPr().append(shading_elem(cell_color))

        for c in row.cells:
            set_cell_style(c, border_color, *b_styles["none"])

        set_cell_style(row.cells[0], border_color, *b_styles["first_col"])
        set_cell_style(row.cells[-1], border_color, *b_styles["last_col"])

    for c in table.rows[0].cells:
        c._tc.get_or_add_tcPr().append(shading_elem(cell_color_head))
        set_cell_style(c, border_color, *b_styles["first_row"])
    for c in table.rows[-1].cells:
        if brief:
            c._tc.get_or_add_tcPr().append(shading_elem(cell_color_head))
        set_cell_style(c, border_color, *b_styles["last_row"])

    set_cell_style(
        table.rows[0].cells[0], border_color, *b_styles["first_row_cell_first"]
    )
    set_cell_style(
        table.rows[0].cells[-1], border_color, *b_styles["first_row_cell_last"]
    )

    set_cell_style(
        table.rows[-1].cells[0], border_color, *b_styles["last_row_cell_first"]
    )
    set_cell_style(
        table.rows[-1].cells[-1], border_color, *b_styles["last_row_cell_last"]
    )

    set_repeat_table_header(table.rows[0])


class ImageDescription:
    def __init__(
        self,
        fname,
        description,
        width,
        height,
        need_pre_pagebreak,
        need_post_pagebreak,
        need_image_prefix=False,
        pspace=6,
        paspace=6,
        aspace=3,
    ):
        # описание рисунка
        self.fname = fname
        # текст заголовка
        self.description = description
        # ширина
        self.width = width
        # высота
        self.height = height
        # обрыв страницы перед рисунком
        self.need_pre_pagebreak = need_pre_pagebreak
        # обрыв страницы после рисунка
        self.need_post_pagebreak = need_post_pagebreak
        # нужен ли заголовок рисунка
        self.need_image_prefix = need_image_prefix
        # отступ перед заголовком
        self.pspace = pspace
        # отступ после рисунка
        self.aspace = aspace
        # отступ после заголовка
        self.paspace = paspace

    def __str__(self):
        return f"""{self.fname} {self.description} {self.width} {self.height} {self.need_pre_pagebreak} {
                    self.need_post_pagebreak} {self.need_image_prefix} {self.pspace} {self.aspace} {self.paspace}"""
