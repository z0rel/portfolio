# distutils: language = c++
# cython: language_level=3


import re
import traceback
import sys

from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.unordered_map cimport unordered_map
from libcpp.memory cimport shared_ptr
from commercial_role cimport RoleParsingContext
from commercial_role cimport CathegoryPosition
from libcpp.algorithm cimport sort
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.utility cimport pair
from xlcommercial.calc.estimate.wordgen.word_styles import ImageDescription
from aggregate_commercial_internal cimport Cathegory

# from xlcommercial.calc.estimate.commercial_config_base import ConfigBase

cdef object RoleParsingContext_getstate(RoleParsingContext *self):
    return (self.role_name.decode(), self.salary_value, self.role_position_on_calculation)

cdef void RoleParsingContext_setstate(RoleParsingContext *self, object state):
    (role_name, salary_value, role_position_on_calculation) = state
    self.role_name = role_name.encode()
    self.salary_value = salary_value
    self.role_position_on_calculation = role_position_on_calculation


cdef class AdditionalTkpData:
    def __cinit__(self, bint is_price=False):
        self.is_price = is_price
        self.images = []
        self.brief_items_p0 = []
        self.brief_items_p1 = []
        self.brief_items_row_indices = set()
        self.brief_payment_additional = []
        self.brief_preamble_after_tkp = ''
        self.additional_statement_after_guarantee = []
        self.nobgavance = False
        self.no_out_deadline = False
        self.price_table_brief_cell = ''
        self.header_image = None
        self.first_brief = ''
        self.additional_statements_tkp = []
        self.tkp_filename = ''
        self.break_before_roles_table = False
        self.break_before_material_table = False
        self.break_before_trud_table = False
        self.break_before_other_table = False
        self.break_before_calculation_brief = False
        self.spacer_before_title = -1
        self.spacer_before_lemma = 12

    def __getstate__(self):
        return (
            self.is_price,
            self.images,
            self.brief_items_p0,
            self.brief_items_p1,
            self.brief_payment_additional,
            self.brief_preamble_after_tkp,
            self.additional_statement_after_guarantee,
            self.nobgavance,
            self.no_out_deadline,
            self.price_table_brief_cell,
            self.header_image,
            self.first_brief,
            self.additional_statements_tkp,
            self.tkp_filename,
            self.break_before_roles_table,
            self.break_before_material_table,
            self.break_before_trud_table,
            self.break_before_other_table,
            self.break_before_calculation_brief,
            self.spacer_before_title,
            self.spacer_before_lemma
        )

    def __setstate__(self, state):
        (
            is_price,
            images,
            brief_items_p0,
            brief_items_p1,
            brief_payment_additional,
            brief_preamble_after_tkp,
            additional_statement_after_guarantee,
            nobgavance,
            no_out_deadline,
            price_table_brief_cell,
            header_image,
            first_brief,
            additional_statements_tkp,
            tkp_filename,
            break_before_roles_table,
            break_before_material_table,
            break_before_trud_table,
            break_before_other_table,
            break_before_calculation_brief,
            spacer_before_title,
            spacer_before_lemma
         ) = state

        self.is_price = is_price
        self.images = images
        self.brief_items_p0 = brief_items_p0
        self.brief_items_p1 = brief_items_p1
        self.brief_payment_additional = brief_payment_additional
        self.brief_preamble_after_tkp = brief_preamble_after_tkp
        self.additional_statement_after_guarantee = additional_statement_after_guarantee
        self.nobgavance = nobgavance
        self.no_out_deadline = no_out_deadline
        self.price_table_brief_cell = price_table_brief_cell
        self.header_image = header_image
        self.first_brief = first_brief
        self.additional_statements_tkp = additional_statements_tkp
        self.tkp_filename = tkp_filename
        self.break_before_roles_table = break_before_roles_table
        self.break_before_material_table = break_before_material_table
        self.break_before_trud_table = break_before_trud_table
        self.break_before_other_table = break_before_other_table
        self.break_before_calculation_brief = break_before_calculation_brief
        self.spacer_before_title = spacer_before_title
        self.spacer_before_lemma = spacer_before_lemma



cdef class ParsingContext:
    def __cinit__(self):
        self.idx_col_complect_price = -1
        self.idx_col_workname = -1
        self.idx_col_cathegory_number = -1
        self.ip_position = -1
        self.idx_col_markers = -1
        self.local_risk_zp_positions = -1
        self.aggregated_works = []
        self.compl_in_usd = 0.0
        self.compl_in_eur = 0.0

        self.commercial = AdditionalTkpData(False)
        self.price = AdditionalTkpData(True)

    def __getstate__(self):
        dst_map = {}
        cdef unordered_map[int, shared_ptr[RoleParsingContext] ].iterator it = self.indexed_pairs_role_to_salary.begin()
        while it != self.indexed_pairs_role_to_salary.end():
            dst_map[deref(it).first] = RoleParsingContext_getstate(deref(it).second.get())
            inc(it)

        return (dst_map, self.ip_position, self.idx_col_cathegory_number, self.idx_col_workname,
                self.idx_col_complect_price, self.idx_col_markers, self.aggregated_works, self.len_zp_values,
                self.local_risk_zp_positions, self.compl_in_usd, self.compl_in_eur,
                self.commercial, self.price, self.cfg)

    def __setstate__(self, state):
        cdef pair[int, shared_ptr[RoleParsingContext]] val

        (dst_map, ip_position, idx_col_cathegory_number, idx_col_workname, idx_col_complect_price, idx_col_markers,
         aggregated_works, len_zp_values, local_risk_zp_positions, compl_in_usd, compl_in_eur,
         commercial, price, cfg) = state

        for i,v in dst_map.items():
            val.first = i
            val.second = shared_ptr[RoleParsingContext](new RoleParsingContext())
            RoleParsingContext_setstate(val.second.get(), v)
            self.indexed_pairs_role_to_salary.insert(val)

        self.ip_position              = ip_position
        self.idx_col_cathegory_number = idx_col_cathegory_number
        self.idx_col_workname         = idx_col_workname
        self.idx_col_complect_price   = idx_col_complect_price
        self.idx_col_markers          = idx_col_markers
        self.aggregated_works         = aggregated_works
        self.len_zp_values            = len_zp_values
        self.local_risk_zp_positions  = local_risk_zp_positions
        self.compl_in_usd             = compl_in_usd
        self.compl_in_eur             = compl_in_eur
        self.commercial               = commercial
        self.price                    = price
        self.cfg                      = cfg

    cpdef double get_summary_local_risk(self):
        cdef double res = 0.0;
        cdef Cathegory wrk
        for wrk in self.aggregated_works:
            res += wrk.get_local_correcture()
        return res

    cdef void make_parsing_context(self, list src_data, int cathegory_column) except*:
        cdef shared_ptr[RoleParsingContext] it_role_context
        cdef int i = -1
        cdef int first_role_pos = -1
        cdef int current_role_position
        cdef int last_nonip_position = -1

        cdef list zarplata_values = src_data[0] # Значения зарплаты
        cdef list role_positions = src_data[1] # Значения позиций должностей
        cdef list heads_of_columns = src_data[2] # Заголовки столбцов

        cdef object it = None
        cdef object val = None

        self.len_zp_values = len(zarplata_values)

        if self.len_zp_values:
            it = iter(zarplata_values)
            next(it)
            for i in range(1, self.len_zp_values):
                val = next(it)
                if val == 'ZP':
                    first_role_pos = i + 1
                    break

            if first_role_pos < 0:
                it = iter(zarplata_values)
                next(it)
                first_role_pos = 1

            try:
                for i in range(first_role_pos, self.len_zp_values):
                    val = next(it)
                    if val:
                        n = i
                        if role_positions[i] is None:
                            print('ERROR: Role position is None, i = ', i)
                        current_role_position = int(role_positions[i])
                        self.indexed_pairs_role_to_salary[i] = (shared_ptr[RoleParsingContext])(new RoleParsingContext(
                            heads_of_columns[i].encode(), # Кортежи 'должность, зарплата', 'позиция в калькуляции'
                            float(val),
                            current_role_position
                        ))
                        if role_positions[i+1] == 'IP':
                            self.ip_position = i
                        else:
                            last_nonip_position = i
                        if current_role_position == 1:
                            self.local_risk_zp_positions = i # по умолчанию - локальные корректуры делаются зарплатой директора

                    if self.local_risk_zp_positions < 0 <= last_nonip_position:
                        self.local_risk_zp_positions = last_nonip_position
            except TypeError as exc:
                print(traceback.extract_tb(sys.exc_info()[2],limit=1))
                print('TYPE ERROR:', str(exc))
                print('Role position: ', i)
                raise



        cdef int par_values = 0
        cdef Py_ssize_t len_heads_of_columns = len(heads_of_columns)

        if cathegory_column >= 0:
            self.idx_col_cathegory_number = cathegory_column
            par_values += 1

        if len_heads_of_columns:
            it = iter(heads_of_columns)
            for i in range(0, len_heads_of_columns):
                val = next(it)
                if par_values > 3:
                    break
                if self.idx_col_cathegory_number < 0 and val == "Категория":
                    self.idx_col_cathegory_number = i
                    par_values += 1
                if val == "Техническое описание работ":
                    self.idx_col_workname = i
                    par_values += 1
                if val == "Комплектующие":
                    self.idx_col_complect_price = i
                    par_values += 1
                if val == "m":
                    self.idx_col_markers = i
                    par_values += 1

        if self.idx_col_markers < 0 <= self.idx_col_cathegory_number:
            self.idx_col_markers = self.idx_col_cathegory_number + 1 # поведение по умолчанию для старых расчетов
            print('old-style col marker: ', self.idx_col_markers)


cdef bint cathegories_comparator(const shared_ptr[CathegoryPosition] &first, const shared_ptr[CathegoryPosition] &second):
    return deref(first).dcathegory < deref(second).dcathegory



cpdef list PREDLOGS = [
    'а', 'бы', 'в', 'вопреки', 'время', 'вроде', 'все', 'да', 'дабы', 'даже', 'даром', 'для', 'едва', 'если', 'еще',
    'же', 'затем', 'зато', 'зачем', 'и', 'как', 'лишь', 'не', 'но', 'с', 'так', 'тем', 'то', 'тогда', 'того', 'том',
    'тому', 'чего', 'что', 'чтобы', 'на', 'из', 'под', 'то', 'либо', 'нибудь', 'ли', 'г[.]', 'ТК', 'по',
]

cpdef str SEP = r'\b|\b'
cpdef object RE_REPLACEMENT_SPACE = re.compile(f'({SEP.join(PREDLOGS)})[ ]', re.IGNORECASE)
cpdef object RE_REPLACEMENT_BR_BEFORE = re.compile(f'-({SEP.join(PREDLOGS)})', re.IGNORECASE)
cpdef object RE_REPLACEMENT_BR_AFTER = re.compile(f'({SEP.join(PREDLOGS)})-', re.IGNORECASE)


cpdef str tr_description(description):
    description = RE_REPLACEMENT_SPACE.sub(r'\1 ', description)
    description = RE_REPLACEMENT_BR_BEFORE.sub(r'‑\1', description)
    description = RE_REPLACEMENT_BR_AFTER.sub(r'\1‑', description)
    return (description
            .replace('<NBR>', ' ')
            .replace('<NBD>', '‑')
            .replace('<BR>', '\n')
            .replace('<TAB>', '\t')
            .replace('&plusmn;', '±')
            .replace('&laquo;', '«')
            .replace('&raquo;', '»')
            .replace('&nbsp;', ' ')
            .replace('&deg;', '°')
            .replace('&quot;', '"')
            .replace('&Prime;', '″')
            .replace('&mdash;', '–')
            )


cdef class AggregatorObject:
    def __cinit__(self, double div_coeff, object cfg):
        self.div_coeff = div_coeff
        self.cfg = cfg  # config of project
        self.result_unsorted = []
        self.ctx = ParsingContext()
        self.ctx.cfg = cfg

    cdef bint check_marker(self, str scathegory, AdditionalTkpData dst,
                           str description, list row_of_calculation, object current_compl_price_obj, int row_idx) except*:
        cdef object width
        cdef object height
        cdef object image_description

        if scathegory == 'I':
            dst.brief_items_p0.append((0, description, row_idx))
            dst.brief_items_row_indices.add(row_idx)
            return True
        elif scathegory == 'I1':
            dst.brief_items_p0.append((1, description, row_idx))
            dst.brief_items_row_indices.add(row_idx)
            return True
        elif scathegory == 'NOBG': # не выводить информацию о банковской гарантии на возврат аванса
            dst.nobgavance = True
            return True
        elif scathegory == 'NODD': # не выводить срок действия ТКП
            dst.no_out_deadline = True
            return True
        elif scathegory == 'PRB': # Краткое описание в таблице прайс-листа
            dst.price_table_brief_cell = description
            return True
        elif scathegory == 'II':
            dst.brief_items_p1.append((0, description, row_idx))
            dst.brief_items_row_indices.add(row_idx)
            return True
        elif scathegory == 'II1':
            dst.brief_items_p1.append((1, description, row_idx))
            dst.brief_items_row_indices.add(row_idx)
            return True
        elif scathegory == 'FB': # краткое описание в начале ТКП под логотипом
            dst.first_brief = description
            return True
        elif scathegory == 'BR_ROLES':
            dst.break_before_roles_table = True
            return True
        elif scathegory == 'BR_TRUD':
            dst.break_before_trud_table = True
            return True
        elif scathegory == 'BR_OTHER':
            dst.break_before_other_table = True
            return True
        elif scathegory == 'BR_MATERIAL':
            dst.break_before_material_table = True
            return True
        elif scathegory == 'BR_CALC_BRIEF':
            dst.break_before_calculation_brief = True
            return True
        elif scathegory == 'IM' or scathegory == 'IMH': # изображение или изображение станка
            width = row_of_calculation[self.ctx.idx_col_workname + 1]
            height = row_of_calculation[self.ctx.idx_col_workname + 2]
            image_description = ImageDescription(
                str(current_compl_price_obj),
                description,
                float(width or 0),
                float(height or 0),
                int(float(row_of_calculation[self.ctx.idx_col_workname + 3] or 0)),
                int(float(row_of_calculation[self.ctx.idx_col_workname + 4] or 0)),
                row_of_calculation[self.ctx.idx_col_workname + 5],
                row_of_calculation[self.ctx.idx_col_workname + 6],
                row_of_calculation[self.ctx.idx_col_workname + 7],
                row_of_calculation[self.ctx.idx_col_workname - 1]
            )
            if scathegory == 'IM':
                # print('parsing_context.pyx:347', dst.is_price, str(image_description))
                dst.images.append(image_description)
            else:
                dst.header_image = image_description
            return True
        elif scathegory == 'IH': # Лемма после заголовка ТКП
            dst.brief_preamble_after_tkp = description
            return True
        elif scathegory == 'IA': # Дополнительные условия оплаты
            dst.brief_payment_additional.append(description)
            return True
        elif scathegory[:2] == 'IA': # Порядок расрочки платежей
            pass
        elif scathegory == 'IZ': # Дополнительные предложения после информации о гарантии
            if description:
                dst.additional_statement_after_guarantee.append(description)
            return True
        elif scathegory == 'TKP_AS': # Дополнительные условия ТКП
            if description:
                dst.additional_statements_tkp.append(description)
            return True
        elif scathegory == 'FNAME': # Дополнительные условия ТКП
            dst.tkp_filename = str(description)
            return True
        elif scathegory == 'HTITLE': # Интервал перед заголовком КП
            dst.spacer_before_title = int(float(description))
            return True
        elif scathegory == 'HLEMMA': # Интервал перед леммой КП
            dst.spacer_before_lemma = int(float(description))
            return True


        return False


    cpdef parse(self, list src_data, list additional_data, int cathegory_column):
        cdef int idx = -10
        cdef Py_ssize_t len_data_calculation_body = len(src_data)
        cdef object it = None
        cdef list row_of_calculation = None
        cdef object cathegory_of_row
        cdef vector[shared_ptr[CathegoryPosition] ].iterator cathegory_list_it
        cdef int pos = 0
        cdef object current_compl_price_obj
        cdef RowProperties empty_row_props = RowProperties()
        cdef RowProperties local_risk_row_props = RowProperties()
        cdef list local_risk_row = None
        cdef RowProperties current_row_props
        cdef list current_local_risk_row = None
        cdef str marker
        cdef double current_compl_price
        cdef object row3
        cdef double calc_euro_cource
        cdef double calc_usd_cource
        cdef str marker_valuta = ''
        cdef object marker_valuta_object
        cdef double sum_compl_euro = 0.0
        cdef double sum_compl_usd = 0.0
        cdef str scathegory_of_row
        cdef str scathegory_of_marker
        cdef str description
        cdef bint tcontinue0 = False
        cdef bint tcontinue1 = False
        cdef object obj_description = None
        # cdef bint convert_to_euro = False
        cdef double conv_euro_cource = 1.0
        cdef int additional_rownum = -1
        cdef bint is_price_row


        self.ctx.make_parsing_context(src_data, cathegory_column)
        if cathegory_column >= self.ctx.idx_col_markers:
            return None
        calc_euro_cource = self.cfg.calc_eur
        calc_usd_cource = self.cfg.calc_usd

        # convert_to_euro = self.cfg.convert_to_euro
        # if convert_to_euro:
        #     calc_usd_cource =  calc_euro_cource / calc_usd_cource
        #     conv_euro_cource = 1.0 / calc_euro_cource
        #     calc_euro_cource = 1.0

        local_risk_row_props.is_local_risk = True
        local_risk_row_props.local_risk_zp_positions = self.ctx.local_risk_zp_positions

        try:
            if self.ctx.ip_position < 0:
                print("! ERROR !: need to enter IP position")

            if len_data_calculation_body > 3:
                it = iter(src_data)
                next(it)
                next(it)
                next(it)

                for idx in range(3, len_data_calculation_body):
                    row_of_calculation = next(it)
                    cathegory_of_row = row_of_calculation[self.ctx.idx_col_cathegory_number]
                    # Добавить работу в аггрегированный список работ для заданной категории работ, посчитать ее трудоемкость по профессиям и стоимость комплектующих
                    current_compl_price_obj = row_of_calculation[self.ctx.idx_col_complect_price]

                    obj_description = row_of_calculation[self.ctx.idx_col_workname]
                    description = tr_description(str(obj_description)) if obj_description is not None else ''
                    scathegory_of_marker = str(row_of_calculation[self.ctx.idx_col_markers])

                    tcontinue0 = False
                    if cathegory_of_row:
                        scathegory_of_row = str(cathegory_of_row)
                        tcontinue0 = self.check_marker(scathegory_of_row, self.ctx.commercial, description, row_of_calculation, current_compl_price_obj, idx)

                    tcontinue1 = self.check_marker(scathegory_of_marker, self.ctx.price, description, row_of_calculation, current_compl_price_obj, idx)

                    if tcontinue0 or tcontinue1 or not cathegory_of_row:
                        continue

                    current_compl_price = float(current_compl_price_obj) if current_compl_price_obj else 0.0 # цена комплектующих

                    marker_valuta_object = row_of_calculation[self.ctx.idx_col_markers + 1]
                    if isinstance(marker_valuta_object, str):
                        marker_valuta = marker_valuta_object
                    else:
                        marker_valuta = ''

                    if marker_valuta == 'U':
                        sum_compl_usd += current_compl_price
                        current_compl_price *= calc_usd_cource
                    elif marker_valuta == 'E':
                        sum_compl_euro += current_compl_price
                        current_compl_price *= calc_euro_cource
                    # else:
                    #     current_compl_price *= conv_euro_cource

                    pos = self.aggregate_row(pos,
                                             cathegory_of_row, # категория
                                             description, # описание
                                             current_compl_price,
                                             row_of_calculation,
                                             None,
                                             idx,
                                             self.cfg)

                self.ctx.compl_in_usd = sum_compl_usd
                self.ctx.compl_in_eur = sum_compl_euro


            if additional_data:
                additional_rownum = 0
                for row in additional_data:
                    idx += 1
                    additional_rownum += 1
                    # print(row)
                    marker = row[1]
                    row3 = row[3]
                    cathegory_of_row = row[2]

                    if cathegory_of_row is not None and row3 is not None:
                        tcontinue0 = self.check_marker(str(cathegory_of_row), self.ctx.commercial, str(row3), None, None, idx)
                        if tcontinue0:
                            continue

                    if marker == 'RC':
                        current_row_props = local_risk_row_props
                        current_local_risk_row = None
                        current_compl_price = float(row3) * self.div_coeff if row3 else 0.0 # цена комплектующих
                    elif marker == 'RW':
                        current_row_props = local_risk_row_props
                        if local_risk_row is None:
                            local_risk_row = [0 for i in range(0, self.ctx.len_zp_values)]
                        local_risk_row[self.ctx.local_risk_zp_positions+1] = float(row3) if row3 else 0.0
                        local_risk_row[self.ctx.local_risk_zp_positions] = 1.0
                        current_local_risk_row = local_risk_row
                        current_compl_price = 0.0
                    else:
                        current_row_props = empty_row_props
                        current_local_risk_row = None
                        current_compl_price = float(row3) * self.div_coeff if row3 else 0.0 # цена комплектующих

                    is_price_row = True
                    # Условия рассрочки платежей
                    if isinstance(cathegory_of_row, str):
                        if cathegory_of_row[:3] == 'IAA':
                            if len(cathegory_of_row) > 2:
                                self.cfg.avance_graph_list[int(cathegory_of_row[3:])] = int(row3)
                            is_price_row = False
                        elif cathegory_of_row == 'SKIP_IN_PIRCE':
                            self.cfg.skip_work_in_price = True
                            is_price_row = False
                        elif cathegory_of_row == 'DETAIL':
                            self.cfg.out_calculation_detailed_view = 1
                            is_price_row = False


                    if is_price_row:
                        pos = self.aggregate_row(pos,
                                                 cathegory_of_row, # категория
                                                 row[0], # описание
                                                 current_compl_price,
                                                 current_local_risk_row,
                                                 current_row_props,
                                                 idx,
                                                 self.cfg)

            sort(self.cathegories_list.begin(), self.cathegories_list.end(), cathegories_comparator)
            cathegory_list_it = self.cathegories_list.begin()

            while cathegory_list_it != self.cathegories_list.end():
                self.ctx.aggregated_works.append(self.result_unsorted[deref(deref(cathegory_list_it)).position])
                inc(cathegory_list_it)

            return self.ctx

        except Exception as exc:
            print("!!! EXCEPTION aggregate_commercial_internal: ", str(exc))
            print("!!! cathegory_column: ", cathegory_column)
            print(f'idx: {idx + 1}, row_of_calculation: {row_of_calculation}, additional rownum: {additional_rownum}, len of additional_data: {len(additional_data)}')
            raise exc


    cdef int aggregate_row(self, int pos, object cathegory_of_row, str workname, double compl_price, list row_of_calculation,
                           RowProperties row_props, int idx, object cfg) except*:
        cdef pair[double, int] inserted_value
        cdef shared_ptr[CathegoryPosition] cathegory_current
        cdef Cathegory aggregate_work_object
        cdef str scathegory
        cdef double fcathegory
        cdef pair[unordered_map[double, int].iterator, bint] result_inserting

        #  Всё что не имеет заполненную категорию, не попадет в итоговый расчет
        if not cathegory_of_row:  # пропустить строки, категория которых незаполнена
            return pos

        if not cfg.out_calculation_detailed_view:
            scathegory = str(cathegory_of_row)
            fcathegory = float(cathegory_of_row)
        else:
            scathegory = str(cathegory_of_row)
            fcathegory = float(cathegory_of_row)
            if scathegory not in {'90000', '90002', '90003', '90053', '90100', '90101', '90900', '100052',
                                  '100053', '110001', '117000', '117001'}:
                if '.' in scathegory:
                    scathegory = "{0}{1}".format(scathegory, idx / 10000.0)[2:]
                else:
                    scathegory = "{0}{1}".format(scathegory, idx / 10000.0)[1:]
                fcathegory = float(cathegory_of_row) + idx / 10000.0

        inserted_value.first = fcathegory
        inserted_value.second = pos
        result_inserting = self.aggregated_works.insert(inserted_value)

        if not result_inserting.second:
            aggregate_work_object = self.result_unsorted[deref(result_inserting.first).second]
        else: # такой категории еще не существует
            # Добавить новое описание аггрегированной работы в словарь аггрегированных работ либо получить из него временную переменную, если такая аггрегированная работа уже существует
            aggregate_work_object = Cathegory().c_init(scathegory, fcathegory, self.ctx, self.cfg.euro_cource, self.cfg.zagran, self.div_coeff)
            assert(aggregate_work_object.descr is not None)

            cathegory_current = shared_ptr[CathegoryPosition](new CathegoryPosition(fcathegory, pos))
            self.result_unsorted.append(aggregate_work_object)
            self.cathegories_list.push_back(cathegory_current)
            pos += 1

        if row_props is None:
            row_props = aggregate_work_object.parse_rownumber(row_of_calculation, self.ctx.idx_col_markers)

        aggregate_work_object.add_descr_item(workname, compl_price, row_of_calculation, row_props)
        return pos
