# distutils: language = c++
# cython: language_level=3

cimport generate_commercial
from libcpp.unordered_map cimport unordered_map
from libcpp.memory cimport shared_ptr
from commercial_role cimport Role
from parsing_context cimport ParsingContext
cimport numpy as np
ctypedef np.float64_t DTYPE_t


cdef class SalaryEmployee:
    cdef public double salary_per_day # дневная зп работника
    cdef public double salary_summary
    cdef public double laboriousness_clear # чистая суммарная трудоемкость
    cdef public int rows_cnt # число строк пункта в таблице где присутствует работа для работника
    cdef public double employees_cnt # число указанных работников суммарно по всем строкам
    cdef public double salary_summary_corrected # скорректированная зарплата работника
    cdef public int position
    cdef public int is_ip # является ли работник ИП

    cpdef double get_salary_per_day(self)
    cpdef double get_salary_summary(self)
    cpdef double get_laboriousness_clear(self)
    cpdef double get_rows_cnt(self)
    cpdef double get_employees_cnt(self)
    cpdef double get_salary_summary_corrected_fot(self)
    cpdef double nakl_in_salary(self)
    cpdef double nakl_in_laboriousness(self)
    cpdef double salary_per_day_fot(self)
    cpdef double salary_summary_fot(self)
    cpdef laboriousness_corrected(self)

cdef class RowProperties:
    cdef str rownumber
    cdef bint is_local_risk
    cdef bint need_skip_descr
    cdef public int local_risk_zp_positions


cdef class Flags:
    # Значение категории
    cdef public double cathegory_val
    # Флаг - это транспортный риск
    cdef bint flag_is_transport
    # Флаг - транспортировка станка, материалов и компоенетов
    cdef bint flag_is_transport_materials

    cdef bint flag_is_transport_instruments
    cdef bint flag_is_transport_arenda
    cdef bint flag_is_transport_tickets
    # Флаг - это командировочные или суточные срасходы
    cdef bint flag_comandir_or_sutoch_rash
    # Зарплата всех работников кроме ИП
    cdef public double summary_zarplata_value
    # Зарплата всех ИП
    cdef public double summary_zarplata_value_ip
    # стоимость комплектующих
    cdef public double compl_price

    # Необходимо ли преобразовывать итоговые суммы в евро
    cdef bint need_convert_to_euro
    # Это заграничная командировка
    cdef bint zagran_comandir

    cpdef set_compl_price(self, double compl_price)
    cpdef is_transport_rash(self)
    cpdef is_transport_risk_nakl(self)
    cpdef bint is_transport_tickets(self)
    cpdef bint is_transport_arenda(self)
    cpdef bint is_transport_instruments(self)
    cpdef bint is_transport_materials(self)
    cpdef bint is_transport_risk(self)
    cpdef bint is_transport_nakl(self)
    cpdef bint is_bank_guarantee(self)
    cpdef bint is_cathegory(self, cathegory)
    cpdef bint is_naklad_comandir_or_sutoch_value(self)
    cpdef bint is_comandir_or_sutoch_value(self)
    cpdef bint is_work_or_compl(self)
    cpdef bint is_transport(self)
    cpdef bint is_nakl(self)
    cpdef bint is_inform(self)
    cpdef bint is_risks(self)
    cpdef bint is_work(self)
    cpdef bint is_complect(self)


cdef class SumPriceObj:
    cpdef public double summary_work
    cpdef public double summary_compl


cdef class Cathegory(Flags):
    # индекс профессии: трудоемкость, зарплата
    cdef unordered_map[int, shared_ptr[Role]] trud
    # Краткое описание категории
    cdef public str key
    # Полное описание категории
    cdef public list descr
    # Суммарный риск по комплектующим
    cdef public double local_correcture_compl
    # Суммарный риск по работам
    cdef public double local_correcture_work
    # Зарплата для локального риска
    cdef public double local_correcture_salary
    # Форматный счетчик
    cdef public int int_counter
    # Курс евро
    cdef public double euro_cource
    # Позиции маркеров разметки
    cdef int idx_col_markers
    # обратная ссылка на контейнер, хранящий итоги категории
    cdef public np.ndarray cb_itog_data
    # индекс строки в контейнере итога
    cdef public int cb_itog_data_idx
    # процент НДС
    cdef public int nds

    cpdef object c_init(self, str skey, double fkey, ParsingContext ctx, double euro_cource, int zagran_comandir, double valut_divide_coeff)

    # Коэффициент-делитель для конвертации валют
    cdef public double valut_divide_coeff
    cdef double __get_summary_zarplata_merge_trud(self)
    cdef double get_compl(self)
    cpdef add_description(self, str row)
    cdef void merge_trud_employees(self, list dst, double naklad_percent) except*
    cpdef double get_double_cathegory(self)
    cpdef double get_local_correcture(self)
    cpdef set_local_correcture(self, double val)
    cpdef double get_local_correcture_raw(self)
    cpdef double get_local_risk_salary(self)
    cdef RowProperties parse_rownumber(self, list row_of_calculation, int idx)

    cpdef add_descr_item(self, str export_row_workname, double compl_price, list row_of_calculation, RowProperties row_props)
    cpdef double get_summary_zarplata(self)
    cpdef double get_compl_price(self)
    cpdef double get_work_price(self)
    cpdef double get_itog(self)
    cpdef sum_compl_and_work_price(self, SumPriceObj dst)

    cdef double get_fot(self) except*
    cdef double __get_sutoch_zp_fot(self) except*
    cpdef double get_sutoch_fot(self)
    cdef double __get_summary_fot(self) except*
    cdef double get_fot_of_nakl_table_row(self)
    cdef public void append_cathegory_and_description(self, generate_commercial.ItogClass dst, int idx) except*
    cdef void _setup_dst_data_with_nakl(self, generate_commercial.ItogClass dst, int idx, DTYPE_t dst0, DTYPE_t dst2) except*
    cdef void _setup_dst_data_nakl_table_row(self, generate_commercial.ItogClass dst, int idx, DTYPE_t dst0, DTYPE_t dst2) except*


    cdef void expand_works_obj(self, generate_commercial.ItogClass dst, int idx) except*
    cdef void expand_compl_obj(self, generate_commercial.ItogClass dst, int idx) except*
    cdef void expand_transp_obj(self, generate_commercial.ItogClass dst, int idx) except*
    cdef void expand_nakl_obj(self, generate_commercial.ItogClass dst, int idx) except*


cpdef merge_trud_employees_of_list(list src, double naklad_percent, bint is_transp_risk)
