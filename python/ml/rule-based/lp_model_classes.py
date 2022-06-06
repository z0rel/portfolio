#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import random
# from lp_bitarray import bitarray as bitarray_obj
from lp_bitarray import make_bitarray as bitarray

# from lp_bitarray import make_bitarray as bitarray
from typing import Tuple, List, Dict
from graph_tool import Graph
from itertools import product

NATOMS_EXTRA_CNT = 2




class Rule:
    def __init__(self, pre, cons, mu, idx, natoms, pre_names, cons_names):
        self.pre = pre
        self.cons = cons
        self.mu = mu
        self.idx = idx
        self.pre_set = set(pre)
        self.pre_barray = bitarray(natoms)
        self.pre_names = pre_names
        self.cons_names = cons_names
        for f in pre:
            self.pre_barray.bit_set(f)

        self.pre_init = []
        self.pre_not_init = []

    def to_init_list(self):
        return (self.pre, self.cons, self.mu, self.idx)

    def __str__(self):
        return str(self.to_init_list())

    def set_init_notinit(self, init_barray):
        self.pre_init = [f for f in self.pre if init_barray.bit_test(f)]
        self.pre_not_init = [f for f in self.pre if not init_barray.bit_test(f)]


def build_f(rules):
    f_all = set()
    f_notinit = set()

    for r in rules:
        f_all.add(r.cons)
        f_notinit.add(r.cons)
        for f in r.pre:
            f_all.add(f)

    return f_all, f_notinit, f_all - f_notinit





class Relevance:
    def __init__(self, f, w_total, w_median, w_mu):
        self.f = f
        self.w_total = w_total
        self.w_median = w_median
        self.w_mu = w_mu
        self.weigted = 0
        self.mu = None

    def calc_weigted(self, c_total, c_median, c_mu, max_w_total, max_w_median, max_w_mu):
        self.weigted = (c_total  * self.w_total / max_w_total +
                        c_median * max_w_median / self.w_median +
                        c_mu     * self.w_mu    / max_w_mu)

    def __le__(self, other):
        return self.weigted <= other.weigted

    def __lt__(self, other):
        return self.weigted < other.weigted

    def __ge__(self, other):
        return self.weigted >= other.weigted

    def __gt__(self, other):
        return self.weigted > other.weigted


def random_requester(ctx, atom):
    if ctx.atoms_cathegories[atom] == ctx.ATTRIBUTE_QUANTITATIVE:
        return random.uniform(0.0,1.0)
    elif ctx.atoms_cathegories[atom] == ctx.ATTRIBUTE_CATHEGORIAL:
        return random.randint(0,1)
    else:
        print(atom, ctx.atoms_cathegories[atom])
        raise Exception()




