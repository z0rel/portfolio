from itertools import islice

from libcpp.unordered_map cimport unordered_map
from libcpp.memory cimport shared_ptr
from libcpp.vector cimport vector
from libcpp.string cimport string
from aggregate_commercial_internal import Cathegory
from libcpp.utility cimport pair
from libcpp cimport bool
from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.algorithm cimport sort


