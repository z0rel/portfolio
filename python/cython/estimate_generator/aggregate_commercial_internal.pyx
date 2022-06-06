# distutils: language = c++
# cython: language_level=3

import re

from cython.operator cimport dereference as deref, preincrement as inc

from libcpp.unordered_map cimport unordered_map
from libcpp.memory cimport shared_ptr
from libcpp.utility cimport pair

from commercial_role cimport Role
from commercial_role cimport RoleParsingContext

from xlcommercial.calc.estimate.sysdefines import calculate_nalog_fzp_coeff
from xlcommercial.calc.estimate.sysdefines import calculate_fot_coeff, calculate_fot_without_otpusk
from xlcommercial.calc.estimate import commercial_defines as defs


from parsing_context cimport ParsingContext

cdef double FZP_NALOG = calculate_nalog_fzp_coeff()
cdef double FOT_WITH_OTPUSK = calculate_fot_coeff()
cdef double FOT_WITHOUT_OTPUSK = calculate_fot_without_otpusk()

cdef int NAKLAD_ID_MIN = defs.NAKLAD_ID_MIN
cdef int NAKLAD_ID_MAX = defs.NAKLAD_ID_MAX
cdef int INFORM_ID_MIN = defs.INFORM_ID_MIN
cdef int INFORM_ID_MAX = defs.INFORM_ID_MAX
cdef int RISKS_ID_MIN = defs.RISKS_ID_MIN
cdef int RISKS_ID_MAX = defs.RISKS_ID_MAX
cdef int SPECIAL_ID = defs.SPECIAL_ID
cdef int TRANSPORT_ID = defs.TRANSPORT
cdef double C_BANK_GUARANTEE = defs.BANK_GUARANTEE
cdef double C_SMET_PRIB = defs.SMET_PRIB
cdef double C_ZP_DIR_AFTER = defs.ZP_DIR_AFTER
cdef double C_POSTPAY_PRIB = defs.POSTPAY_PRIB
cdef double C_WORKS_GRAY_COMPLECT = defs.WORKS_GRAY_COMPLECT
cdef double C_OTKAT_ID = defs.OTKAT_ID
cdef double C_BUCH_COST = defs.BUCH_COST
cdef double C_DIRECTOR_COST = defs.DIRECTOR_COST
cdef double C_GUARANTEE = defs.GUARANTEE
cdef double C_AVANCE_NALOG = defs.AVANCE_NALOG

cimport numpy as np
cimport generate_commercial
from generate_commercial cimport DTYPE_t
import generate_commercial


# Столбец, в котором может записываться номер пункта в экспортированной строке калькуляции
DEF IDX_COL_EXPORT_ROW_NUMBER = 1

cdef double stub(double x):
    return x

# cdef ROUND_FUN = round
cdef ROUND_FUN = stub
cdef object SUBS_KICKBACKS = re.compile("Откаты местным", re.IGNORECASE)
cdef object SUBS_KOMANDIR_SUTOCH_PAY = re.compile("Командировочные и суточные выплаты", re.IGNORECASE)


cdef class RowProperties:
    def __cinit__(self):
        self.rownumber = ''
        self.is_local_risk = False
        self.need_skip_descr = False
        self.local_risk_zp_positions = 999999


cdef object Role_getstate(Role* self):
        return (self.trudoemkost_summary, self.employees_cnt, self.salary_day, self.summary_salary,
                self.role_name.decode(), self.rows_cnt, self.is_ip, self.position, self.role_id)


cdef void Role_setstate(Role* self, object state):
    (trudoemkost_summary, employees_cnt, salary_day, summary_salary,
            role_name, rows_cnt, is_ip, position, role_id) = state
    self.trudoemkost_summary = trudoemkost_summary
    self.employees_cnt = employees_cnt
    self.salary_day = salary_day
    self.summary_salary = summary_salary
    self.role_name = role_name.encode()
    self.rows_cnt = rows_cnt
    self.is_ip = is_ip
    self.position = position
    self.role_id = role_id


cdef class SalaryEmployee:
    def __repr__(self):
        return ", ".join([str(x) for x in [
            self.salary_per_day, self.salary_summary, self.laboriousness_clear,
            self.rows_cnt, self.employees_cnt, self.salary_summary_corrected
        ]])

    cpdef double get_salary_per_day(self):
        return self.salary_per_day

    cpdef double get_salary_summary(self):
        return self.salary_summary

    cpdef double get_laboriousness_clear(self):
        return self.laboriousness_clear

    cpdef double get_rows_cnt(self):
        return self.rows_cnt

    cpdef double get_employees_cnt(self):
        return self.employees_cnt

    cpdef double get_salary_summary_corrected_fot(self):
        if self.is_ip:
            return self.salary_summary_corrected / 0.94
        else:
            return self.salary_summary_corrected * FOT_WITH_OTPUSK

    cpdef double nakl_in_salary(self):
        """ накладные расходы в суммарной зарплате работника """
        return self.salary_summary_corrected - self.salary_summary

    cpdef double nakl_in_laboriousness(self):
        """ накладные расходы в суммарной трудоемкости работника """
        return  self.laboriousness_corrected() - self.laboriousness_clear

    cpdef double salary_per_day_fot(self):
        if self.is_ip:
            return self.salary_per_day / 0.94
        else:
            return self.salary_per_day * FOT_WITH_OTPUSK

    cpdef double salary_summary_fot(self):
        if self.is_ip:
            return self.salary_summary / 0.94
        else:
            return self.salary_summary * FOT_WITH_OTPUSK

    cpdef laboriousness_corrected(self):
        """Скорректированная суммарная трудоемкость работника"""
        return self.salary_summary_corrected / self.salary_per_day

    def __cinit__(self, salary_per_day, pos, is_ip):
        self.salary_per_day = salary_per_day
        self.position = pos
        self.salary_summary = 0.0
        self.laboriousness_clear = 0.0
        self.rows_cnt = 0
        self.employees_cnt = 0
        self.salary_summary_corrected = 0.0
        # self.item_summary_salary = []
        self.is_ip = is_ip


cdef class Flags:
    cpdef set_compl_price(self, double compl_price):
        self.compl_price = compl_price

    def __cinit__(self):
        self.cathegory_val = -1.0
        self.flag_is_transport = False
        self.flag_comandir_or_sutoch_rash = False
        self.compl_price = 0.0
        self.summary_zarplata_value_ip = 0.0
        self.summary_zarplata_value = 0.0
        self.need_convert_to_euro = False
        self.flag_is_transport_materials = False
        self.flag_is_transport_instruments = False
        self.flag_is_transport_arenda = False
        self.flag_is_transport_tickets = False
        self.zagran_comandir = False

    def __getstate__(self):
        return (self.cathegory_val, self.flag_is_transport, self.flag_is_transport_materials,
            self.flag_is_transport_instruments, self.flag_is_transport_arenda, self.flag_is_transport_tickets,
            self.flag_comandir_or_sutoch_rash, self.summary_zarplata_value, self.summary_zarplata_value_ip,
            self.compl_price, self.need_convert_to_euro, self.zagran_comandir)

    def __setstate__(self, state):
        (self.cathegory_val, self.flag_is_transport, self.flag_is_transport_materials,
         self.flag_is_transport_instruments, self.flag_is_transport_arenda, self.flag_is_transport_tickets,
         self.flag_comandir_or_sutoch_rash, self.summary_zarplata_value, self.summary_zarplata_value_ip,
         self.compl_price, self.need_convert_to_euro, self.zagran_comandir) = state

    cpdef is_transport_rash(self):
        return (self.flag_is_transport or self.flag_comandir_or_sutoch_rash)

    cpdef is_transport_risk_nakl(self):
        return (self.flag_is_transport or self.flag_comandir_or_sutoch_rash) and (self.is_nakl() or self.is_risks())

    cpdef bint is_transport_tickets(self):
        return self.flag_is_transport_tickets

    cpdef bint is_transport_arenda(self):
        return self.flag_is_transport_arenda

    cpdef bint is_transport_instruments(self):
        return self.flag_is_transport_instruments

    cpdef bint is_transport_materials(self):
        return self.flag_is_transport_materials

    cpdef bint is_transport_risk(self):
        return self.flag_is_transport and self.is_risks()

    cpdef bint is_transport_nakl(self):
        return self.flag_is_transport and self.is_nakl()

    cpdef bint is_bank_guarantee(self):
        return self.cathegory_val == C_BANK_GUARANTEE

    cpdef bint is_cathegory(self, cathegory):
        return self.cathegory_val == float(cathegory)

    cpdef bint is_naklad_comandir_or_sutoch_value(self):
        return self.flag_comandir_or_sutoch_rash and self.is_nakl()

    cpdef bint is_comandir_or_sutoch_value(self):
        return self.flag_comandir_or_sutoch_rash

    cpdef bint is_work_or_compl(self):
        return self.cathegory_val < SPECIAL_ID

    cpdef bint is_transport(self):
        return (self.cathegory_val >= TRANSPORT_ID)

    cpdef bint is_nakl(self):
        return (self.cathegory_val >= NAKLAD_ID_MIN) and (self.cathegory_val < NAKLAD_ID_MAX)

    cpdef bint is_inform(self):
        return (self.cathegory_val >= INFORM_ID_MIN) and (self.cathegory_val < INFORM_ID_MAX)

    cpdef bint is_risks(self):
        return (self.cathegory_val >= RISKS_ID_MIN) and (self.cathegory_val < RISKS_ID_MAX)

    cpdef bint is_work(self):
        return (self.cathegory_val < SPECIAL_ID) and ((self.summary_zarplata_value > 0) or (self.summary_zarplata_value_ip > 0))

    cpdef bint is_complect(self):
        return (self.cathegory_val < SPECIAL_ID) and (self.compl_price > 0)


cdef class Cathegory(Flags):

    cdef double __get_summary_zarplata_merge_trud(self):
        return self.summary_zarplata_value + self.summary_zarplata_value_ip

    cdef double get_compl(self):
        # TODO: округление является узким местом здесь, т.к. участвует в расчете корректур и создает разрывную функцию
        return ROUND_FUN(self.compl_price)

    cpdef add_description(self, str row):
        self.descr.append(row)

    cpdef double get_double_cathegory(self):
        return self.cathegory_val

    cpdef double get_local_correcture(self):
        # не считать нечаянно введенные корректуры комплектующих или работ если реальной стомости пункта
        # по комплектующим или работам - нет
        if self.compl_price and (self.summary_zarplata_value or self.summary_zarplata_value_ip):
            return self.local_correcture_compl + self.local_correcture_work * FOT_WITH_OTPUSK
        elif self.compl_price:
            return self.local_correcture_compl
        else:
            return self.local_correcture_work * FOT_WITH_OTPUSK

    cpdef set_local_correcture(self, double val):
        cdef double val1
        cdef double val2
        # if self.compl_price and (self.summary_zarplata_value or self.summary_zarplata_value_ip):
        #     val1 = val / 2
        #     val2 = val
        #     self.local_correcture_compl = val1
        #     self.local_correcture_work = val2 * self.local_correcture_salary
        if self.compl_price:
            self.local_correcture_compl = val * self.euro_cource
        else:
            self.local_correcture_work = val * self.local_correcture_salary
            # print(self.get_double_cathegory(), self.local_correcture_work, self.local_correcture_compl)

    cpdef double get_local_correcture_raw(self):
        """ Используется в коде расчета корректур """
        # не считать нечаянно введенные корректуры комплектующих или работ если реальной стомости пункта
        # по комплектующим или работам - нет
        # if self.compl_price and (self.summary_zarplata_value or self.summary_zarplata_value_ip):
        #     return self.local_correcture_compl + self.local_correcture_work / self.local_correcture_salary
        if self.compl_price or not self.local_correcture_salary:
            return self.local_correcture_compl / self.euro_cource
        else:
            try:
                return self.local_correcture_work / self.local_correcture_salary
            except ZeroDivisionError as e:
                print(str(e))
                print('return self.local_risk_work / self.local_risk_salary: {0} / {1}'.format(self.local_correcture_work, self.local_correcture_salary))
                print('cathegory:', self.cathegory_val)
                raise

    cpdef double get_local_risk_salary(self):
        return self.local_correcture_salary

    cpdef double get_summary_zarplata(self):
        return self.summary_zarplata_value + self.summary_zarplata_value_ip

    cpdef double get_compl_price(self):
        return self.compl_price

    cpdef double get_work_price(self):
        if self.flag_comandir_or_sutoch_rash:
            return self.get_sutoch_fot()
        else:
            return self.__get_summary_fot()

    cpdef double get_itog(self):
        if self.flag_comandir_or_sutoch_rash:
            return self.compl_price + self.get_sutoch_fot()
        else:
            return self.compl_price + self.__get_summary_fot()

    cdef double __get_summary_fot(self) except*:
        return self.summary_zarplata_value * FOT_WITH_OTPUSK + self.summary_zarplata_value_ip / 0.94

    cdef double get_fot(self) except*:
        if self.flag_comandir_or_sutoch_rash:
            return ROUND_FUN(self.get_sutoch_fot())
        else:
            # TODO: округление является узким местом здесь, т.к. участвует в расчете корректур и создает разрывную функцию
            return ROUND_FUN(self.summary_zarplata_value) * FOT_WITH_OTPUSK + ROUND_FUN(self.summary_zarplata_value_ip / 0.94)

    cpdef double get_sutoch_fot(self):
        return self.__get_sutoch_zp_fot() + self.summary_zarplata_value_ip / 0.94

    cdef double __get_sutoch_zp_fot(self) except*:
        cdef double trud_summary = 0.0
        cdef unordered_map[int, shared_ptr[Role]].iterator trud_it = self.trud.begin()
        while trud_it != self.trud.end():
            if not deref(trud_it).second.get().is_ip:
                trud_summary += deref(trud_it).second.get().trudoemkost_summary
            inc(trud_it)

        cdef double x
        cdef double y
        cdef double sutoch_value = 2500.0 if self.zagran_comandir else 700.0

        x = sutoch_value * self.euro_cource
        if trud_summary:
            x -= self.summary_zarplata_value / trud_summary
        if x > 0:
            y = x * trud_summary * FOT_WITHOUT_OTPUSK
        else:
            return self.summary_zarplata_value

        return sutoch_value * self.euro_cource * trud_summary + y + self.summary_zarplata_value_ip / 0.94

    cdef double get_fot_of_nakl_table_row(self):
        if self.is_naklad_comandir_or_sutoch_value():
            return self.__get_sutoch_zp_fot() + self.summary_zarplata_value_ip / 0.94
        else:
            return self.summary_zarplata_value * FOT_WITH_OTPUSK + ROUND_FUN(self.summary_zarplata_value_ip) / 0.94

    def __cinit__(self):
        self.descr = list()
        self.cb_itog_data_idx = -1
        self.nds = 0
        self.cb_itog_data = None
        self.int_counter = 1
        self.local_correcture_salary = 2100

    cpdef object c_init(self, str skey, double fkey, ParsingContext ctx, double euro_cource, int zagran_comandir, double valut_divide_coeff):

        self.valut_divide_coeff = valut_divide_coeff
        self.key = skey
        self.zagran_comandir = True if int(zagran_comandir) else False
        self.cathegory_val = fkey

        self.need_convert_to_euro = ctx.cfg.convert_to_euro
        self.euro_cource = 1.0 if not self.need_convert_to_euro else 1.0 / ctx.cfg.calc_eur
        self.idx_col_markers = ctx.idx_col_markers

        cdef pair[int, shared_ptr[Role]] trud_empty_val
        trud_empty_val.first = 0
        cdef double zarpata

        cdef int is_IP = -1
        cdef int ip_idx_val = ctx.ip_position

        cdef unordered_map[int, shared_ptr[RoleParsingContext]].iterator indexed_pairs_role_to_salary_it
        cdef unordered_map[int, shared_ptr[RoleParsingContext]].iterator indexed_pairs_role_to_salary_it_end
        cdef shared_ptr[RoleParsingContext] rolename_and_zarplata
        cdef int idx = 0
        indexed_pairs_role_to_salary_it = ctx.indexed_pairs_role_to_salary.begin()
        indexed_pairs_role_to_salary_it_end = ctx.indexed_pairs_role_to_salary.end()

        while indexed_pairs_role_to_salary_it != indexed_pairs_role_to_salary_it_end:
            i = deref(indexed_pairs_role_to_salary_it).first
            rolename_and_zarplata = deref(indexed_pairs_role_to_salary_it).second

            is_IP = (1 if i == ip_idx_val else 0) # флаг - вычислять ФОТ для ИП а не для наемного работника

            trud_empty_val.first = i
            trud_empty_val.second = shared_ptr[Role](new Role())
            trud_empty_val.second.get().role_name = deref(rolename_and_zarplata).role_name
            trud_empty_val.second.get().salary_day = deref(rolename_and_zarplata).salary_value

            trud_empty_val.second.get().position = deref(rolename_and_zarplata).role_position_on_calculation
            trud_empty_val.second.get().is_ip = is_IP
            trud_empty_val.second.get().role_id = idx
            # empty_val.second.first = 0.0
            self.trud.insert(trud_empty_val) # инициализировать нулевую трудоемкость для профессии
            inc(indexed_pairs_role_to_salary_it)
            inc(idx)

        return self

    def __getstate__(self):
        trud_map = {}
        cdef unordered_map[int, shared_ptr[Role]].iterator trud_it = self.trud.begin()
        while trud_it != self.trud.end():
            trud_map[deref(trud_it).first] = Role_getstate(deref(trud_it).second.get())
            inc(trud_it)
        return (super().__getstate__(), trud_map, self.key, self.descr, self.local_correcture_compl,
                self.local_correcture_work, self.local_correcture_salary, self.int_counter,
                self.euro_cource, self.idx_col_markers, self.cb_itog_data, self.cb_itog_data_idx, self.nds)


    def __setstate__(self, state):
        cdef pair[int, shared_ptr[Role]] trud_state_val

        (super_state, trud_map, key, descr, local_correcture_compl, local_correcture_work, local_correcture_salary,
            int_counter, euro_cource, idx_col_markers, cb_itog_data, cb_itog_data_idx, nds) = state

        super().__setstate__(super_state)

        for i,v in trud_map.items():
            trud_state_val.first = i
            trud_state_val.second = shared_ptr[Role](new Role())
            Role_setstate(trud_state_val.second.get(), v)
            self.trud.insert(trud_state_val)

        self.key = key
        self.descr = descr
        self.local_correcture_compl = local_correcture_compl
        self.local_correcture_work = local_correcture_work
        self.local_correcture_salary = local_correcture_salary
        self.int_counter = int_counter
        self.euro_cource = euro_cource
        self.idx_col_markers = idx_col_markers
        self.cb_itog_data = cb_itog_data
        self.cb_itog_data_idx = cb_itog_data_idx
        self.nds = nds


    cdef void merge_trud_employees(self, list dst, double naklad_percent) except*:
        """ 
            Агрегировать трудоемкость служащих из контейнера unordered_map<int, shared_ptr<Role> >  self.trud
        :param dst: Приемник агрегации
        :param naklad_percent: 
        :return: 
        """
        # dst = {'local_risk': 0, 'roles': {}}
        # dst['roles']
        cdef unordered_map[int, shared_ptr[Role]].iterator trud_it = self.trud.begin()
        cdef int role_idx
        cdef str role_name
        cdef double percent_employee_item_salary0 # процент зарплаты работника в общей зарплате пункта
        cdef double percent_employee_item_salary # процент зарплаты работника в общей зарплате пункта
        cdef double item_employee_summary_salary

        # суммарный локальный риск по зарплате self.local_risk_work
        # размер зарплаты get_fot()
        # размер зарплаты работника salary_summary = summary_salary * fot_coeff

        # ФОТ пункта
        cdef double __item_fot = self.__get_summary_fot()
        if not __item_fot: # если ФОТ нулевой, то в этом пункте работ нет
            return

        # скорректированная зарплата пункта
        cdef double item_summary_salary = self.get_fot() * naklad_percent + self.local_correcture_work * FOT_WITH_OTPUSK
        # Превышение скорректированной зарплаты над общей зарплатой пункта
        cdef double percent_corrected_salary = item_summary_salary / __item_fot
        cdef naklad_val = item_summary_salary - __item_fot
        cdef int idx = 0
        cdef SalaryEmployee salary_employee

        while trud_it != self.trud.end():
            role_idx = deref(trud_it).first
            salary_employee = dst[idx]

            item_employee_summary_salary = deref(trud_it).second.get().summary_salary
            salary_employee.salary_summary += item_employee_summary_salary
            salary_employee.laboriousness_clear += deref(trud_it).second.get().trudoemkost_summary
            salary_employee.rows_cnt += deref(trud_it).second.get().rows_cnt
            salary_employee.employees_cnt += deref(trud_it).second.get().employees_cnt

            # процент зарплаты работника в общей зарплате пункта увеличенный пропорционально накладным в пункте
            if salary_employee.is_ip:
                percent_employee_item_salary0 = (item_employee_summary_salary / 0.94) / __item_fot
            else:
                percent_employee_item_salary0 = item_employee_summary_salary * FOT_WITH_OTPUSK / __item_fot

            percent_employee_item_salary = percent_corrected_salary * percent_employee_item_salary0

            # salary_employee.item_summary_salary.append((self.key, item_summary_salary, percent_corrected_salary,
            #                                             percent_employee_item_salary0, percent_employee_item_salary))

            # скорректированная зарплата работника
            salary_employee.salary_summary_corrected += item_employee_summary_salary * (1.0 + naklad_val / __item_fot)

            inc(trud_it)
            inc(idx)


    cdef RowProperties parse_rownumber(self, list row_of_calculation, int idx):
        """ Проанализировать маркеры разметки """
        cdef str need_row_number
        cdef RowProperties result = RowProperties()

        try:
            need_row_number = row_of_calculation[idx] if row_of_calculation else False
        except Exception as exc:
            print('ERROR parse_rownumber', str(exc))
            print('ERROR parse_rownumber', self.cathegory_val, 'idx', idx)
            return result

        if not need_row_number:
            return result

        if len(need_row_number) >= 5 and need_row_number[-5:] == "NDS20":
            self.nds = 20
            need_row_number = need_row_number[:-5]
            if not need_row_number:
                return result

        if need_row_number == '#':
            self.int_counter = 2
            result.rownumber = '1\t'
        elif need_row_number == '#.':
            self.int_counter = 2
            result.rownumber = '1.\t'
        elif need_row_number == '+' or need_row_number == 'p':
            result.rownumber = "-\t"
        elif need_row_number == 'S+':
            result.need_skip_descr = True
        elif need_row_number == 'n':
            result.rownumber = str(self.int_counter) + ".\t"
            self.int_counter += 1
        elif need_row_number == '.':
            result.rownumber = str(self.int_counter) + '.\t'
            self.int_counter += 1
        elif need_row_number == 'R':
            result.is_local_risk = True
        elif need_row_number == 's':
            return result
        elif need_row_number == 'T':
            self.flag_is_transport = True
        elif need_row_number == 'S':
            self.flag_comandir_or_sutoch_rash = True
        elif need_row_number == 'SS':
            self.flag_comandir_or_sutoch_rash = True
            result.need_skip_descr = True
        elif need_row_number == 'TM':
            self.flag_is_transport_materials = True
        elif need_row_number == 'TMS':
            self.flag_is_transport_materials = True
            result.need_skip_descr = True
        elif need_row_number == 'TA':
            self.flag_is_transport = True
            self.flag_is_transport_arenda = True
        elif need_row_number == 'TAS':
            self.flag_is_transport = True
            self.flag_is_transport_arenda = True
            result.need_skip_descr = True
        elif need_row_number == 'TI':
            self.flag_is_transport = True
            self.flag_is_transport_instruments = True
        elif need_row_number == 'TIS':
            self.flag_is_transport = True
            self.flag_is_transport_instruments = True
            result.need_skip_descr = True
        elif need_row_number == 'TT':
            self.flag_is_transport = True
            self.flag_is_transport_tickets = True
        elif need_row_number == 'TTS':
            self.flag_is_transport = True
            self.flag_is_transport_tickets = True
            result.need_skip_descr = True

        return result

    cpdef add_descr_item(self, str export_row_workname, double compl_price, list row_of_calculation,
                         RowProperties row_props):
        """ Добавить пункт описания и значения трудоемкости по профессиям  
            
        :param export_row_workname: Наименование работы в экспортированной строке калькуляции
        :param compl_price:         Стоимость комплектующих
        :param row_of_calculation:  Строка калькуляции для разбора цен
        """
        cdef bint is_local_risk = row_props.is_local_risk
        cdef int local_risk_zp_positions = row_props.local_risk_zp_positions

        # специальная пометка подпункта в экспортированной строке калькуляции

        # row_props = self.parse_rownumber(row_of_calculation, self.idx_col_markers)

        # подсчитать стоимость комплектующих
        if is_local_risk and compl_price:
            self.local_correcture_compl += compl_price * self.euro_cource
        elif compl_price:
            self.compl_price += compl_price * self.euro_cource

        # Наименование работы в экспортированной строке калькуляции
        export_row_workname = str(export_row_workname).strip() if export_row_workname else ""

        # Итоговое экспортируемое описание работы в которое опционально добавляется номер пункта работы
        export_row_work_description = (row_props.rownumber + (export_row_workname if export_row_workname else "")).strip()
        export_row_work_description = SUBS_KICKBACKS.sub("Дополнительная заработная плата", export_row_work_description)

        # Добавить работу в аггрегированный список работ для заданной категории работ
        if export_row_work_description and not is_local_risk and not row_props.need_skip_descr:
            self.descr.append(export_row_work_description)

        # Пройтись по всем строкам должностей и подсчитать из них стоимость трудозатрат по профессиям
        cdef double sum_trud = 0.0
        cdef unordered_map[int, shared_ptr[Role]].iterator trud_it = self.trud.begin() if row_of_calculation else self.trud.end()
        cdef double item_risk_compl = 0.0
        cdef double item_risk_work = 0.0

        cdef double trudoemkost_empl_cnt = 0.0
        cdef double trudoemkost_empl_single  = 0.0
        cdef int role_idx = 0
        cdef double role_salary_item_value = 0.0
        cdef double role_salary = 0.0
        cdef double role_risk_salary = 0.0
        cdef int is_IP = 0
        while trud_it != self.trud.end():
            role_idx = deref(trud_it).first
            is_IP = deref(trud_it).second.get().is_ip # флаг - вычислять ФОТ для ИП а не для наемного работника

            py_trudoemkost_empl_cnt = row_of_calculation[role_idx]
            trudoemkost_empl_cnt = py_trudoemkost_empl_cnt if py_trudoemkost_empl_cnt is not None else 0.0
            py_trudoemkost_empl_single = row_of_calculation[role_idx+1]
            try:
                trudoemkost_empl_single = py_trudoemkost_empl_single if (py_trudoemkost_empl_single is not None and py_trudoemkost_empl_single != '') else 0.0
            except TypeError as e:
                print(self.cathegory_val, py_trudoemkost_empl_single, str(e))
                raise

            role_salary = deref(trud_it).second.get().salary_day * self.euro_cource
            role_salary_item_value = trudoemkost_empl_cnt * trudoemkost_empl_single * role_salary

            # если строка таблицы - локальный риск - расчитать суммарное значение локального риска
            if is_local_risk:
                item_risk_work += role_salary_item_value
                if role_idx == local_risk_zp_positions or role_salary_item_value or trudoemkost_empl_cnt or trudoemkost_empl_single:
                    role_risk_salary = role_salary
                # print(export_row_work_description, item_risk_work)

            # если строка таблицы - обычные данные трудоемкости и комплектующих
            else:
                # если трудоемкость роли и число работников роли - заданы
                if trudoemkost_empl_cnt and trudoemkost_empl_single:
                    # Добавить к суммарной трудоемкости заданной профессии в аггрегированном списке работ -
                    # трудоемкость из текущей строки текущей профессии помноженная на зарплату
                    deref(trud_it).second.get().employees_cnt += trudoemkost_empl_cnt
                    deref(trud_it).second.get().rows_cnt += 1

                deref(trud_it).second.get().summary_salary += role_salary_item_value

                deref(trud_it).second.get().trudoemkost_summary += trudoemkost_empl_cnt * trudoemkost_empl_single
                if is_IP:
                    self.summary_zarplata_value_ip += role_salary_item_value
                else:
                    self.summary_zarplata_value += role_salary_item_value

            inc(trud_it)

        if is_local_risk:
            # print(self.cathegory_val, role_risk_salary, is_local_risk, local_risk_zp_positions,row_of_calculation)
            self.local_correcture_salary = role_risk_salary
            self.local_correcture_work += item_risk_work

    cdef public void append_cathegory_and_description(Cathegory self, generate_commercial.ItogClass dst, int idx) except *:
        dst.cathegories.append(self.cathegory_val)
        dst.descriptions.append("\r\n".join(self.descr))
        self.cb_itog_data = dst.data
        self.cb_itog_data_idx = idx


    cdef void _setup_dst_data_with_nakl(Cathegory self, generate_commercial.ItogClass dst, int idx,
                                        DTYPE_t dst0, DTYPE_t dst2) except *:
        cdef np.ndarray[generate_commercial.DTYPE_t, ndim=2] data = dst.data
        dst2 += dst0 * dst.naklad_percent
        data[idx, 0] = dst0
        data[idx, 1] = dst2 - dst0
        data[idx, 2] = dst2

    cdef void _setup_dst_data_nakl_table_row(Cathegory self, generate_commercial.ItogClass dst, int idx, DTYPE_t dst0, DTYPE_t dst2) except *:
        cdef np.ndarray[generate_commercial.DTYPE_t, ndim=2] data = dst.data

        data[idx, 0] = dst2       # "Комплектующие"
        data[idx, 1] = dst0       # ФОТ
        data[idx, 2] = dst2 + dst0 # Итого

    cpdef sum_compl_and_work_price(Cathegory self, SumPriceObj dst):
        dst.summary_compl += self.get_compl_price()
        dst.summary_work += self.get_work_price()

    cdef void expand_works_obj(Cathegory self, generate_commercial.ItogClass dst, int idx) except*:
        self._setup_dst_data_with_nakl(dst, idx, self.get_fot(), self.local_correcture_work * FOT_WITH_OTPUSK)

    cdef void expand_compl_obj(Cathegory self, generate_commercial.ItogClass dst, int idx) except*:
        self._setup_dst_data_with_nakl(dst, idx, self.get_compl(), self.local_correcture_compl)

    cdef void expand_transp_obj(Cathegory self, generate_commercial.ItogClass dst, int idx) except*:
        self._setup_dst_data_with_nakl(dst, idx, self.get_compl() + self.get_fot(), self.local_correcture_compl)

    cdef void expand_nakl_obj(Cathegory self, generate_commercial.ItogClass dst, int idx) except*:
        self._setup_dst_data_nakl_table_row(dst, idx, self.get_fot_of_nakl_table_row(), self.get_compl())


cdef class SumPriceObj:
    def __cinit__(self):
        self.summary_compl = 0.0
        self.summary_work = 0.0


cpdef merge_trud_employees_of_list(list src, double naklad_percent, bint is_transp_risk):
    cdef Cathegory wrk
    cdef dict dst
    cdef unordered_map[int, shared_ptr[Role]].iterator trud_it
    cdef list fast_dst
    cdef str role_name
    cdef SalaryEmployee salary_employee
    cdef int idx = 0
    dst = {}
    if not len(src):
        return dst
    wrk = src[0]
    trud_it = wrk.trud.begin()
    fast_dst = []


    while trud_it != wrk.trud.end():
        role_name = deref(trud_it).second.get().role_name.decode()
        salary_employee = SalaryEmployee(deref(trud_it).second.get().salary_day,
                                         deref(trud_it).second.get().position,
                                         deref(trud_it).second.get().is_ip)
        dst[role_name] = salary_employee
        fast_dst.append(salary_employee)
        inc(idx)
        inc(trud_it)

    if is_transp_risk:
        for wrk in src:
            if not wrk.is_comandir_or_sutoch_value():
                wrk.merge_trud_employees(fast_dst, naklad_percent)
    else:
        for wrk in src:
            wrk.merge_trud_employees(fast_dst, naklad_percent)

    return dst

