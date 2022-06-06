
from itertools import product
from cython.operator cimport preincrement, dereference
from libcpp.vector cimport vector
from libcpp.unordered_map cimport unordered_map
from libcpp.string cimport string
from libcpp.utility cimport pair
from libcpp cimport bool

from lp_bitarray cimport bitarray
from lp_bitarray cimport make_bitarray


cdef class RuleR:
    def __cinit__(self, object rule):
        self.cons = rule.cons
        self.pre_not_init = rule.pre_not_init
        self.pre_init = rule.pre_init
        self.idx = rule.idx


cdef class RestrictedEquivalenceClass:
    def __cinit__(self, int f, object eq_class, object atoms_set, bitarray atoms_bitarr):
        cdef bitarray mask
        cdef bitarray fast_rules_barray
        cdef vector[int] rules_idxs
        cdef vector[int] pre_atoms_idxs
        cdef int i

        self.rules = None
        assert(eq_class is not None)

        mask = eq_class.barray_pre_atoms & atoms_bitarr

        if not mask.any():
            self.rules = [RuleR(r) for r in eq_class.rules]
        else:
            self.rules = []
            fast_rules_barray = bitarray(eq_class.w_rules)

            rules_idxs.clear()
            pre_atoms_idxs.clear()
            for f in atoms_set:
                if mask.bit_test(f):
                    pre_atoms_idxs.push_back(f)

                    for i in eq_class.rules_per_atoms_fast_indices[f]:
                        if not fast_rules_barray.test_bit(i):
                            fast_rules_barray.bit_set(i)
                            rules_idxs.push_back(i)

                    self.rules.extend([RuleR(r) for r in eq_class.rules_per_atoms[f]])

            self.rules = [RuleR(eq_class.rules[i]) for i in sorted(rules_idxs)]


cdef class StackLevel:
    def __cinit__(self,
                 bitarray path_atoms,
                 vector[int] &path_rules,
                 bitarray premage,
                 vector[int] &level_noninit_atoms,
                 object quotientset_restricted,
                 size_t w_preimage_chunck):
        cdef int f
        cdef RestrictedEquivalenceClass qs

        self.w_preimage_chunck = w_preimage_chunck
        self.path_atoms = path_atoms
        self.path_rules.swap(path_rules)
        # self.path_rules = path_rules
        self.preimage = premage
        self.noninit_atoms.swap(level_noninit_atoms)
        # self.noninit_atoms = level_noninit_atoms

        self.gen_rules_list = []
        for f in self.noninit_atoms:
            qs = quotientset_restricted[f]
            assert(qs is not None)
            self.gen_rules_list.append(qs.rules)

        self.generator = product(*self.gen_rules_list)

cdef class Preimage:
    def __cinit__(self, bitarray preimage, vector[int] &inference_tree, size_t w_preimage, size_t target):
        self.preimage = preimage
        self.inference_tree_target = target
        self.inference_tree.swap(inference_tree) # ?
        # self.inference_tree = inference_tree
        self.w_preimage = w_preimage
        self.inference_variants = 1

    def hash_value(self):
        return self.preimage.hash_value()

    cdef void chash_value(self, string &dst):
        self.preimage.str_hash_value(dst)


cpdef make_preimage


cpdef void find_minpreimage_step(StackLevel level,
                                 object preimages_dst,
                                 unsigned long int t,
                                 object atoms_stack,
                                 object quotientset_restricted,
                                 object rules_level_tuple):
    cdef RuleR r
    cdef int f
    cdef vector[int] level_noninit_atoms # список неначальных атомов решетки
    cdef size_t w_preimage = level.w_preimage_chunck
    cdef bitarray path_atoms = level.path_atoms.copy() # путь из неначальных атомов для обнаружения циклов
    cdef bitarray preimage = level.preimage.copy()
    cdef bitarray level_path = level.path_atoms
    cdef vector[int] path_rules = level.path_rules.copy()
    cdef Preimage p
    cdef StackLevel s


    # Все неначальные атомы генерации собираются в путь уровня для генерации следующего уровня
    # Все начальные атомы собираются в образец для прообраза

    for r in rules_level_tuple:
        for f in r.pre_not_init:
            if level_path.bit_test(f):  # обнаружен цикл
                break
            else:
                if not path_atoms.bit_test(f):
                    level_noninit_atoms.push_back(f)

                    path_atoms.bit_set(f)
        else:  # при обходе неначальных атомов - цикл не найден
            w_preimage += r.pre_init.size()
            for f in r.pre_init:
                preimage.bit_set(f)
            path_rules.push_back(r.idx)
            continue
        break  # найден цикл
    else:  # цикл не найден после просмотра всего кортежа активированных правил
        if level_noninit_atoms.empty():  # в кортеже активированных правил нет неначальных атомов => прообраз
            # preimage[t] = True
            # Добавление в список вывода: выведенного прообраза и дерева вывода прообраза
            p = Preimage.__new__(Preimage, preimage.copy(), path_rules, w_preimage, t)
            preimages_dst.append(p)
        else:
            s = StackLevel.__new__(StackLevel, path_atoms, path_rules, preimage, level_noninit_atoms, quotientset_restricted, w_preimage)
            atoms_stack.append(s)



cpdef void get_historamm(agg_lengths, unordered_map[size_t, size_t] &hist):
    cdef size_t i
    for i in agg_lengths.values():
        try:
            hist[i] += 1
        except KeyError:
            hist[i] = 1


cpdef object make_unique_preimages(object preimage_dst): # List[Preimage]:
    cdef unordered_map[string, size_t] agg_hist
    cdef unordered_map[string, size_t] preimages
    cdef unordered_map[size_t, size_t] hist
    cdef pair[unordered_map[string, size_t].iterator, bool] insert_res
    cdef pair[size_t, size_t] it

    cdef size_t founded_index
    cdef size_t *pfounded_index
    cdef Preimage p
    cdef pair[string, size_t] insert_val
    cdef size_t i


    i = 0
    for p in preimage_dst:
        p.chash_value(insert_val.first)
        insert_val.second = i
        insert_res = preimages.insert(insert_val)
        if not insert_res.second:
            founded_index = dereference(insert_res.first).second
            preimage_dst[founded_index].inference_variants += 1


        insert_val.second = 1
        insert_res = agg_hist.insert(insert_val)
        if not insert_res.second:
            pfounded_index = &(dereference(insert_res.first).second)
            preincrement(pfounded_index[0])

        preincrement(i)

    get_historamm(agg_hist, hist)
    py_hist = {}
    for it in hist:
       py_hist[it.first] = it.second

    print('unique pooled preimages histogramm:', py_hist)
    print('unique preimages:', preimages.size())
    return preimage_dst
    # return [preimages[k] for k in sorted(preimages)]


