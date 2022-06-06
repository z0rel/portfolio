# distutils: language = c++
# cython: language_level=3


# from xlcommercial.calc.estimate.commercial_config_base import ConfigBase

from libcpp.unordered_map cimport unordered_map
from libcpp.memory cimport shared_ptr
from libcpp.vector cimport vector
from libcpp.string cimport string
from commercial_role cimport RoleParsingContext
from commercial_role cimport CathegoryPosition
from aggregate_commercial_internal cimport RowProperties

cpdef str tr_description(description)

cdef class AdditionalTkpData:
    # текущая категория доп. данных - прайсовые данные или отдельного ТКП
    cdef public bint is_price;
    # описание изображений
    cdef public list images
    # описание элементов краткого описания страницы №1
    cdef public list brief_items_p0
    # описание элементов краткого описания страницы №2
    cdef public list brief_items_p1
    # описание элементов краткого описания страницы №2
    cdef public set brief_items_row_indices
    # элементы дополнительных условий платежа
    cdef public list brief_payment_additional
    # преамбула ТКП (заголовок)
    cdef public str brief_preamble_after_tkp
    # дополнительные условия поставки после гарантийных условий
    cdef public additional_statement_after_guarantee
    # не выводить предложение о банковской гарантии на возврат аванса
    cdef public bint nobgavance
    # не выводить срок действия ценового предложения
    cdef public bint no_out_deadline
    # описание варианта модернизации в строке прайс-листа
    cdef public str price_table_brief_cell
    # рисунок в заголовке предложения
    cdef public object header_image
    # ООО предлагает ...
    cdef public str first_brief
    # элементы дополнительных условий технико-коммерческого предложения
    cdef public list additional_statements_tkp
    # имя файла ТКП
    cdef public str tkp_filename
    # вставить разрыв страницы перед таблицей затрат по ролям
    cdef public bint break_before_roles_table
    # вставить разрыв страницы перед таблицей материалов
    cdef public bint break_before_material_table
    # вставить разрыв страницы перед таблицей трудозатрат
    cdef public bint break_before_trud_table
    # вставить разрыв страницы перед таблицей прочих затрат
    cdef public bint break_before_other_table
    # вставить разрыв страницы перед подписью в калькуляции
    cdef public bint break_before_calculation_brief
    # интервал перед заголовком КП
    cdef public int spacer_before_title
    # интервал перед леммой КП
    cdef public int spacer_before_lemma


cdef class ParsingContext:
    # Словарь: Индексированная должность: номер строки в заголовке - (должность, зарплата, номер строки в калькуляции)
    cdef unordered_map[int, shared_ptr[RoleParsingContext] ]  indexed_pairs_role_to_salary
    # Позиция ИП в строке TODO: может стать set
    cdef int ip_position
    # Индекс столбца с номером категории
    cdef int idx_col_cathegory_number
    # Индекс столбца 'Техническое описание работ'
    cdef int idx_col_workname
    # Индекс столбца 'Комплектующие'
    cdef int idx_col_complect_price
    # Индекс столбца 'пометки'
    cdef int idx_col_markers
    # контейнер собранных категорий
    cdef public list aggregated_works
    # длина строки листа excel с данными о трудоемкости
    cdef Py_ssize_t len_zp_values
    # позиция строки для заполнения локальными корректурами
    cdef int local_risk_zp_positions
    # объем комплектующих в долларах
    cdef public double compl_in_usd
    # объем комплектующих в евро
    cdef public double compl_in_eur

    # дополнительные данные для технико-коммерчиского предложения
    cdef public AdditionalTkpData commercial
    # дополнительные данные для прайс-листа
    cdef public AdditionalTkpData price
    # конфигурация текущего обсчитываемого варианта сметы
    cpdef public object cfg

    cpdef double get_summary_local_risk(self)
    cdef void make_parsing_context(self, list src_data, int cathegory_column) except*


cdef class AggregatorObject:
    cdef unordered_map[double, int] aggregated_works
    cdef double div_coeff
    cpdef object cfg
    cdef vector[shared_ptr[CathegoryPosition] ] cathegories_list
    cdef list result_unsorted
    cdef ParsingContext ctx

    cpdef parse(self, list src_data, list additional_data, int cathegory_column)
    cdef bint check_marker(self, str scathegory, AdditionalTkpData dst,
                           str description, list row_of_calculation, object current_compl_price_obj, int row_idx) except*

    cdef int aggregate_row(self, int pos, object cathegory_of_row, str workname, double compl_price, list row_of_calculation,
                           RowProperties row_props, int idx, object cfg) except*
