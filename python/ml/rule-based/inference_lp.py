#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Реализация LP-вывода и стохастического LP-вывода

import bitarray
import math
from typing import Tuple, List
from itertools import chain
import lp_model
import fuzzy_norms
from lp_model import Model, build_quotientset_a, build_atoms
from lp_model_classes import build_f, Relevance
import lp_model_classes
from functools import reduce
from statistics import median
import marshal
import pickle
import random
import sys
import numpy as np
import mu_calculator
import array
from mu_calculator import PreimagesContainerAll
from scipy.optimize import differential_evolution

epsilon = sys.float_info.epsilon

random.seed()

class AtomStatus:
    def __init__(self):
        self.mu = 1.0
        self.inferenced = False

CNT = 0

secure_random = random.SystemRandom()


class InferenceFuzzyLpContext:
    def __init__(self, model : Model, target : int, is_stochastic : bool):
        self.is_stochastic = is_stochastic
        self.model = model
        self.f_all, self.f_notinit, self.f_init = self.model.get_atoms_by_cathegory()
        if target not in self.f_notinit:
            raise Exception("Error target atom not in f_notintit atoms")

        self.norm = fuzzy_norms.MinMax()

        self.objects_by_atoms = {}
        self.atoms_names = {}
        self.natoms = len(self.f_all)
        self.f_is_init = [False for f in self.f_all]
        for f in self.f_init:
            self.f_is_init[f] = True

        self.target_atoms = [] # множество целевых атомов, которые могут быть выедены прообразами

        self.ATTRIBUTE_CATHEGORIAL = 0
        self.ATTRIBUTE_QUANTITATIVE = 1


        for (objname, atoms_with_names) in self.model.structured_objects.items():
            for f, fname in atoms_with_names:
                self.atoms_names[f] = fname
                if f not in self.objects_by_atoms:
                    self.objects_by_atoms[f] = objname
                else:
                    assert(self.objects_by_atoms[f] == objname)

        self.object_atoms = {}
        self.object_atoms_target = {}
        for f in self.f_init:
            objname = self.objects_by_atoms[f]
            try:
                self.object_atoms[objname].add(f)
            except KeyError:
                self.object_atoms[objname] = set([f])

        self.target_object_atoms = {} # Dict[object,{atoms}]

        self.mu_calc = mu_calculator.MuCalculator(self.natoms, self.norm, self.model, self)

        self.init_atoms_by_targets = {} # target, set(init_atoms)


        self.c_total = 1.0
        self.c_mu = 1.0
        self.c_median = 1.0

        self.set_mode_lp()


    def set_mode_lp(self):
        self.select_atom_for_quering = self.inference_get_relevance_atom

    def set_mode_random(self):
        self.select_atom_for_quering = self.inference_get_random_atom


    def random_object_cathegories_set(self):
        self.objects_cathegories = {objname: random.randint(0,1) for objname in self.model.structured_objects}

    def random_object_cathegories_save(self):
        with open("random_cathegories.db", "wb") as output:
            pickle.dump(self.objects_cathegories, output)

    def random_object_cathegories_load(self):
        with open("random_cathegories.db", "rb") as input:
            self.objects_cathegories = {a:b for (a,b) in (pickle.load(input)).items()}


    def init_targets(self):
        # ta = set()
        # for p in self.preimages:
        #    ta.add(p.inference_tree_target)
        ta = self.preimages.get_target_atoms()

        self.target_atoms.extend(list(sorted(ta)))

        for f in self.target_atoms:
            objname = self.objects_by_atoms[f]
            try:
                self.object_atoms_target[objname].add(f)
            except KeyError:
                self.object_atoms_target[objname] = set([f])

        targets_set = set(self.target_atoms)
        objs = set([self.objects_by_atoms[atom] for atom in self.target_atoms])
        for obj in objs:
            self.target_object_atoms[obj] = self.object_atoms_target[obj] & targets_set

        self.init_atoms_by_targets = self.preimages.get_init_atoms_linked_with_target(targets_set)

        self.preimages.init_dicts(self.f_init, self.target_atoms)
        self.f_init_linked = set(self.preimages.get_f_init_linked())  # рабочее множество начальных атомов, которые еще нужно конкретизировать


    def random_clear(self):
        self.mu_calc.set_random_rule_mu()
        self.atoms_cathegories = {f: self.objects_cathegories[self.objects_by_atoms[f]] for f in self.f_all}

    def clear_targets(self):
        self.random_clear()
        self.f_init_work = self.preimages.get_f_init_linked()  # рабочее множество начальных атомов, которые еще нужно конкретизировать
        self.target_wrk_objects = list(self.target_object_atoms.keys())
        self.target_answers  = {obj: None for obj in self.target_wrk_objects}
        self.target_inferenced_answer_cnt = 0
        self.target_atoms_mu = {atom: [-1] for atom in self.target_atoms} # Dict[int atom, double mu] множество степеней истинности целевых атомов
        self.target_wrk_object_atoms = {atom: set(s) for (atom,s) in self.target_object_atoms.items()}

        self.preimages.clear_preimages_status(self.mu_calc)

        # обновление быстрых контейнеров
        self.preimages.init_clear()

        # очистка рабочего множества запрошенных степеней истинности начальных атомов
        self.mu_calc.clear_f_mu_queried()

        self.preimages.calculate_inference_mus(self.mu_calc)

        # вычисление возможных степеней истинности для целей
        self.target_atoms_possible_mus = {atom: [-1] for atom in self.target_atoms}
        for f in self.target_atoms:
            self.set_possible_target_mu(f)

        self.f_init_relevance_wrk = {f: self.preimages.get_estimates_for_atom(f, self.norm) for f in self.f_init_work}

    def set_possible_target_mu(self, target):
        # обновление множества возможных степеней истинности целевых атомов для неопровергнутых но и невыведенных прообразов
        self.target_atoms_possible_mus[target][0] = self.preimages.calculate_possible_target_mus(target, self.norm)


    def add_answer(self, objname, mu, answers):
        v = self.target_answers[objname]
        if v is not None:
            mu_has, answers_has = v
            assert(mu_has >= mu)
            answers_has.update(set(answers))
        else:
            self.target_answers[objname] = (mu, set(answers))
            self.target_inferenced_answer_cnt += 1

        target_atoms_for_object = self.target_wrk_object_atoms[objname]
        f_init_for_deactualize = set()
        for target in target_atoms_for_object:
            f_init_for_deactualize.update(self.init_atoms_by_targets[target])

        for f in f_init_for_deactualize:
            self.preimages.invalidate_by_not_targets(f, target_atoms_for_object)

        for f in target_atoms_for_object:
            self.preimages.set_notactual_by_target(f)


    def calculate_preimages(self):
        self.preimages = self.model.stochastic_find_preimages() if self.is_stochastic else self.model.find_full_preimages()

    def save_preimages(self, min_variants):
        with open("tmp_preimages.db", "wb") as output:
            l = []

            for i, p in enumerate(self.preimages):
                if p.inference_variants >= min_variants:
                    f_l = []
                    for i in self.f_init:
                        if p.preimage.bit_test(i):
                            f_l.append(i)

                    target, tree = p.inference_tree_target, p.inference_tree
                    l.append((target,
                              p.w_preimage,
                              array.array('l', f_l), # p.preimage.tobytes(),
                              array.array('l', tree)))


            pickle.dump(l, output)
            self.preimages = PreimagesContainerAll()
            self.preimages.extend(l, self.natoms)

    def load_preimages(self):
        with open("tmp_preimages.db", "rb") as input:
            l = pickle.load(input)
            self.preimages = PreimagesContainerAll()
            print("preimages loaded")
            self.preimages.extend(l, self.natoms)
            print("preimages parsed")


    def calculate_estimates(self):
        """ Вычислить оценки для атомов по заданному множеству прообразов"""
        estimates = []

        mu_calc = self.mu_calc

        for f in self.f_init_work:
            relevance = self.f_init_relevance_wrk[f]
            if relevance is not None:
                estimates.append(relevance)

        if len(estimates) == 0:
            return []

        max_w_median = max([o.w_median for o in estimates])
        max_w_mu = max([o.w_mu for o in estimates])
        max_w_total = max([o.w_total for o in estimates])

        for e in estimates:
            e.calc_weigted(self.c_total, self.c_median, self.c_mu, max_w_total, max_w_median, max_w_mu)

        # print(c_total, c_median, c_mu, max_w_total, max_w_median, max_w_mu)

        return estimates

    def inference_get_relevance_atom(self):
        estimates = self.calculate_estimates()

        #for r in sorted(estimates):
        #    print(r.f, r.w_total, r.w_median, r.w_mu, r.weigted)
        # confidences = [r.mu for r in self.model.rules]
        # print(confidences)
        if len(estimates) == 0:
            return None # плохой случай, нужно отладить в этой точке и проверить пустоту рабочего множества атомов
        queried_atom = max(estimates)
        return queried_atom

    def inference_get_random_atom(self):
        if len(self.f_init_work) > 0:
            return Relevance(secure_random.choice(list(self.f_init_work)), 0, 1, 0)
        else:
            return None

    def actualize_preimages(self, actualized_atoms, deactualized_atoms):

        return self.preimages.actualize_preimages(actualized_atoms, deactualized_atoms, self.target_atoms_mu, self.norm,
                                                  self.f_init_relevance_wrk, self)

    def check_inference_success(self, actualized_atoms, deactualized_atoms, changed_targets):
        # == Вычислить максимально возможную степень истинности атома для невыведенных прообразов ==
        possible_mus = self.target_atoms_possible_mus

        # Сравнение должно выполнятся внутри подгруппы атомов для вопроса и для max_mu взятой для подгруппы
        assert(self.target_inferenced_answer_cnt < len(self.target_object_atoms))
        try:
            changed_objects = set([self.objects_by_atoms[atom] for atom in changed_targets])
        except KeyError as err:
            raise err

        for target_obj in changed_objects:
            target_atoms_for_object = self.target_wrk_object_atoms[target_obj]
            # max_mu должно определяться по каждому объекту
            max_mu = max([self.target_atoms_mu[target][0] for target in target_atoms_for_object])
            max_possible_mu = max([possible_mus[target][0] for target in target_atoms_for_object])
            if max_mu > max_possible_mu: # TODO: ? >=
                # ответ найден по факту масксимальности
                answers = [atom for atom in target_atoms_for_object if math.fabs(self.target_atoms_mu[atom][0] - max_mu) < epsilon]
                self.add_answer(target_obj, max_mu, answers)
            else:
                # Сравнить максимально возможние истинности c выведенными
                for f_target in target_atoms_for_object:
                    mu_possible = possible_mus[f_target][0]
                    target_mu = self.target_atoms_mu[f_target][0]
                    if mu_possible < 0 or mu_possible == 0 or mu_possible < epsilon: # данный атом невозможне в контексте этого вывода
                        continue
                    if target_mu < 0 and mu_possible >= max_mu:
                        break  # вывод незавершен т.к. есть еще невыведенные атомы, возможная истинность которых больше либо равна максимально известной
                    if target_mu >= 0:
                        real_max_possible_mu = self.norm.s(target_mu, mu_possible)
                        if target_mu <= real_max_possible_mu and real_max_possible_mu > max_mu: # TODO: ? >=
                            break  # вывод незавершен т.к. степень истинности выведенного атома может быть увеличена и ее увеличение может изменить максимально возможную истинность
                else:
                    answers = [atom for atom in target_atoms_for_object if math.fabs(self.target_atoms_mu[atom][0] - max_mu) < epsilon]
                    self.add_answer(target_obj, max_mu, answers)


        if self.target_inferenced_answer_cnt == len(self.target_object_atoms):
            return True # вывод завершен, выведен атом с максимальной степенью истинности и ее нельзя увеличить либо найти другой более истинный
        return False


    def query_atom(self, requester, f_relevance):
        f = f_relevance.f
        f_relevance.mu = requester(self, f)

        self.mu_calc.set_queried_mu(f, f_relevance.mu)
        self.preimages.update_mu(f, self.mu_calc)

        return f_relevance

    def deactualize_atoms_by_objects(self, actual_atoms):
        """Деактуализация атомов согласно их взаимосвязям в объектах"""
        actual_atoms_set = set([relevance.f for relevance in actual_atoms])
        deactualized_atoms = set()

        for f in sorted(actual_atoms_set):
            objname = self.objects_by_atoms[f]
            if self.objects_cathegories[objname] != self.ATTRIBUTE_CATHEGORIAL:
                # количественные атрибуты должны быть известны все
                self.f_init_work.remove(f)
            else:
                try:
                    atoms_to_deactualize = self.object_atoms[objname] # - actual_atoms_set
                except AttributeError:
                    raise

                for atom in atoms_to_deactualize:
                    if atom in self.f_init_linked:
                        self.f_init_work.remove(atom)
                        if atom != f:
                            deactualized_atoms.add(atom)

        return deactualized_atoms

    def inference_lp_body(self):
        queried_atoms_cnt = 1
        requester = lp_model_classes.random_requester

        queried_atom = self.select_atom_for_quering()
        actual_atoms = [self.query_atom(requester, queried_atom)]
        deactualized_atoms = self.deactualize_atoms_by_objects(actual_atoms)
        f_actual_atoms = [relevance.f for relevance in actual_atoms]
        changed_targets = self.actualize_preimages(f_actual_atoms, deactualized_atoms)

        while (not self.check_inference_success(f_actual_atoms, deactualized_atoms, changed_targets)
               and len(self.f_init_work) > 0):
            queried_atom = self.select_atom_for_quering()
            actual_atoms = [self.query_atom(requester, queried_atom)]
            f_actual_atoms = [relevance.f for relevance in actual_atoms]
            deactualized_atoms = self.deactualize_atoms_by_objects(actual_atoms)
            changed_targets = self.actualize_preimages(f_actual_atoms, deactualized_atoms)

            queried_atoms_cnt += 1


        # печать ответа
        if 0:
            # выбор наиболее истинного целевого ответа
            print("=== queried_atoms_cnt: ", queried_atoms_cnt, "===")
            try:
                for objname in sorted(self.target_answers):
                    t = self.target_answers[objname]
                    if t is not None:
                        mu, answers = t
                        for answer in sorted(answers):
                            print(objname, '=', self.atoms_names[answer], 'mu = ', round(mu, 2))
                    else:
                        print(objname, '= None, mu = None')

                # TODO: реализовать обучение весовых коэффициентов с помощью дифференциальной эволюции
            except TypeError as err:
                raise err

        return queried_atoms_cnt

    def print_learned(self):
        self.c_total = 55.344688434
        self.c_mu = 196.412026301
        self.c_median = 87.3046551743

        self.c_total = 1.0
        self.c_mu = 1.0
        self.c_median = 1.0

        # self.set_mode_random()
        self.set_mode_lp()

        def func():
            testlen = 20
            q_s = []
            for j in range(0,testlen):
                self.clear_targets()
                q_s.append(self.inference_lp_body())

            Q = float(sum(q_s)) / testlen
            Q = Q / len(self.f_init_linked)
            print("{'q_s':'", int(sum(q_s) / testlen), ", 'Q': ", Q, ", 'c_total':", self.c_total, ", 'c_mu:", self.c_mu, ", 'c_median':", self.c_median, "},")
        for i in range(0, 100):
            func()



    def learn_coeffs(self):
        def func(x):
            testlen = 20
            self.c_total = x[0]
            self.c_mu = x[1]
            self.c_median = x[2]

            q_s = []
            for j in range(0,testlen):
                self.clear_targets()
                q_s.append(self.inference_lp_body())

            Q = float(sum(q_s)) / testlen
            Q = Q / len(self.f_init_linked)
            print("{'q_s':'", int(sum(q_s) / testlen), ", 'Q': ", Q, ", 'c_total':", self.c_total, ", 'c_mu:", self.c_mu, ", 'c_median':", self.c_median, "},")
            return Q
        bounds = [(0,200), (0, 200), (0, 200)]
        res = differential_evolution(func, bounds, maxiter=50, popsize=15)
        print(res)



    def inference_lp(self):
        if 0:
            self.random_object_cathegories_set()
            self.random_object_cathegories_save()
        else:
            self.random_object_cathegories_load()

        if 1:
            self.calculate_preimages()
            #self.save_preimages(min_variants=2)
            return
        else:
            self.load_preimages()

        self.init_targets()


        if 0:
            for i in range(0,500):
                self.clear_targets()
                self.inference_lp_body()
        else:
            # self.learn_coeffs()
            self.print_learned()


def stat_print(ctx):
    print('rules:', len(ctx.model.rules))
    print('f_init:', len(ctx.f_init))
    print('f_all:', len(ctx.f_all))
    print('objs:', len(ctx.model.structured_objects))



def main():
    model = Model.from_json('electricians_conv.json')
    ctx = InferenceFuzzyLpContext(model, model.targets[0], is_stochastic=False)
    stat_print(ctx)


    ctx.inference_lp()


if __name__ == '__main__':
    main()

